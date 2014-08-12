static enum v7_err arith(struct v7_val *a, struct v7_val *b,
                         struct v7_val *res, int op) {
  if (a->type == V7_TYPE_STR && a->type == V7_TYPE_STR && op == '+') {
    char *str = (char *) malloc(a->v.str.len + b->v.str.len + 1);
    CHECK(str != NULL, V7_OUT_OF_MEMORY);
    v7_init_str(res, str, a->v.str.len + b->v.str.len, 0);
    memcpy(str, a->v.str.buf, a->v.str.len);
    memcpy(str + a->v.str.len, b->v.str.buf, b->v.str.len);
    str[res->v.str.len] = '\0';
    return V7_OK;
  } else if (a->type == V7_TYPE_NUM && b->type == V7_TYPE_NUM) {
    v7_init_num(res, res->v.num);
    switch (op) {
      case '+': res->v.num = a->v.num + b->v.num; break;
      case '-': res->v.num = a->v.num - b->v.num; break;
      case '*': res->v.num = a->v.num * b->v.num; break;
      case '/': res->v.num = a->v.num / b->v.num; break;
      case '%': res->v.num = (unsigned long) a->v.num %
        (unsigned long) b->v.num; break;
      case '^': res->v.num = (unsigned long) a->v.num ^
        (unsigned long) b->v.num; break;
    }
    return V7_OK;
  } else {
    return V7_TYPE_ERROR;
  }
}

static enum v7_err do_arithmetic_op(struct v7 *v7, int op, int sp1, int sp2) {
  struct v7_val *res, tmp, *v1 = v7->stack[sp1 - 1], *v2 = v7->stack[sp2 - 1];

  CHECK(v7->sp >= 2, V7_STACK_UNDERFLOW);

  memset(&tmp, 0, sizeof(tmp));
  TRY(arith(v1, v2, &tmp, op));
  res = make_value(v7, tmp.type);
  CHECK(res != NULL, V7_OUT_OF_MEMORY);
  res->v = tmp.v;
  TRY(v7_push(v7, res));

  return V7_OK;
}

