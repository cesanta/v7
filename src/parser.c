#include "internal.h"

enum {
  OP_INVALID,

  // Relational ops
  OP_GREATER_THEN,    //  >
  OP_LESS_THEN,       //  <
  OP_GREATER_EQUAL,   //  >=
  OP_LESS_EQUAL,      //  <=

  // Equality ops
  OP_EQUAL,           //  ==
  OP_NOT_EQUAL,       //  !=
  OP_EQUAL_EQUAL,     //  ===
  OP_NOT_EQUAL_EQUAL, //  !==

  // Assignment ops
  OP_ASSIGN,          //  =
  OP_PLUS_ASSIGN,     //  +=
  OP_MINUS_ASSIGN,    //  -=
  OP_MUL_ASSIGN,      //  *=
  OP_DIV_ASSIGN,      //  /=
  OP_REM_ASSIGN,      //  %=
  OP_AND_ASSIGN,      //  &=
  OP_XOR_ASSIGN,      //  ^=
  OP_OR_ASSIGN,       //  |=
  OP_RSHIFT_ASSIGN,   //  >>=
  OP_LSHIFT_ASSIGN,   //  <<=
  OP_RRSHIFT_ASSIGN,  //  >>>=

  NUM_OPS
};

static const int s_op_lengths[NUM_OPS] = {
  -1,
  1, 1, 2, 2,
  2, 2, 3, 3,
  1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4
};