static int is_alpha(int ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static int is_digit(int ch) {
  return ch >= '0' && ch <= '9';
}

static int is_alnum(int ch) {
  return is_digit(ch) || is_alpha(ch);
}

static int is_space(int ch) {
  return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static void skip_whitespaces_and_comments(struct v7 *v7) {
  const char *s = v7->pc, *p = NULL;
  while (s != p && *s != '\0' && (is_space(*s) || *s == '/')) {
    p = s;
    while (*s != '\0' && is_space(*s)) {
      if (*s == '\n') v7->line_no++;
      s++;
    }
    if (s[0] == '/' && s[1] == '/') {
      s += 2;
      while (s[0] != '\0' && s[0] != '\n') s++;
    }
    if (s[0] == '/' && s[1] == '*') {
      s += 2;
      while (s[0] != '\0' && !(s[-1] == '/' && s[-2] == '*')) {
        if (s[0] == '\n') v7->line_no++;
        s++;
      }
    }
  }
  v7->pc = s;
}

static enum v7_err match(struct v7 *v7, int ch) {
  CHECK(*v7->pc++ == ch, V7_SYNTAX_ERROR);
  skip_whitespaces_and_comments(v7);
  return V7_OK;
}

static int test_and_skip_char(struct v7 *v7, int ch) {
  if (*v7->pc == ch) {
    v7->pc++;
    skip_whitespaces_and_comments(v7);
    return 1;
  }
  return 0;
}

static int test_token(struct v7 *v7, const char *kw, unsigned long kwlen) {
  return kwlen == v7->tok_len && memcmp(v7->tok, kw, kwlen) == 0;
}

static enum v7_err parse_num(struct v7 *v7) {
  double value = 0;
  char *end;

  value = strtod(v7->pc, &end);
  // Handle case like 123.toString()
  if (end != NULL && (v7->pc < &end[-1]) && end[-1] == '.') end--;
  CHECK(value != 0 || end > v7->pc, V7_SYNTAX_ERROR);
  v7->pc = end;
  v7->tok_len = (unsigned long) (v7->pc - v7->tok);
  skip_whitespaces_and_comments(v7);

  if (!v7->no_exec) {
    TRY(v7_make_and_push(v7, V7_TYPE_NUM));
    v7_top(v7)[-1]->v.num = value;
  }

  return V7_OK;
}

static int is_valid_start_of_identifier(int ch) {
  return ch == '$' || ch == '_' || is_alpha(ch);
}

static int is_valid_identifier_char(int ch) {
  return ch == '$' || ch == '_' || is_alnum(ch);
}

static enum v7_err parse_identifier(struct v7 *v7) {
  CHECK(is_valid_start_of_identifier(v7->pc[0]), V7_SYNTAX_ERROR);
  v7->tok = v7->pc;
  v7->pc++;
  while (is_valid_identifier_char(v7->pc[0])) v7->pc++;
  v7->tok_len = (unsigned long) (v7->pc - v7->tok);
  skip_whitespaces_and_comments(v7);
  return V7_OK;
}

static int lookahead(struct v7 *v7, const char *str, int str_len) {
  int equal = 0;
  if (memcmp(v7->pc, str, str_len) == 0 &&
      !is_valid_identifier_char(v7->pc[str_len])) {
    equal++;
    v7->pc += str_len;
    skip_whitespaces_and_comments(v7);
  }
  return equal;
}

static enum v7_err parse_compound_statement(struct v7 *v7, int *has_return) {
  if (*v7->pc == '{') {
    int old_sp = v7->sp;
    TRY(match(v7, '{'));
    while (*v7->pc != '}') {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(parse_statement(v7, has_return));
      if (*has_return && !v7->no_exec) return V7_OK;
    }
    TRY(match(v7, '}'));
  } else {
    TRY(parse_statement(v7, has_return));
  }
  return V7_OK;
}

static struct v7_val *cur_scope(struct v7 *v7) {
  return v7->curr_func == NULL ? &v7->root_scope : v7->curr_func->v.func.scope;
}

static enum v7_err parse_function_definition(struct v7 *v7, struct v7_val **v,
                                             int num_params) {
  int i = 0, old_no_exec = v7->no_exec, old_sp = v7->sp, has_return = 0, ln = 0;
  unsigned long func_name_len = 0;
  const char *src = v7->pc, *func_name = NULL;
  struct v7_val args;

  if (*v7->pc != '(') {
    // function name is given, e.g. function foo() {}
    CHECK(v == NULL, V7_SYNTAX_ERROR);
    TRY(parse_identifier(v7));
    func_name = v7->tok;
    func_name_len = v7->tok_len;
    src = v7->pc;
  }

  // If 'v' (func to call) is NULL, that means we're just parsing function
  // definition to save it's body.
  v7->no_exec = v == NULL;
  ln = v7->line_no;  // Line number where function starts
  TRY(match(v7, '('));

  memset(&args, 0, sizeof(args));
  v7_set_class(&args, V7_CLASS_OBJECT);

  while (*v7->pc != ')') {
    TRY(parse_identifier(v7));
    if (!v7->no_exec) {
      // TODO(lsm): use v7_setv() here
      struct v7_val *key = v7_mkv(v7, V7_TYPE_STR, v7->tok, v7->tok_len, 1);
      struct v7_val *val = i < num_params ? v[i + 1] : make_value(v7, V7_TYPE_UNDEF);
      inc_ref_count(key);
      TRY(v7_set(v7, &args, key, val));
      v7_freeval(v7, key);
    }
    i++;
    if (!test_and_skip_char(v7, ',')) break;
  }
  TRY(match(v7, ')'));

  if (!v7->no_exec) {
    assert(v7->curr_func != NULL);
    v7->curr_func->v.func.args = &args;
  }

  TRY(parse_compound_statement(v7, &has_return));

  if (v7->no_exec) {
    struct v7_val *func;
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    func = v7_top(v7)[-1];
    func->flags |= V7_JS_FUNC;
    v7_set_class(func, V7_CLASS_FUNCTION);

    func->v.func.line_no = ln;
    func->v.func.source_code = v7_strdup(src, (unsigned long) (v7->pc - src));
    func->v.func.scope = v7_mkv(v7, V7_TYPE_OBJ);
    func->v.func.scope->ref_count = 1;
    func->v.func.upper = v7->curr_func;
    if (func->v.func.upper != NULL) {
      inc_ref_count(func->v.func.upper);
    }

    if (func_name != NULL) {
      TRY(v7_setv(v7, cur_scope(v7), V7_TYPE_STR, V7_TYPE_OBJ,
                  func_name, func_name_len, 1, func));
    }
  }

  if (!v7->no_exec) {
    // If function didn't have return statement, return UNDEF
    if (!has_return) {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
    }

    // Cleanup arguments
    v7->curr_func->v.func.args = NULL;
    args.ref_count = 1;
    v7_freeval(v7, &args);
  }

  v7->no_exec = old_no_exec;
  return V7_OK;
}

enum v7_err v7_call(struct v7 *v7, struct v7_val *this_obj, int num_args,
                    int called_as_ctor) {
  struct v7_val **top = v7_top(v7), **v = top - (num_args + 1), *f, *res;

  if (v7->no_exec) return V7_OK;
  f = v[0];
  CHECK(v7->sp > num_args, V7_INTERNAL_ERROR);
  CHECK(v7_is_class(f, V7_CLASS_FUNCTION), V7_CALLED_NON_FUNCTION);

  // Push return value on stack
  v7_make_and_push(v7, V7_TYPE_UNDEF);
  res = v7_top(v7)[-1];


  // Stack looks as follows:
  //  v   --->  <called_function>     v[0]
  //            <argument_0>        ---+
  //            <argument_1>           |
  //            <argument_2>           |  <-- num_args
  //            ...                    |
  //            <argument_N>        ---+
  // top  --->  <return_value>
  if (f->flags & V7_JS_FUNC) {
    const char *old_pc = v7->pc;
    struct v7_val *old_curr_func = v7->curr_func;
    int old_line_no = v7->line_no;

    // Move control flow to the function body
    v7->pc = f->v.func.source_code;
    v7->line_no = f->v.func.line_no;
    v7->curr_func = f;

    // Execute function body
    TRY(parse_function_definition(v7, v, num_args));

    // Return control flow back
    v7->pc = old_pc;
    v7->line_no = old_line_no;
    v7->curr_func = old_curr_func;
    CHECK(v7_top(v7) >= top, V7_INTERNAL_ERROR);
  } else {
    struct v7_c_func_arg arg = {
      v7, this_obj, res, v + 1, num_args, called_as_ctor
    };
    f->v.c_func(&arg);
  }
  return V7_OK;
}

static enum v7_err parse_function_call(struct v7 *v7, struct v7_val *this_obj,
                                       int called_as_ctor) {
  struct v7_val **v = v7_top(v7) - 1;
  int num_args = 0;

  //TRACE_OBJ(v[0]);
  CHECK(v7->no_exec || v7_is_class(v[0], V7_CLASS_FUNCTION),
        V7_CALLED_NON_FUNCTION);

  // Push arguments on stack
  TRY(match(v7, '('));
  while (*v7->pc != ')') {
    TRY(parse_expression(v7));
    test_and_skip_char(v7, ',');
    num_args++;
  }
  TRY(match(v7, ')'));

  TRY(v7_call(v7, this_obj, num_args, called_as_ctor));

  return V7_OK;
}

static enum v7_err parse_string_literal(struct v7 *v7) {
  char buf[MAX_STRING_LITERAL_LENGTH];
  const char *begin = v7->pc++;
  struct v7_val *v;
  size_t i = 0;

  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v = v7_top(v7)[-1];

  // Scan string literal into the buffer, handle escape sequences
  while (*v7->pc != *begin && *v7->pc != '\0') {
    switch (*v7->pc) {
      case '\\':
        v7->pc++;
        switch (*v7->pc) {
          // TODO: add escapes for quotes, \XXX, \xXX, \uXXXX
          case 'b': buf[i++] = '\b'; break;
          case 'f': buf[i++] = '\f'; break;
          case 'n': buf[i++] = '\n'; break;
          case 'r': buf[i++] = '\r'; break;
          case 't': buf[i++] = '\t'; break;
          case 'v': buf[i++] = '\v'; break;
          case '\\': buf[i++] = '\\'; break;
          default: if (*v7->pc == *begin) buf[i++] = *begin; break;
        }
        break;
      default:
        buf[i++] = *v7->pc;
        break;
    }
    if (i >= sizeof(buf) - 1) i = sizeof(buf) - 1;
    v7->pc++;
  }
  v7_init_str(v, buf, v7->no_exec ? 0 : i, 1);
  TRY(match(v7, *begin));
  skip_whitespaces_and_comments(v7);

  return V7_OK;
}

enum v7_err v7_append(struct v7 *v7, struct v7_val *arr, struct v7_val *val) {
  struct v7_prop **head, *prop;
  CHECK(v7_is_class(arr, V7_CLASS_ARRAY), V7_INTERNAL_ERROR);
  // Append to the end of the list, to make indexing work
  for (head = &arr->v.array; *head != NULL; head = &head[0]->next);
  prop = mkprop(v7);
  CHECK(prop != NULL, V7_OUT_OF_MEMORY);
  prop->next = *head;
  *head = prop;
  prop->key = NULL;
  prop->val = val;
  inc_ref_count(val);
  return V7_OK;
}

static enum v7_err parse_array_literal(struct v7 *v7) {
  // Push empty array on stack
  if (!v7->no_exec) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    v7_set_class(v7_top(v7)[-1], V7_CLASS_ARRAY);
  }
  TRY(match(v7, '['));

  // Scan array literal, append elements one by one
  while (*v7->pc != ']') {
    // Push new element on stack
    TRY(parse_expression(v7));
    if (!v7->no_exec) {
      TRY(v7_append(v7, v7_top(v7)[-2], v7_top(v7)[-1]));
      TRY(inc_stack(v7, -1));
    }
    test_and_skip_char(v7, ',');
  }

  TRY(match(v7, ']'));
  return V7_OK;
}

static enum v7_err parse_object_literal(struct v7 *v7) {
  // Push empty object on stack
  TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
  TRY(match(v7, '{'));

  // Assign key/values to the object, until closing "}" is found
  while (*v7->pc != '}') {
    // Push key on stack
    if (*v7->pc == '\'' || *v7->pc == '"') {
      TRY(parse_string_literal(v7));
    } else {
      struct v7_val *v;
      TRY(parse_identifier(v7));
      v = v7_mkv(v7, V7_TYPE_STR, v7->tok, v7->tok_len, 1);
      CHECK(v != NULL, V7_OUT_OF_MEMORY);
      TRY(v7_push(v7, v));
    }

    // Push value on stack
    TRY(match(v7, ':'));
    TRY(parse_expression(v7));

    // Stack should now have object, key, value. Assign, and remove key/value
    if (!v7->no_exec) {
      struct v7_val **v = v7_top(v7) - 3;
      CHECK(v[0]->type == V7_TYPE_OBJ, V7_INTERNAL_ERROR);
      TRY(v7_set(v7, v[0], v[1], v[2]));
      TRY(inc_stack(v7, -2));
    }
    test_and_skip_char(v7, ',');
  }
  TRY(match(v7, '}'));
  return V7_OK;
}

enum v7_err v7_del(struct v7 *v7, struct v7_val *obj, struct v7_val *key) {
  struct v7_prop **p;
  CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_ERROR);
  for (p = &obj->props; *p != NULL; p = &p[0]->next) {
    if (cmp(key, p[0]->key) == 0) {
      struct v7_prop *next = p[0]->next;
      free_prop(v7, p[0]);
      p[0] = next;
      break;
    }
  }
  return V7_OK;
}

static enum v7_err parse_delete(struct v7 *v7) {
  struct v7_val key;
  TRY(parse_expression(v7));
  key = str_to_val(v7->tok, v7->tok_len);  // Must go after parse_expression
  TRY(v7_del(v7, v7->cur_obj, &key));
  return V7_OK;
}

static enum v7_err parse_regex(struct v7 *v7) {
  char regex[MAX_STRING_LITERAL_LENGTH];
  size_t i;

  CHECK(*v7->pc == '/', V7_SYNTAX_ERROR);
  for (i = 0, v7->pc++; i < sizeof(regex) - 1 && *v7->pc != '/' &&
    *v7->pc != '\0'; i++, v7->pc++) {
    if (*v7->pc == '\\' && v7->pc[1] == '/') v7->pc++;
    regex[i] = *v7->pc;
  }
  regex[i] = '\0';
  TRY(match(v7, '/'));
  if (!v7->no_exec) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    v7_set_class(v7_top(v7)[-1], V7_CLASS_REGEXP);
    v7_top(v7)[-1]->v.regex = v7_strdup(regex, strlen(regex));
  }

  return V7_OK;
}

static enum v7_err parse_variable(struct v7 *v7) {
  struct v7_val key = str_to_val(v7->tok, v7->tok_len), *v = NULL;
  if (!v7->no_exec) {
    v = find(v7, &key);
    if (v == NULL) {
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
    } else {
      TRY(v7_push(v7, v));
    }
  }
  return V7_OK;
}