static enum v7_err arith(struct v7 *v7, struct v7_val *a, struct v7_val *b,
                         struct v7_val *res, int op) {
  char *str;

  if (a->type == V7_TYPE_STR && op == '+') {
    if (b->type != V7_TYPE_STR) {
      // Do type conversion, result pushed on stack
      TRY(toString(v7, b));
      b = v7_top_val(v7);
    }
    str = (char *) malloc(a->v.str.len + b->v.str.len + 1);
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
  struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7->stack[sp2 - 1];
  int sp;

  assert(EXECUTING(v7->flags));
  CHECK(v7->sp >= 2, V7_STACK_UNDERFLOW);
  TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
  sp = v7->sp;
  TRY(arith(v7, v1, v2, v7_top_val(v7), op));

  // arith() might push another value on stack if type conversion was made.
  // if that happens, re-push the result again
  if (v7->sp > sp) {
    TRY(v7_push(v7, v7->stack[sp - 1]));
  }

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

V7_PRIVATE void skip_whitespaces_and_comments(struct v7 *v7) {
  const char *s = v7->pstate.pc, *p = NULL;
  while (s != p && *s != '\0' && (is_space(*s) || *s == '/')) {
    p = s;
    while (*s != '\0' && is_space(*s)) {
      if (*s == '\n') v7->pstate.line_no++;
      s++;
    }
    if (s[0] == '/' && s[1] == '/') {
      s += 2;
      while (s[0] != '\0' && s[0] != '\n') s++;
    }
    if (s[0] == '/' && s[1] == '*') {
      s += 2;
      while (s[0] != '\0' && !(s[-1] == '/' && s[-2] == '*')) {
        if (s[0] == '\n') v7->pstate.line_no++;
        s++;
      }
    }
  }
  v7->pstate.pc = s;
}

static enum v7_err match(struct v7 *v7, int ch) {
  CHECK(*v7->pstate.pc++ == ch, V7_SYNTAX_ERROR);
  skip_whitespaces_and_comments(v7);
  return V7_OK;
}

static int test_and_skip_char(struct v7 *v7, int ch) {
  if (*v7->pstate.pc == ch) {
    v7->pstate.pc++;
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

  value = strtod(v7->pstate.pc, &end);
  // Handle case like 123.toString()
  if (end != NULL && (v7->pstate.pc < &end[-1]) && end[-1] == '.') end--;
  CHECK(value != 0 || end > v7->pstate.pc, V7_SYNTAX_ERROR);
  v7->pstate.pc = end;
  v7->tok_len = (unsigned long) (v7->pstate.pc - v7->tok);
  skip_whitespaces_and_comments(v7);

  if (EXECUTING(v7->flags)) {
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
  CHECK(is_valid_start_of_identifier(v7->pstate.pc[0]), V7_SYNTAX_ERROR);
  v7->tok = v7->pstate.pc;
  v7->pstate.pc++;
  while (is_valid_identifier_char(v7->pstate.pc[0])) v7->pstate.pc++;
  v7->tok_len = (unsigned long) (v7->pstate.pc - v7->tok);
  skip_whitespaces_and_comments(v7);
  return V7_OK;
}

static int lookahead(struct v7 *v7, const char *str, int str_len) {
  int equal = 0;
  if (memcmp(v7->pstate.pc, str, str_len) == 0 &&
      !is_valid_identifier_char(v7->pstate.pc[str_len])) {
    equal++;
    v7->pstate.pc += str_len;
    skip_whitespaces_and_comments(v7);
  }
  return equal;
}

static enum v7_err parse_compound_statement(struct v7 *v7, int *has_return) {
  if (*v7->pstate.pc == '{') {
    int old_sp = v7->sp;
    TRY(match(v7, '{'));
    while (*v7->pstate.pc != '}') {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(parse_statement(v7, has_return));
      if (*has_return && EXECUTING(v7->flags)) return V7_OK;
    }
    TRY(match(v7, '}'));
  } else {
    TRY(parse_statement(v7, has_return));
  }
  return V7_OK;
}

static enum v7_err parse_function_definition(struct v7 *v7, struct v7_val **v,
                                             int num_params) { // <#fdef#>
  int i = 0, old_flags = v7->flags, old_sp = v7->sp, has_ret = 0;
  unsigned long func_name_len = 0;
  const char *src = v7->pstate.pc, *func_name = NULL;
  struct v7_val *ctx = NULL, *f = NULL;

  if (*v7->pstate.pc != '(') {
    // function name is given, e.g. function foo() {}
    CHECK(v == NULL, V7_SYNTAX_ERROR);
    TRY(parse_identifier(v7));
    func_name = v7->tok;
    func_name_len = v7->tok_len;
    src = v7->pstate.pc;
  }

  // 1. SCANNING: do nothing, just pass through the function code
  // 2. EXECUTING && v == 0: don't execute but create a closure
  // 3. EXECUTING && v != 0: execute the closure

  if (EXECUTING(v7->flags) && v == NULL) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    f = v7_top_val(v7);
    v7_set_class(f, V7_CLASS_FUNCTION);
    f->flags |= V7_JS_FUNC;

    f->v.func.source_code = (char *) v7->pstate.pc;
    f->v.func.line_no = v7->pstate.line_no;

    f->v.func.var_obj = v7->ctx;
    inc_ref_count(v7->ctx);

    v7->flags |= V7_NO_EXEC;
  } else if (EXECUTING(v7->flags) && v != NULL) {
    f = v[0];
    assert(v7_is_class(f, V7_CLASS_FUNCTION));

    f->next = v7->cf;
    v7->cf = f;

    ctx = make_value(v7, V7_TYPE_OBJ);
    v7_set_class(ctx, V7_CLASS_OBJECT);
    inc_ref_count(ctx);

    ctx->next = v7->ctx;
    v7->ctx = ctx;
  }

  // Add function arguments to the variable object
  TRY(match(v7, '('));
  while (*v7->pstate.pc != ')') {
    TRY(parse_identifier(v7));
    if (EXECUTING(v7->flags)) {
      struct v7_val *val = i < num_params ? v[i + 1] : make_value(v7, V7_TYPE_UNDEF);
      TRY(v7_setv(v7, ctx, V7_TYPE_STR, V7_TYPE_OBJ,
                  v7->tok, v7->tok_len, 1, val));
    }
    i++;
    if (!test_and_skip_char(v7, ',')) break;
  }
  TRY(match(v7, ')'));

  // Execute (or pass) function body
  TRY(parse_compound_statement(v7, &has_ret));

  // Add function to the namespace for notation "function x(y,z) { ... } "
  if (EXECUTING(old_flags) && v == NULL && func_name != NULL) {
    TRY(v7_setv(v7, v7->ctx, V7_TYPE_STR, V7_TYPE_OBJ,
                func_name, func_name_len, 1, f));
  }

  if (EXECUTING(v7->flags)) {
    // Cleanup execution context
    v7->ctx = ctx->next;
    ctx->next = NULL;
    //assert(f->v.func.var_obj == NULL);
    //f->v.func.var_obj = ctx;
    v7_freeval(v7, ctx);

    v7->cf = f->next;
    f->next = NULL;

    // If function didn't have return statement, return UNDEF
    if (!has_ret) {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
    }
  }

  v7->flags = old_flags;

  return V7_OK;
}

enum v7_err v7_call(struct v7 *v7, struct v7_val *this_obj, int num_args,
                    int called_as_ctor) {
  struct v7_val **top = v7_top(v7), **v = top - (num_args + 1), *f, *res;

  if (!EXECUTING(v7->flags)) return V7_OK;
  f = v[0];
  CHECK(v7->sp > num_args, V7_INTERNAL_ERROR);
  CHECK(f != NULL, V7_TYPE_ERROR);
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
    struct v7_pstate old_pstate = v7->pstate;

    // Move control flow to the function body
    v7->pstate.pc = f->v.func.source_code;
    v7->pstate.line_no = f->v.func.line_no;

    // Execute function body
    TRY(parse_function_definition(v7, v, num_args));

    // Return control flow back
    v7->pstate = old_pstate;
    CHECK(v7_top(v7) >= top, V7_INTERNAL_ERROR);
  } else {
    struct v7_c_func_arg arg = {
      v7, this_obj, res, v + 1, num_args, called_as_ctor
    };
    TRY(f->v.c_func(&arg));
  }
  return V7_OK;
}

static enum v7_err parse_function_call(struct v7 *v7, struct v7_val *this_obj,
                                       int called_as_ctor) {
  struct v7_val **v = v7_top(v7) - 1;
  int num_args = 0;

  //TRACE_OBJ(v[0]);
  CHECK(!EXECUTING(v7->flags) || v7_is_class(v[0], V7_CLASS_FUNCTION),
        V7_CALLED_NON_FUNCTION);

  // Push arguments on stack
  TRY(match(v7, '('));
  while (*v7->pstate.pc != ')') {
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
  const char *begin = v7->pstate.pc++;
  struct v7_val *v;
  size_t i = 0;

  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v = v7_top(v7)[-1];

  // Scan string literal into the buffer, handle escape sequences
  while (*v7->pstate.pc != *begin && *v7->pstate.pc != '\0') {
    switch (*v7->pstate.pc) {
      case '\\':
        v7->pstate.pc++;
        switch (*v7->pstate.pc) {
          // TODO: add escapes for quotes, \XXX, \xXX, \uXXXX
          case 'b': buf[i++] = '\b'; break;
          case 'f': buf[i++] = '\f'; break;
          case 'n': buf[i++] = '\n'; break;
          case 'r': buf[i++] = '\r'; break;
          case 't': buf[i++] = '\t'; break;
          case 'v': buf[i++] = '\v'; break;
          case '\\': buf[i++] = '\\'; break;
          default: if (*v7->pstate.pc == *begin) buf[i++] = *begin; break;
        }
        break;
      default:
        buf[i++] = *v7->pstate.pc;
        break;
    }
    if (i >= sizeof(buf) - 1) i = sizeof(buf) - 1;
    v7->pstate.pc++;
  }
  v7_init_str(v, buf, !EXECUTING(v7->flags) ? 0 : i, 1);
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
  if (EXECUTING(v7->flags)) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    v7_set_class(v7_top(v7)[-1], V7_CLASS_ARRAY);
  }
  TRY(match(v7, '['));

  // Scan array literal, append elements one by one
  while (*v7->pstate.pc != ']') {
    // Push new element on stack
    TRY(parse_expression(v7));
    if (EXECUTING(v7->flags)) {
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
  while (*v7->pstate.pc != '}') {
    // Push key on stack
    if (*v7->pstate.pc == '\'' || *v7->pstate.pc == '"') {
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
    if (EXECUTING(v7->flags)) {
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

  CHECK(*v7->pstate.pc == '/', V7_SYNTAX_ERROR);
  for (i = 0, v7->pstate.pc++; i < sizeof(regex) - 1 && *v7->pstate.pc != '/' &&
    *v7->pstate.pc != '\0'; i++, v7->pstate.pc++) {
    if (*v7->pstate.pc == '\\' && v7->pstate.pc[1] == '/') v7->pstate.pc++;
    regex[i] = *v7->pstate.pc;
  }
  regex[i] = '\0';
  CHECK(*v7->pstate.pc++ == '/', V7_SYNTAX_ERROR);
  uint8_t fl_g=0, fl_i=0, fl_m=0, rep;
  do{
    switch(*v7->pstate.pc){
      case 'g': fl_g=1; rep=1; break;
      case 'i': fl_i=1; rep=1; break;
      case 'm': fl_m=1; rep=1; break;
      default :         rep=0; break;
    }
    v7->pstate.pc += rep;
  }while(rep);
  skip_whitespaces_and_comments(v7);
  if (EXECUTING(v7->flags)) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    struct v7_val *top = v7_top(v7)[-1];
    v7_set_class(top, V7_CLASS_REGEXP);
    top->v.regex = v7_strdup(regex, strlen(regex));
    top->regexp_fl_g = fl_g;
    top->regexp_fl_i = fl_i;
    top->regexp_fl_m = fl_m;
  }

  return V7_OK;
}

static enum v7_err parse_variable(struct v7 *v7) {
  struct v7_val key = str_to_val(v7->tok, v7->tok_len), *v = NULL;
  if (EXECUTING(v7->flags)) {
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
  if (*v7->pstate.pc == '(') {
    TRY(match(v7, '('));
    TRY(parse_expression(v7));
    TRY(match(v7, ')'));
  } else if (*v7->pstate.pc == '\'' || *v7->pstate.pc == '"') {
    TRY(parse_string_literal(v7));
  } else if (*v7->pstate.pc == '{') {
    TRY(parse_object_literal(v7));
  } else if (*v7->pstate.pc == '[') {
    TRY(parse_array_literal(v7));
  } else if (*v7->pstate.pc == '/') {
    TRY(parse_regex(v7));
  } else if (is_valid_start_of_identifier(v7->pstate.pc[0])) {
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

  if (EXECUTING(v7->flags)) {
    ns = v7_top(v7)[-1];
    v7_make_and_push(v7, V7_TYPE_UNDEF);
    v = v7_top(v7)[-1];
    v7->cur_obj = v7->this_obj = ns;
  }
  CHECK(!EXECUTING(v7->flags) || ns != NULL, V7_SYNTAX_ERROR);

  if (op == '.') {
    TRY(parse_identifier(v7));
    if (EXECUTING(v7->flags)) {
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
    if (EXECUTING(v7->flags)) {
      ns = get2(ns, v7_top(v7)[-1]);
      if (ns != NULL && (ns->flags & V7_PROP_FUNC)) {
        ns->v.prop_func(v7->cur_obj, v);
        ns = v;
      }
      TRY(inc_stack(v7, -1));
    }
  }

  if (EXECUTING(v7->flags)) {
    TRY(v7_push(v7, ns == NULL ? v : ns));
  }

  return V7_OK;
}

static enum v7_err parse_precedence_1(struct v7 *v7, int has_new) {
  struct v7_val *old_this = v7->this_obj;

  TRY(parse_precedence_0(v7));
  while (*v7->pstate.pc == '.' || *v7->pstate.pc == '[') {
    int op = v7->pstate.pc[0];
    TRY(match(v7, op));
    TRY(parse_prop_accessor(v7, op));

    while (*v7->pstate.pc == '(') {
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
    if (EXECUTING(v7->flags)) {
      v7_make_and_push(v7, V7_TYPE_OBJ);
      cur_this = v7->this_obj = v7_top(v7)[-1];
      v7_set_class(cur_this, V7_CLASS_OBJECT);
    }
  }
  TRY(parse_precedence_1(v7, has_new));
  while (*v7->pstate.pc == '(') {
    // Use cur_this, not v7->this_obj: v7->this_obj could have been changed
    TRY(parse_function_call(v7, cur_this, has_new));
  }

  if (has_new && EXECUTING(v7->flags)) {
    TRY(v7_push(v7, cur_this));
  }

  v7->this_obj = old_this_obj;

  return V7_OK;
}

static enum v7_err parse_precedence_3(struct v7 *v7) {
  TRY(parse_precedence_2(v7));
  if ((v7->pstate.pc[0] == '+' && v7->pstate.pc[1] == '+') ||
      (v7->pstate.pc[0] == '-' && v7->pstate.pc[1] == '-')) {
    int increment = (v7->pstate.pc[0] == '+') ? 1 : -1;
    v7->pstate.pc += 2;
    skip_whitespaces_and_comments(v7);
    if (EXECUTING(v7->flags)) {
      struct v7_val *v = v7_top(v7)[-1];
      CHECK(v->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      v->v.num += increment;
    }
  }
  return V7_OK;
}

static enum v7_err parse_precedence4(struct v7 *v7) {
  int has_neg = 0, has_typeof = 0;

  if (v7->pstate.pc[0] == '!') {
    TRY(match(v7, v7->pstate.pc[0]));
    has_neg++;
  }
  has_typeof = lookahead(v7, "typeof", 6);

  TRY(parse_precedence_3(v7));
  if (has_neg && EXECUTING(v7->flags)) {
    int is_true = v7_is_true(v7_top(v7)[-1]);
    TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
    v7_top(v7)[-1]->v.num = is_true ? 0.0 : 1.0;
  }
  if (has_typeof && EXECUTING(v7->flags)) {
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
  while ((*v7->pstate.pc == '*' || *v7->pstate.pc == '/' || *v7->pstate.pc == '%') &&
         v7->pstate.pc[1] != '=') {
    int sp1 = v7->sp, ch = *v7->pstate.pc;
    TRY(match(v7, ch));
    TRY(parse_precedence4(v7));
    if (EXECUTING(v7->flags)) {
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

  v7->pstate.pc += s_op_lengths[op];
  skip_whitespaces_and_comments(v7);
  TRY(parse_expression(v7));

  // Stack layout at this point (assuming stack grows down):
  //
  //          | object's value (rvalue)    |    top[-2]
  //          +----------------------------+
  //          | expression value (lvalue)  |    top[-1]
  //          +----------------------------+
  // top -->  |       nothing yet          |

  //<#parse_assign#>
  if (EXECUTING(v7->flags)) {
    struct v7_val **top = v7_top(v7), *a = top[-2], *b = top[-1];

    switch (op) {
      case OP_ASSIGN:
        CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
        TRY(v7_setv(v7, obj, V7_TYPE_STR, V7_TYPE_OBJ, tok, tok_len, 1, b));
        return V7_OK;
      case OP_PLUS_ASSIGN: TRY(arith(v7, a, b, a, '+')); break;
      case OP_MINUS_ASSIGN: TRY(arith(v7, a, b, a, '-')); break;
      case OP_MUL_ASSIGN: TRY(arith(v7, a, b, a, '*')); break;
      case OP_DIV_ASSIGN: TRY(arith(v7, a, b, a, '/')); break;
      case OP_REM_ASSIGN: TRY(arith(v7, a, b, a, '%')); break;
      case OP_XOR_ASSIGN: TRY(arith(v7, a, b, a, '^')); break;
      default: return V7_NOT_IMPLEMENTED;
    }
  }

  return V7_OK;
}

static enum v7_err parse_add_sub(struct v7 *v7) {
  TRY(parse_term(v7));
  while ((*v7->pstate.pc == '-' || *v7->pstate.pc == '+') && v7->pstate.pc[1] != '=') {
    int sp1 = v7->sp, ch = *v7->pstate.pc;
    TRY(match(v7, ch));
    TRY(parse_term(v7));
    if (EXECUTING(v7->flags)) {
      TRY(do_arithmetic_op(v7, ch, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err parse_relational(struct v7 *v7) {
  int op;
  TRY(parse_add_sub(v7));
  while ((op = is_relational_op(v7->pstate.pc)) > OP_INVALID) {
    int sp1 = v7->sp;
    v7->pstate.pc += s_op_lengths[op];
    skip_whitespaces_and_comments(v7);
    TRY(parse_add_sub(v7));
    if (EXECUTING(v7->flags)) {
      TRY(do_logical_op(v7, op, sp1, v7->sp));
    }
  }
  if (lookahead(v7, "instanceof", 10)) {
    TRY(parse_identifier(v7));
    if (EXECUTING(v7->flags)) {
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
  if ((op = is_equality_op(v7->pstate.pc)) > OP_INVALID) {
    int sp1 = v7->sp;
    v7->pstate.pc += s_op_lengths[op];
    skip_whitespaces_and_comments(v7);
    TRY(parse_relational(v7));
    if (EXECUTING(v7->flags)) {
      TRY(do_logical_op(v7, op, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_and(struct v7 *v7) {
  TRY(parse_equality(v7));
  if (*v7->pstate.pc == '&' && v7->pstate.pc[1] != '&' && v7->pstate.pc[1] != '=') {
    int sp1 = v7->sp;
    TRY(match(v7, '&'));
    TRY(parse_equality(v7));
    if (EXECUTING(v7->flags)) {
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
  if (*v7->pstate.pc == '^' && v7->pstate.pc[1] != '=') {
    int sp1 = v7->sp;
    TRY(match(v7, '^'));
    TRY(parse_bitwise_and(v7));
    if (EXECUTING(v7->flags)) {
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
  if (*v7->pstate.pc == '|' && v7->pstate.pc[1] != '=' && v7->pstate.pc[1] != '|') {
    int sp1 = v7->sp;
    TRY(match(v7, '|'));
    TRY(parse_bitwise_xor(v7));
    if (EXECUTING(v7->flags)) {
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
  while (*v7->pstate.pc == '&' && v7->pstate.pc[1] == '&') {
    int sp1 = v7->sp;
    match(v7, '&');
    match(v7, '&');
    TRY(parse_bitwise_or(v7));
    if (EXECUTING(v7->flags)) {
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
  if (*v7->pstate.pc == '|' && v7->pstate.pc[1] == '|') {
    int sp1 = v7->sp;
    match(v7, '|');
    match(v7, '|');
    TRY(parse_logical_and(v7));
    if (EXECUTING(v7->flags)) {
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

V7_PRIVATE enum v7_err parse_expression(struct v7 *v7) {
#ifdef V7_DEBUG
  const char *stmt_str = v7->pstate.pc;
#endif
  int op, old_sp = v7->sp;

  v7->cur_obj = v7->ctx;
  TRY(parse_logical_or(v7));

  // Parse assignment
  if ((op = is_assign_op(v7->pstate.pc))) {
    TRY(parse_assign(v7, v7->cur_obj, op));
  }

  // Parse ternary operator
  if (*v7->pstate.pc == '?') {
    int old_flags = v7->flags;
    int condition_true = 1;

    if (EXECUTING(v7->flags)) {
      CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
      condition_true = v7_is_true(v7_top(v7)[-1]);
      TRY(inc_stack(v7, -1));   // Remove condition result
    }

    TRY(match(v7, '?'));
    if (!condition_true || !EXECUTING(old_flags)) v7->flags |= V7_NO_EXEC;
    TRY(parse_expression(v7));
    TRY(match(v7, ':'));
    v7->flags = old_flags;
    if (condition_true || !EXECUTING(old_flags)) v7->flags |= V7_NO_EXEC;
    TRY(parse_expression(v7));
    v7->flags = old_flags;
  }

  // Collapse stack, leave only one value on top
  if (EXECUTING(v7->flags)) {
    struct v7_val *result = v7_top(v7)[-1];
    inc_ref_count(result);
    TRY(inc_stack(v7, old_sp - v7->sp));
    TRY(v7_push(v7, result));
    assert(result->ref_count > 1);
    v7_freeval(v7, result);
  } else {
    TRY(inc_stack(v7, old_sp - v7->sp));
  }

  return V7_OK;
}

static enum v7_err parse_declaration(struct v7 *v7) { // <#parse_decl#>
  int old_sp = v7_sp(v7);

  do {
    inc_stack(v7, old_sp - v7_sp(v7));  // Clean up the stack after prev decl
    TRY(parse_identifier(v7));
    if (*v7->pstate.pc == '=') {
      TRY(parse_assign(v7, v7->ctx, OP_ASSIGN));
    }
  } while (test_and_skip_char(v7, ','));

  return V7_OK;
}

static enum v7_err parse_if_statement(struct v7 *v7, int *has_return) {
  int old_flags = v7->flags, condition_true;

  TRY(match(v7, '('));
  TRY(parse_expression(v7));      // Evaluate condition, pushed on stack
  TRY(match(v7, ')'));
  if (EXECUTING(old_flags)) {
    // If condition is false, do not execute "if" body
    CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
    condition_true = v7_is_true(v7_top_val(v7));
    if (!condition_true) v7->flags |= V7_NO_EXEC;
    TRY(inc_stack(v7, -1));   // Cleanup condition result from the stack
  }
  TRY(parse_compound_statement(v7, has_return));

  if (lookahead(v7, "else", 4)) {
    v7->flags = old_flags;
    if (!EXECUTING(old_flags) || condition_true) v7->flags |= V7_NO_EXEC;
    TRY(parse_compound_statement(v7, has_return));
  }

  v7->flags = old_flags;  // Restore old execution flags
  return V7_OK;
}

static enum v7_err parse_for_in_statement(struct v7 *v7, int has_var,
                                          int *has_return) {
  const char *tok = v7->tok;
  int tok_len = v7->tok_len;
  struct v7_pstate s_block;

  TRY(parse_expression(v7));
  TRY(match(v7, ')'));
  s_block = v7->pstate;

  // Execute loop body
  if (!EXECUTING(v7->flags)) {
    TRY(parse_compound_statement(v7, has_return));
  } else {
    int old_sp = v7->sp;
    struct v7_val *obj = v7_top(v7)[-1];
    struct v7_val *scope = has_var ? v7->ctx : &v7->root_scope;
    struct v7_prop *prop;

    CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_ERROR);
    for (prop = obj->props; prop != NULL; prop = prop->next) {
      TRY(v7_setv(v7, scope, V7_TYPE_STR, V7_TYPE_OBJ,
                  tok, tok_len, 1, prop->key));
      v7->pstate = s_block;
      TRY(parse_compound_statement(v7, has_return));  // Loop body
      TRY(inc_stack(v7, old_sp - v7->sp));  // Clean up stack
    }
  }

  return V7_OK;
}

static enum v7_err parse_for_statement(struct v7 *v7, int *has_return) {
  int is_true, old_flags = v7->flags, has_var = 0;
  struct v7_pstate s1, s2, s3, s_block, s_end;

  TRY(match(v7, '('));
  s1 = v7->pstate;

  // See if this is an enumeration loop
  if (lookahead(v7, "var", 3)) {
    has_var = 1;
  }
  if (parse_identifier(v7) == V7_OK && lookahead(v7, "in", 2)) {
    return parse_for_in_statement(v7, has_var, has_return);
  } else {
    v7->pstate = s1;
  }

  if (lookahead(v7, "var", 3)) {
    parse_declaration(v7);
  } else {
    TRY(parse_expression(v7));    // expr1
  }
  TRY(match(v7, ';'));

  // Pass through the loop, don't execute it, just remember locations
  v7->flags |= V7_NO_EXEC;
  s2 = v7->pstate;
  TRY(parse_expression(v7));    // expr2 (condition)
  TRY(match(v7, ';'));

  s3 = v7->pstate;
  TRY(parse_expression(v7));    // expr3  (post-iteration)
  TRY(match(v7, ')'));

  s_block = v7->pstate;
  TRY(parse_compound_statement(v7, has_return));
  s_end = v7->pstate;

  v7->flags = old_flags;

  // Execute loop
  if (EXECUTING(v7->flags)) {
    int old_sp = v7->sp;
    for (;;) {
      v7->pstate = s2;
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_expression(v7));    // Evaluate condition
      assert(v7->sp > old_sp);
      is_true = !v7_is_true(v7_top(v7)[-1]);
      if (is_true) break;

      v7->pstate = s_block;
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_compound_statement(v7, has_return));  // Loop body
      assert(!EXECUTING(v7->flags) == 0);

      v7->pstate = s3;
      TRY(parse_expression(v7));    // expr3  (post-iteration)

      TRY(inc_stack(v7, old_sp - v7->sp));  // Clean up stack
    }
  }

  // Jump to the code after the loop
  v7->pstate = s_end;

  return V7_OK;
}

static enum v7_err parse_return_statement(struct v7 *v7, int *has_return) {
  if (EXECUTING(v7->flags)) {
    *has_return = 1;
  }
  if (*v7->pstate.pc != ';' && *v7->pstate.pc != '}') {
    TRY(parse_expression(v7));
  }
  return V7_OK;
}

static enum v7_err parse_try_statement(struct v7 *v7, int *has_return) {
  enum v7_err err_code;
  const char *old_pc = v7->pstate.pc;
  int old_flags = v7->flags, old_line_no = v7->pstate.line_no;

  CHECK(v7->pstate.pc[0] == '{', V7_SYNTAX_ERROR);
  err_code = parse_compound_statement(v7, has_return);

  if (!EXECUTING(old_flags) && err_code != V7_OK) {
    return err_code;
  }

  // If exception has happened, skip the block
  if (err_code != V7_OK) {
    v7->pstate.pc = old_pc;
    v7->pstate.line_no = old_line_no;
    v7->flags |= V7_NO_EXEC;
    TRY(parse_compound_statement(v7, has_return));
  }

  // Process catch/finally blocks
  TRY(parse_identifier(v7));

  if (test_token(v7, "catch", 5)) {
    TRY(match(v7, '('));
    TRY(parse_identifier(v7));
    TRY(match(v7, ')'));

    // Insert error variable into the namespace
    if (err_code != V7_OK) {
      TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
      v7_set_class(v7_top_val(v7), V7_CLASS_ERROR);
      v7_setv(v7, v7->ctx, V7_TYPE_STR, V7_TYPE_OBJ,
              v7->tok, v7->tok_len, 1, v7_top_val(v7));
    }

    // If there was no exception, do not execute catch block
    if (!EXECUTING(old_flags) || err_code == V7_OK) v7->flags |= V7_NO_EXEC;
    TRY(parse_compound_statement(v7, has_return));
    v7->flags = old_flags;

    if (lookahead(v7, "finally", 7)) {
      TRY(parse_compound_statement(v7, has_return));
    }
  } else if (test_token(v7, "finally", 7)) {
    v7->flags = old_flags;
    TRY(parse_compound_statement(v7, has_return));
  } else {
    v7->flags = old_flags;
    return V7_SYNTAX_ERROR;
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err parse_statement(struct v7 *v7, int *has_return) {
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
  while (*v7->pstate.pc == ',') match(v7, *v7->pstate.pc);
  while (*v7->pstate.pc == ';') match(v7, *v7->pstate.pc);
  return V7_OK;
}