static enum v7_err parse_precedence_0(struct v7 *v7) {
  if (*v7->pc == '(') {
    TRY(match(v7, '('));
    TRY(parse_expression(v7));
    TRY(match(v7, ')'));
  } else if (*v7->pc == '\'' || *v7->pc == '"') {
    TRY(parse_string_literal(v7));
  } else if (*v7->pc == '{') {
    TRY(parse_object_literal(v7));
  } else if (*v7->pc == '[') {
    TRY(parse_array_literal(v7));
  } else if (*v7->pc == '/') {
    TRY(parse_regex(v7));
  } else if (is_valid_start_of_identifier(v7->pc[0])) {
    TRY(parse_identifier(v7));
    if (test_token(v7, "this", 4)) {
      TRY(v7_push(v7, v7->this_obj));
    } else if (test_token(v7, "null", 4)) {
      TRY(v7_make_and_push(v7, V7_TYPE_NULL));
    } else if (test_token(v7, "undefined", 9)) {
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
    } else if (test_token(v7, "true", 4)) {
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = 1;
    } else if (test_token(v7, "false", 5)) {
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = 0;
    } else if (test_token(v7, "function", 8)) {
      TRY(parse_function_definition(v7, NULL, 0));
    } else if (test_token(v7, "delete", 6)) {
      TRY(parse_delete(v7));
    } else if (test_token(v7, "NaN", 3)) {
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = NAN;
    } else if (test_token(v7, "Infinity", 8)) {
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = INFINITY;
    } else {
      TRY(parse_variable(v7));
    }
  } else {
    TRY(parse_num(v7));
  }

  return V7_OK;
}


static enum v7_err parse_prop_accessor(struct v7 *v7, int op) {
  struct v7_val *v = NULL, *ns = NULL;

  if (!v7->no_exec) {
    ns = v7_top(v7)[-1];
    v7_make_and_push(v7, V7_TYPE_UNDEF);
    v = v7_top(v7)[-1];
  }
  v7->cur_obj = v7->this_obj = ns;
  CHECK(v7->no_exec || ns != NULL, V7_SYNTAX_ERROR);

  if (op == '.') {
    TRY(parse_identifier(v7));
    if (!v7->no_exec) {
      struct v7_val key = str_to_val(v7->tok, v7->tok_len);
      ns = get2(ns, &key);
      if (ns != NULL && (ns->flags & V7_PROP_FUNC)) {
        ns->v.prop_func(v7->cur_obj, v);
        ns = v;
      }
    }
  } else {
    TRY(parse_expression(v7));
    TRY(match(v7, ']'));
    if (!v7->no_exec) {
      ns = get2(ns, v7_top(v7)[-1]);
      if (ns != NULL && (ns->flags & V7_PROP_FUNC)) {
        ns->v.prop_func(v7->cur_obj, v);
        ns = v;
      }
      TRY(inc_stack(v7, -1));
    }
  }

  if (!v7->no_exec) {
    TRY(v7_push(v7, ns == NULL ? v : ns));
  }

  return V7_OK;
}

static enum v7_err parse_precedence_1(struct v7 *v7, int has_new) {
  struct v7_val *old_this = v7->this_obj;

  TRY(parse_precedence_0(v7));
  while (*v7->pc == '.' || *v7->pc == '[') {
    int op = v7->pc[0];
    TRY(match(v7, op));
    TRY(parse_prop_accessor(v7, op));

    while (*v7->pc == '(') {
      TRY(parse_function_call(v7, v7->cur_obj, has_new));
    }
  }
  v7->this_obj = old_this;

  return V7_OK;
}

static enum v7_err parse_precedence_2(struct v7 *v7) {
  int has_new = 0;
  struct v7_val *old_this_obj = v7->this_obj, *cur_this = v7->this_obj;

  if (lookahead(v7, "new", 3)) {
    has_new++;
    if (!v7->no_exec) {
      v7_make_and_push(v7, V7_TYPE_OBJ);
      cur_this = v7->this_obj = v7_top(v7)[-1];
      v7_set_class(cur_this, V7_CLASS_OBJECT);
    }
  }
  TRY(parse_precedence_1(v7, has_new));
  while (*v7->pc == '(') {
    // Use cur_this, not v7->this_obj: v7->this_obj could have been changed
    TRY(parse_function_call(v7, cur_this, has_new));
  }

  if (has_new && !v7->no_exec) {
    TRY(v7_push(v7, cur_this));
  }

  v7->this_obj = old_this_obj;

  return V7_OK;
}

static enum v7_err parse_precedence_3(struct v7 *v7) {
  TRY(parse_precedence_2(v7));
  if ((v7->pc[0] == '+' && v7->pc[1] == '+') ||
      (v7->pc[0] == '-' && v7->pc[1] == '-')) {
    int increment = (v7->pc[0] == '+') ? 1 : -1;
    v7->pc += 2;
    skip_whitespaces_and_comments(v7);
    if (!v7->no_exec) {
      struct v7_val *v = v7_top(v7)[-1];
      CHECK(v->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      v->v.num += increment;
    }
  }
  return V7_OK;
}

static enum v7_err parse_precedence4(struct v7 *v7) {
  int has_neg = 0, has_typeof = 0;

  if (v7->pc[0] == '!') {
    TRY(match(v7, v7->pc[0]));
    has_neg++;
  }
  has_typeof = lookahead(v7, "typeof", 6);

  TRY(parse_precedence_3(v7));
  if (has_neg && !v7->no_exec) {
    int is_true = v7_is_true(v7_top(v7)[-1]);
    TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
    v7_top(v7)[-1]->v.num = is_true ? 0.0 : 1.0;
  }
  if (has_typeof && !v7->no_exec) {
    const struct v7_val *v = v7_top(v7)[-1];
    static const char *names[] = {
      "undefined", "object", "boolean", "string", "number", "object"
    };
    const char *s = names[v->type];
    if (v7_is_class(v, V7_CLASS_FUNCTION)) s = "function";
    TRY(v7_push(v7, v7_mkv(v7, V7_TYPE_STR, s, strlen(s), 0)));
  }

  return V7_OK;
}

static enum v7_err parse_term(struct v7 *v7) {
  TRY(parse_precedence4(v7));
  while ((*v7->pc == '*' || *v7->pc == '/' || *v7->pc == '%') &&
         v7->pc[1] != '=') {
    int sp1 = v7->sp, ch = *v7->pc;
    TRY(match(v7, ch));
    TRY(parse_precedence4(v7));
    if (!v7->no_exec) {
      TRY(do_arithmetic_op(v7, ch, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static int is_relational_op(const char *s) {
  switch (s[0]) {
    case '>': return s[1] == '=' ? OP_GREATER_EQUAL : OP_GREATER_THEN;
    case '<': return s[1] == '=' ? OP_LESS_EQUAL : OP_LESS_THEN;
    default: return OP_INVALID;
  }
}

static int is_equality_op(const char *s) {
  if (s[0] == '=' && s[1] == '=') {
    return s[2] == '=' ? OP_EQUAL_EQUAL : OP_EQUAL;
  } else if (s[0] == '!' && s[1] == '=') {
    return s[2] == '=' ? OP_NOT_EQUAL_EQUAL : OP_NOT_EQUAL;
  }
  return OP_INVALID;
}

static enum v7_err do_logical_op(struct v7 *v7, int op, int sp1, int sp2) {
  struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7->stack[sp2 - 1];
  int res = 0;

  if (v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM) {
    switch (op) {
      case OP_GREATER_THEN:   res = v1->v.num >  v2->v.num; break;
      case OP_GREATER_EQUAL:  res = v1->v.num >= v2->v.num; break;
      case OP_LESS_THEN:      res = v1->v.num <  v2->v.num; break;
      case OP_LESS_EQUAL:     res = v1->v.num <= v2->v.num; break;
      case OP_EQUAL: // FALLTHROUGH
      case OP_EQUAL_EQUAL:    res = cmp(v1, v2) == 0; break;
      case OP_NOT_EQUAL: // FALLTHROUGH
      case OP_NOT_EQUAL_EQUAL:  res = cmp(v1, v2) != 0; break;
    }
  } else if (op == OP_EQUAL || op == OP_EQUAL_EQUAL) {
    res = cmp(v1, v2) == 0;
  } else if (op == OP_NOT_EQUAL || op == OP_NOT_EQUAL_EQUAL) {
    res = cmp(v1, v2) != 0;
  }
  TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
  v7_top(v7)[-1]->v.num = res ? 1.0 : 0.0;
  return V7_OK;
}

static enum v7_err parse_assign(struct v7 *v7, struct v7_val *obj, int op) {
  const char *tok = v7->tok;
  unsigned long tok_len = v7->tok_len;

  v7->pc += s_op_lengths[op];
  skip_whitespaces_and_comments(v7);
  TRY(parse_expression(v7));

  // Stack layout at this point (assuming stack grows down):
  //
  //          | object's value (rvalue)    |    top[-2]
  //          +----------------------------+
  //          | expression value (lvalue)  |    top[-1]
  //          +----------------------------+
  // top -->  |       nothing yet          |
  if (!v7->no_exec) {
    struct v7_val **top = v7_top(v7), *a = top[-2], *b = top[-1];
    switch (op) {
      case OP_ASSIGN:
        CHECK(v7->sp >= 2, V7_INTERNAL_ERROR);
        TRY(v7_setv(v7, obj, V7_TYPE_STR, V7_TYPE_OBJ, tok, tok_len, 1, b));
        return V7_OK;
      case OP_PLUS_ASSIGN: TRY(arith(a, b, a, '+')); break;
      case OP_MINUS_ASSIGN: TRY(arith(a, b, a, '-')); break;
      case OP_MUL_ASSIGN: TRY(arith(a, b, a, '*')); break;
      case OP_DIV_ASSIGN: TRY(arith(a, b, a, '/')); break;
      case OP_REM_ASSIGN: TRY(arith(a, b, a, '%')); break;
      case OP_XOR_ASSIGN: TRY(arith(a, b, a, '^')); break;
      default: return V7_NOT_IMPLEMENTED;
    }
    TRY(inc_stack(v7, -1));
  }

  return V7_OK;
}

static enum v7_err parse_add_sub(struct v7 *v7) {
  TRY(parse_term(v7));
  while ((*v7->pc == '-' || *v7->pc == '+') && v7->pc[1] != '=') {
    int sp1 = v7->sp, ch = *v7->pc;
    TRY(match(v7, ch));
    TRY(parse_term(v7));
    if (!v7->no_exec) {
      TRY(do_arithmetic_op(v7, ch, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err parse_relational(struct v7 *v7) {
  int op;
  TRY(parse_add_sub(v7));
  while ((op = is_relational_op(v7->pc)) > OP_INVALID) {
    int sp1 = v7->sp;
    v7->pc += s_op_lengths[op];
    skip_whitespaces_and_comments(v7);
    TRY(parse_add_sub(v7));
    if (!v7->no_exec) {
      TRY(do_logical_op(v7, op, sp1, v7->sp));
    }
  }
  if (lookahead(v7, "instanceof", 10)) {
    TRY(parse_identifier(v7));
    if (!v7->no_exec) {
      struct v7_val key = str_to_val(v7->tok, v7->tok_len);
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = instanceof(v7_top(v7)[-2], find(v7, &key));
    }
  }
  return V7_OK;
}

static enum v7_err parse_equality(struct v7 *v7) {
  int op;
  TRY(parse_relational(v7));
  if ((op = is_equality_op(v7->pc)) > OP_INVALID) {
    int sp1 = v7->sp;
    v7->pc += s_op_lengths[op];
    skip_whitespaces_and_comments(v7);
    TRY(parse_relational(v7));
    if (!v7->no_exec) {
      TRY(do_logical_op(v7, op, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_and(struct v7 *v7) {
  TRY(parse_equality(v7));
  if (*v7->pc == '&' && v7->pc[1] != '&' && v7->pc[1] != '=') {
    int sp1 = v7->sp;
    TRY(match(v7, '&'));
    TRY(parse_equality(v7));
    if (!v7->no_exec) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = v1->v.num, b = v2->v.num;
      CHECK(v1->type == V7_TYPE_NUM && v1->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a & b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_xor(struct v7 *v7) {
  TRY(parse_bitwise_and(v7));
  if (*v7->pc == '^' && v7->pc[1] != '=') {
    int sp1 = v7->sp;
    TRY(match(v7, '^'));
    TRY(parse_bitwise_and(v7));
    if (!v7->no_exec) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = v1->v.num, b = v2->v.num;
      CHECK(v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a ^ b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_or(struct v7 *v7) {
  TRY(parse_bitwise_xor(v7));
  if (*v7->pc == '|' && v7->pc[1] != '=' && v7->pc[1] != '|') {
    int sp1 = v7->sp;
    TRY(match(v7, '|'));
    TRY(parse_bitwise_xor(v7));
    if (!v7->no_exec) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = v1->v.num, b = v2->v.num;
      CHECK(v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a | b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_logical_and(struct v7 *v7) {
  TRY(parse_bitwise_or(v7));
  while (*v7->pc == '&' && v7->pc[1] == '&') {
    int sp1 = v7->sp;
    match(v7, '&');
    match(v7, '&');
    TRY(parse_bitwise_or(v7));
    if (!v7->no_exec) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      int is_true = v7_is_true(v1) && v7_is_true(v2);
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = is_true ? 1.0 : 0.0;
    }
  }
  return V7_OK;
}

static enum v7_err parse_logical_or(struct v7 *v7) {
  TRY(parse_logical_and(v7));
  if (*v7->pc == '|' && v7->pc[1] == '|') {
    int sp1 = v7->sp;
    match(v7, '|');
    match(v7, '|');
    TRY(parse_logical_and(v7));
    if (!v7->no_exec) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      int is_true = v7_is_true(v1) || v7_is_true(v2);
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = is_true ? 1.0 : 0.0;
    }
  }
  return V7_OK;
}

static int is_assign_op(const char *s) {
  if (s[0] == '=') {
    return OP_ASSIGN;
  } else if (s[1] == '=') {
    switch (s[0]) {
      case '+': return OP_PLUS_ASSIGN;
      case '-': return OP_MINUS_ASSIGN;
      case '*': return OP_MUL_ASSIGN;
      case '/': return OP_DIV_ASSIGN;
      case '%': return OP_REM_ASSIGN;
      case '&': return OP_AND_ASSIGN;
      case '^': return OP_XOR_ASSIGN;
      case '|': return OP_OR_ASSIGN;
      default: return OP_INVALID;
    }
  } else if (s[0] == '<' && s[1] == '<' && s[2] == '=') {
    return OP_LSHIFT_ASSIGN;
  } else if (s[0] == '>' && s[1] == '>' && s[2] == '=') {
    return OP_RSHIFT_ASSIGN;
  } else if (s[0] == '>' && s[1] == '>' && s[2] == '>' && s[3] == '=') {
    return OP_RRSHIFT_ASSIGN;
  } else {
    return OP_INVALID;
  }
}

static enum v7_err parse_expression(struct v7 *v7) {
#ifdef V7_DEBUG
  const char *stmt_str = v7->pc;
#endif
  int op, old_sp = v7->sp;

  v7->cur_obj = cur_scope(v7);
  TRY(parse_logical_or(v7));

  // Parse assignment
  if ((op = is_assign_op(v7->pc))) {
    TRY(parse_assign(v7, v7->cur_obj, op));
  }

  // Parse ternary operator
  if (*v7->pc == '?') {
    int old_no_exec = v7->no_exec;
    int condition_true = 1;

    if (!v7->no_exec) {
      CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
      condition_true = v7_is_true(v7_top(v7)[-1]);
      TRY(inc_stack(v7, -1));   // Remove condition result
    }

    TRY(match(v7, '?'));
    v7->no_exec = old_no_exec || !condition_true;
    TRY(parse_expression(v7));
    TRY(match(v7, ':'));
    v7->no_exec = old_no_exec || condition_true;
    TRY(parse_expression(v7));
    v7->no_exec = old_no_exec;
  }

  // Collapse stack, leave only one value on top
  if (!v7->no_exec) {
    struct v7_val *result = v7_top(v7)[-1];
    inc_ref_count(result);
    TRY(inc_stack(v7, old_sp - v7->sp));
    TRY(v7_push(v7, result));
    assert(result->ref_count > 1);
    v7_freeval(v7, result);
  }

  return V7_OK;
}

static enum v7_err parse_declaration(struct v7 *v7) {
  int sp = v7_sp(v7);

  do {
    inc_stack(v7, sp - v7_sp(v7));  // Clean up the stack after prev decl
    TRY(parse_identifier(v7));
    if (v7->no_exec) {
      v7_setv(v7, cur_scope(v7), V7_TYPE_STR, V7_TYPE_UNDEF,
              v7->tok, v7->tok_len, 1);
    }
    if (*v7->pc == '=') {
      if (!v7->no_exec) v7_make_and_push(v7, V7_TYPE_UNDEF);
      TRY(parse_assign(v7, cur_scope(v7), OP_ASSIGN));
    }
  } while (test_and_skip_char(v7, ','));

  return V7_OK;
}

static enum v7_err parse_if_statement(struct v7 *v7, int *has_return) {
  int old_no_exec = v7->no_exec;  // Remember execution flag

  TRY(match(v7, '('));
  TRY(parse_expression(v7));      // Evaluate condition, pushed on stack
  TRY(match(v7, ')'));
  if (!old_no_exec) {
    // If condition is false, do not execute "if" body
    CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
    v7->no_exec = !v7_is_true(v7_top(v7)[-1]);
    TRY(inc_stack(v7, -1));   // Cleanup condition result from the stack
  }
  TRY(parse_compound_statement(v7, has_return));

  if (strncmp(v7->pc, "else", 4) == 0) {
    v7->pc += 4;
    skip_whitespaces_and_comments(v7);
    v7->no_exec = old_no_exec || !v7->no_exec;
    TRY(parse_compound_statement(v7, has_return));
  }

  v7->no_exec = old_no_exec;  // Restore old execution flag
  return V7_OK;
}

static enum v7_err parse_for_in_statement(struct v7 *v7, int has_var,
                                          int *has_return) {
  const char *tok = v7->tok, *stmt;
  int tok_len = v7->tok_len, line_stmt;

  TRY(parse_expression(v7));
  TRY(match(v7, ')'));
  stmt = v7->pc;
  line_stmt = v7->line_no;

  // Execute loop body
  if (v7->no_exec) {
    TRY(parse_compound_statement(v7, has_return));
  } else {
    int old_sp = v7->sp;
    struct v7_val *obj = v7_top(v7)[-1];
    struct v7_val *scope = has_var ? cur_scope(v7) : &v7->root_scope;
    struct v7_prop *prop;

    CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_ERROR);
    for (prop = obj->props; prop != NULL; prop = prop->next) {
      TRY(v7_setv(v7, scope, V7_TYPE_STR, V7_TYPE_OBJ,
                  tok, tok_len, 1, prop->key));
      v7->pc = stmt;
      v7->line_no = line_stmt;
      TRY(parse_compound_statement(v7, has_return));  // Loop body
      TRY(inc_stack(v7, old_sp - v7->sp));  // Clean up stack
    }
  }

  return V7_OK;
}

static enum v7_err parse_for_statement(struct v7 *v7, int *has_return) {
  int line_expr1, line_expr2, line_expr3, line_stmt, line_end,
    is_true, old_no_exec = v7->no_exec, has_var = 0;
  const char *expr1, *expr2, *expr3, *stmt, *end;

  TRY(match(v7, '('));
  expr1 = v7->pc;
  line_expr1 = v7->line_no;

  // See if this is an enumeration loop
  if (lookahead(v7, "var", 3)) {
    has_var = 1;
  }
  if (parse_identifier(v7) == V7_OK && lookahead(v7, "in", 2)) {
    return parse_for_in_statement(v7, has_var, has_return);
  } else {
    v7->pc = expr1;
    v7->line_no = line_expr1;
  }

  if (lookahead(v7, "var", 3)) {
    parse_declaration(v7);
  } else {
    TRY(parse_expression(v7));    // expr1
  }
  TRY(match(v7, ';'));

  // Pass through the loop, don't execute it, just remember locations
  v7->no_exec = 1;
  expr2 = v7->pc;
  line_expr2 = v7->line_no;
  TRY(parse_expression(v7));    // expr2 (condition)
  TRY(match(v7, ';'));

  expr3 = v7->pc;
  line_expr3 = v7->line_no;
  TRY(parse_expression(v7));    // expr3  (post-iteration)
  TRY(match(v7, ')'));

  stmt = v7->pc;
  line_stmt = v7->line_no;
  TRY(parse_compound_statement(v7, has_return));
  end = v7->pc;
  line_end = v7->line_no;

  v7->no_exec = old_no_exec;

  // Execute loop
  if (!v7->no_exec) {
    int old_sp = v7->sp;
    for (;;) {
      v7->pc = expr2;
      v7->line_no = line_expr2;
      TRY(parse_expression(v7));    // Evaluate condition
      is_true = !v7_is_true(v7_top(v7)[-1]);
      TRY(inc_stack(v7, -1));
      if (is_true) break;

      v7->pc = stmt;
      v7->line_no = line_stmt;
      TRY(parse_compound_statement(v7, has_return));  // Loop body

      v7->pc = expr3;
      v7->line_no = line_expr3;
      TRY(parse_expression(v7));    // expr3  (post-iteration)

      TRY(inc_stack(v7, old_sp - v7->sp));  // Clean up stack
    }
  }

  // Jump to the code after the loop
  v7->line_no = line_end;
  v7->pc = end;

  return V7_OK;
}

static enum v7_err parse_return_statement(struct v7 *v7, int *has_return) {
  if (!v7->no_exec) {
    *has_return = 1;
  }
  if (*v7->pc != ';' && *v7->pc != '}') {
    TRY(parse_expression(v7));
  }
  return V7_OK;
}

static enum v7_err parse_try_statement(struct v7 *v7, int *has_return) {
  enum v7_err err_code;
  const char *old_pc = v7->pc;
  int old_no_exec = v7->no_exec, old_line_no = v7->line_no;

  CHECK(v7->pc[0] == '{', V7_SYNTAX_ERROR);
  err_code = parse_compound_statement(v7, has_return);

  if (old_no_exec && err_code != V7_OK) {
    return err_code;
  }

  // If exception has happened, skip the block
  if (err_code != V7_OK) {
    v7->pc = old_pc;
    v7->line_no = old_line_no;
    v7->no_exec = 1;
    TRY(parse_compound_statement(v7, has_return));
  }

  TRY(parse_identifier(v7));
  if (test_token(v7, "catch", 5)) {
    TRY(match(v7, '('));
    TRY(parse_identifier(v7));
    TRY(match(v7, ')'));

    // Insert error variable into the namespace
    if (err_code != V7_OK) {
      TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
      v7_set_class(v7_top_val(v7), V7_CLASS_ERROR);
      v7_setv(v7, cur_scope(v7), V7_TYPE_STR, V7_TYPE_OBJ,
              v7->tok, v7->tok_len, v7_top_val(v7));
    }

    v7->no_exec = old_no_exec || err_code == V7_OK;
    TRY(parse_compound_statement(v7, has_return));
    v7->no_exec = old_no_exec;

    if (lookahead(v7, "finally", 7)) {
      TRY(parse_compound_statement(v7, has_return));
    }
  } else if (test_token(v7, "finally", 7)) {
    v7->no_exec = old_no_exec;
    TRY(parse_compound_statement(v7, has_return));
  } else {
    v7->no_exec = old_no_exec;
    return V7_SYNTAX_ERROR;
  }
  return V7_OK;
}

static enum v7_err parse_statement(struct v7 *v7, int *has_return) {
  if (lookahead(v7, "var", 3)) {
    TRY(parse_declaration(v7));
  } else if (lookahead(v7, "return", 6)) {
    TRY(parse_return_statement(v7, has_return));
  } else if (lookahead(v7, "if", 2)) {
    TRY(parse_if_statement(v7, has_return));
  } else if (lookahead(v7, "for", 3)) {
    TRY(parse_for_statement(v7, has_return));
  } else if (lookahead(v7, "try", 3)) {
    TRY(parse_try_statement(v7, has_return));
  } else {
    TRY(parse_expression(v7));
  }

  // Skip optional colons and semicolons
  while (*v7->pc == ',') match(v7, *v7->pc);
  while (*v7->pc == ';') match(v7, *v7->pc);
  return V7_OK;
}

static enum v7_err do_exec(struct v7 *v7, const char *source_code, int sp) {
  int has_ret = 0;

  v7->source_code = v7->pc = source_code;
  skip_whitespaces_and_comments(v7);

  // Prior calls to v7_exec() may have left current_scope modified, reset now
  // TODO(lsm): free scope chain
  v7->this_obj = &v7->root_scope;

  while (*v7->pc != '\0') {
    TRY(inc_stack(v7, sp - v7->sp));      // Reset stack on each statement
    TRY(parse_statement(v7, &has_ret));   // Last expr result on stack
  }
  assert(v7->root_scope.proto == &s_global);

  return V7_OK;
}

enum v7_err v7_exec(struct v7 *v7, const char *source_code) {
  return do_exec(v7, source_code, 0);
}

enum v7_err v7_exec_file(struct v7 *v7, const char *path) {
  FILE *fp;
  char *p, *old_pc = (char *) v7->pc;
  long file_size, old_line_no = v7->line_no;
  enum v7_err status = V7_INTERNAL_ERROR;

  if ((fp = fopen(path, "r")) == NULL) {
  } else if (fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0) {
    fclose(fp);
  } else if ((p = (char *) calloc(1, (size_t) file_size + 1)) == NULL) {
    fclose(fp);
  } else {
    rewind(fp);
    fread(p, 1, (size_t) file_size, fp);
    fclose(fp);
    v7->line_no = 1;
    status = do_exec(v7, p, v7->sp);
    free(p);
    v7->pc = old_pc;
    if (status == V7_OK) v7->line_no = (int) old_line_no;
  }

  return status;
}
