/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#define EXPECT(v7, t)                                 \
  do {                                                \
    if ((v7)->cur_tok != (t)) return V7_SYNTAX_ERROR; \
    next_tok(v7);                                     \
  } while (0)

static enum v7_err arith(struct v7 *v7, struct v7_val *a, struct v7_val *b,
                         struct v7_val *res, enum v7_tok op) {
  char *str;

  _prop_func_2_value(v7, &a);
  _prop_func_2_value(v7, &b);
  if (op == TOK_PLUS && (is_string(a) || is_string(b))) {
    /* Do type conversion, result pushed on stack */
    TRY(check_str_re_conv(v7, &a, 0));
    TRY(check_str_re_conv(v7, &b, 0));

    str = (char *)malloc(a->v.str.len + b->v.str.len + 1);
    CHECK(str != NULL, V7_OUT_OF_MEMORY);
    v7_init_str(res, str, a->v.str.len + b->v.str.len, 0);
    memcpy(str, a->v.str.buf, a->v.str.len);
    memcpy(str + a->v.str.len, b->v.str.buf, b->v.str.len);
    str[res->v.str.len] = '\0';
    return V7_OK;
  } else {
    struct v7_val *v = res;
    double an = _conv_to_num(v7, a), bn = _conv_to_num(v7, b);
    if (res->fl.fl.prop_func) v = v7_push_new_object(v7);
    v7_init_num(v, res->v.num);
    switch (op) {
      case TOK_PLUS:
        v->v.num = an + bn;
        break;
      case TOK_MINUS:
        v->v.num = an - bn;
        break;
      case TOK_MUL:
        v->v.num = an * bn;
        break;
      case TOK_DIV:
        v->v.num = an / bn;
        break;
      case TOK_REM:
        v->v.num = (unsigned long)an % (unsigned long)bn;
        break;
      case TOK_XOR:
        v->v.num = (unsigned long)an ^ (unsigned long)bn;
        break;
      default:
        return V7_INTERNAL_ERROR;
    }
    if (res->fl.fl.prop_func) {
      res->v.prop_func.f(res->v.prop_func.o, v, NULL);
      INC_REF_COUNT(v);
      TRY(inc_stack(v7, -2));
      v7_push(v7, v);
      DEC_REF_COUNT(v);
    }
    return V7_OK;
  }
}

static enum v7_err arith_op(struct v7 *v7, enum v7_tok tok, int sp1, int sp2) {
  struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7->stack[sp2 - 1];
  int sp;

  assert(EXECUTING(v7->flags));
  CHECK(v7->sp >= 2, V7_STACK_UNDERFLOW);
  TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
  sp = v7->sp;
  TRY(arith(v7, v1, v2, v7_top_val(v7), tok));

  /* arith() might push another value on stack if type conversion was made. */
  /* if that happens, re-push the result again */
  if (v7->sp > sp) {
    TRY(v7_push(v7, v7->stack[sp - 1]));
  }

  return V7_OK;
}

static enum v7_err parse_compound_statement(struct v7 *v7, int *has_return) {
  if (v7->cur_tok == TOK_OPEN_CURLY) {
    int old_sp = v7->sp;
    next_tok(v7);
    while (v7->cur_tok != TOK_CLOSE_CURLY) {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(parse_statement(v7, has_return));
      if (*has_return && EXECUTING(v7->flags)) return V7_OK;
    }
    EXPECT(v7, TOK_CLOSE_CURLY);
  } else {
    TRY(parse_statement(v7, has_return));
  }
  return V7_OK;
}

static enum v7_err parse_function_definition(struct v7 *v7, struct v7_val **v,
                                             int num_params) { /* <#fdef#> */
  int i = 0, old_flags = v7->flags, old_sp = v7->sp, has_ret = 0,
      line_no = v7->pstate.line_no;
  unsigned long func_name_len = 0;
  const char *src = v7->pstate.pc, *func_name = NULL;
  struct v7_val *ctx = NULL, *f = NULL;

  EXPECT(v7, TOK_FUNCTION);
  if (v7->cur_tok == TOK_IDENTIFIER) {
    /* function name is given, e.g. function foo() {} */
    CHECK(v == NULL, V7_SYNTAX_ERROR);
    func_name = v7->tok;
    func_name_len = v7->tok_len;
    src = v7->pstate.pc;
    line_no = v7->pstate.line_no;
    next_tok(v7);
  }

  /* 1. SCANNING: do nothing, just pass through the function code */
  /* 2. EXECUTING && v == 0: don't execute but create a closure */
  /* 3. EXECUTING && v != 0: execute the closure */

  if (EXECUTING(v7->flags) && v == NULL) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    f = v7_top_val(v7);
    v7_set_class(f, V7_CLASS_FUNCTION);
    f->fl.fl.js_func = 1;

    f->v.func.source_code = (char *)src;
    f->v.func.line_no = line_no;
    /* printf("PFD [%.*s]\n", 45, f->v.func.source_code); */

    f->v.func.var_obj = v7->ctx;
    INC_REF_COUNT(v7->ctx);

    v7->flags |= V7_NO_EXEC;
  } else if (EXECUTING(v7->flags) && v != NULL) {
    f = v[0];
    assert(v7_is_class(f, V7_CLASS_FUNCTION));

    f->next = v7->cf;
    v7->cf = f;

    ctx = make_value(v7, V7_TYPE_OBJ);
    v7_set_class(ctx, V7_CLASS_OBJECT);
    INC_REF_COUNT(ctx);

    ctx->next = v7->ctx;
    v7->ctx = ctx;
  }

  /* Add function arguments to the variable object */
  EXPECT(v7, TOK_OPEN_PAREN);
  while (v7->cur_tok != TOK_CLOSE_PAREN) {
    const char *key = v7->tok;
    unsigned long key_len = v7->tok_len;
    EXPECT(v7, TOK_IDENTIFIER);
    if (EXECUTING(v7->flags)) {
      struct v7_val *val =
          i < num_params ? v[i + 1] : make_value(v7, V7_TYPE_UNDEF);
      TRY(v7_setv(v7, ctx, V7_TYPE_STR, V7_TYPE_OBJ, key, key_len, 1, val));
    }
    i++;
    if (v7->cur_tok == TOK_COMMA) {
      next_tok(v7);
    }
  }
  EXPECT(v7, TOK_CLOSE_PAREN);

  /* Execute (or pass) function body */
  TRY(parse_compound_statement(v7, &has_ret));

  /* Add function to the namespace for notation "function x(y,z) { ... } " */
  if (EXECUTING(old_flags) && v == NULL && func_name != NULL) {
    TRY(v7_setv(v7, v7->ctx, V7_TYPE_STR, V7_TYPE_OBJ, func_name, func_name_len,
                1, f));
  }

  if (EXECUTING(v7->flags)) {
    /* Cleanup execution context */
    v7->ctx = ctx->next;
    ctx->next = NULL;
    /* assert(f->v.func.var_obj == NULL); */
    /* f->v.func.var_obj = ctx; */
    v7_freeval(v7, ctx);

    v7->cf = f->next;
    f->next = NULL;

    /* If function didn't have return statement, return UNDEF */
    if (!has_ret) {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
    }
  }

  v7->flags = old_flags;

  return V7_OK;
}

V7_PRIVATE enum v7_err v7_call2(struct v7 *v7, struct v7_val *this_obj,
                                int num_args, int called_as_ctor) {
  struct v7_val **top = v7_top(v7), **v = top - (num_args + 1), *f;

  if (!EXECUTING(v7->flags)) return V7_OK;
  f = v[0];
  CHECK(v7->sp > num_args, V7_INTERNAL_ERROR);
  CHECK(f != NULL, V7_TYPE_ERROR);
  CHECK(v7_is_class(f, V7_CLASS_FUNCTION), V7_CALLED_NON_FUNCTION);

  /* Stack looks as follows: */
  /*  v   --->  <called_function>     v[0] */
  /*            <argument_0>        ---+ */
  /*            <argument_1>           | */
  /*            <argument_2>           |  <-- num_args */
  /*            ...                    | */
  /*            <argument_N>        ---+ */
  /* top  --->  <return_value> */
  if (f->fl.fl.js_func) {
    struct v7_pstate old_pstate = v7->pstate;
    enum v7_tok tok = v7->cur_tok;

    /* Move control flow to the function body */
    v7->pstate.pc = f->v.func.source_code;
    v7->pstate.line_no = f->v.func.line_no;
    v7->cur_tok = TOK_FUNCTION;
    TRY(parse_function_definition(v7, v, num_args));

    /* Return control flow back */
    v7->pstate = old_pstate;
    v7->cur_tok = tok;
    CHECK(v7_top(v7) >= top, V7_INTERNAL_ERROR);
  } else {
    int old_sp = v7->sp;
    struct v7_c_func_arg arg;
    arg.v7 = v7;
    arg.this_obj = this_obj;
    arg.args = v + 1;
    arg.num_args = num_args;
    arg.called_as_constructor = called_as_ctor;

    TRY(f->v.c_func(&arg));
    if (old_sp == v7->sp) {
      v7_make_and_push(v7, V7_TYPE_UNDEF);
    }
  }
  return V7_OK;
}

static enum v7_err parse_function_call(struct v7 *v7, struct v7_val *this_obj,
                                       int called_as_ctor) {
  struct v7_val **v = v7_top(v7) - 1;
  int num_args = 0;

  /* TRACE_OBJ(v[0]); */
  CHECK(!EXECUTING(v7->flags) || v7_is_class(v[0], V7_CLASS_FUNCTION),
        V7_CALLED_NON_FUNCTION);

  /* Push arguments on stack */
  EXPECT(v7, TOK_OPEN_PAREN);
  while (v7->cur_tok != TOK_CLOSE_PAREN) {
    TRY(parse_expression(v7));
    if (EXECUTING(v7->flags)) {
      struct v7_val *v = v7_top_val(v7);
      INC_REF_COUNT(v);
      TRY(inc_stack(v7, -1));
      DEC_REF_COUNT(v);
      _prop_func_2_value(v7, &v);
      v7_push(v7, v);
    }
    if (v7->cur_tok == TOK_COMMA) {
      next_tok(v7);
    }
    num_args++;
  }
  EXPECT(v7, TOK_CLOSE_PAREN);

  TRY(v7_call2(v7, this_obj, num_args, called_as_ctor));

  return V7_OK;
}

static enum v7_err push_string_literal(struct v7 *v7) {
  struct v7_val *v;
  char *p;
  const char *ps = &v7->tok[1], *end = ps + v7->tok_len - 2;
  Rune rune;

  if (!EXECUTING(v7->flags)) return V7_OK;
  TRY(push_string(v7, &v7->tok[1], v7->tok_len - 2, 1));
  v = v7_top_val(v7);
  CHECK(v->v.str.buf != NULL, V7_OUT_OF_MEMORY);
  p = v->v.str.buf;

  /* Scan string literal into the buffer, handle escape sequences */
  while (ps < end) {
    ps += chartorune(&rune, ps);
    if (rune == '\\') switch (nextesc(&rune, &ps)) {
        case 0:
        case 1:
          break;
        case 2:
          switch (rune) {
            case '\\':
            case '\'':
            case '"':
              break;
            default:
              *p++ = '\\';
          }
          break;
        default:
          return V7_SYNTAX_ERROR;
      }
    p += runetochar(p, &rune);
  }
  v->v.str.len = p - v->v.str.buf;
  assert(v->v.str.len < v7->tok_len - 1);
  *p = '\0';

  return V7_OK;
}

static enum v7_err parse_array_literal(struct v7 *v7) {
  /* Push empty array on stack */
  if (EXECUTING(v7->flags)) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    v7_set_class(v7_top(v7)[-1], V7_CLASS_ARRAY);
  }
  CHECK(v7->cur_tok == TOK_OPEN_BRACKET, V7_SYNTAX_ERROR);
  next_tok(v7);

  /* Scan array literal, append elements one by one */
  while (v7->cur_tok != TOK_CLOSE_BRACKET) {
    /* Push new element on stack */
    TRY(parse_expression(v7));
    if (EXECUTING(v7->flags)) {
      TRY(v7_append(v7, v7_top(v7)[-2], v7_top(v7)[-1]));
      TRY(inc_stack(v7, -1));
    }
    if (v7->cur_tok == TOK_COMMA) {
      next_tok(v7);
    }
  }
  CHECK(v7->cur_tok == TOK_CLOSE_BRACKET, V7_SYNTAX_ERROR);
  return V7_OK;
}

static enum v7_err parse_object_literal(struct v7 *v7) {
  /* Push empty object on stack */
  TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
  EXPECT(v7, TOK_OPEN_CURLY);

  /* Assign key/values to the object, until closing "}" is found */
  while (v7->cur_tok != TOK_CLOSE_CURLY) {
    /* Push key on stack */
    if (v7->cur_tok == TOK_STRING_LITERAL) {
      TRY(push_string_literal(v7));
    } else {
      struct v7_val *v;
      CHECK(v7->cur_tok == TOK_IDENTIFIER, V7_SYNTAX_ERROR);
      v = v7_mkv(v7, V7_TYPE_STR, v7->tok, v7->tok_len, 1);
      CHECK(v != NULL, V7_OUT_OF_MEMORY);
      TRY(v7_push(v7, v));
    }

    /* Push value on stack */
    next_tok(v7);
    EXPECT(v7, TOK_COLON);
    TRY(parse_expression(v7));

    /* Stack should now have object, key, value. Assign, and remove key/value */
    if (EXECUTING(v7->flags)) {
      struct v7_val **v = v7_top(v7) - 3;
      CHECK(v[0]->type == V7_TYPE_OBJ, V7_INTERNAL_ERROR);
      TRY(v7_set2(v7, v[0], v[1], v[2]));
      TRY(inc_stack(v7, -2));
    }
    if (v7->cur_tok == TOK_COMMA) {
      next_tok(v7);
    }
  }
  CHECK(v7->cur_tok == TOK_CLOSE_CURLY, V7_SYNTAX_ERROR);
  return V7_OK;
}

static enum v7_err parse_delete_statement(struct v7 *v7) {
  EXPECT(v7, TOK_DELETE);
  TRY(parse_expression(v7));
  TRY(v7_del2(v7, v7->cur_obj, v7->key, v7->key_len));
  return V7_OK;
}

static enum v7_err parse_regex(struct v7 *v7) {
  size_t i, j;
  uint8_t done = 0;

  if (!EXECUTING(v7->flags)) return V7_OK;
  /* CHECK(*v7->tok == '/', V7_SYNTAX_ERROR); */

  for (i = 1; !done; i++) {
    switch (v7->tok[i]) {
      case '\0':
      case '\r':
      case '\n':
        THROW(V7_SYNTAX_ERROR);
      case '/':
        if ('\\' == v7->tok[i - 1]) continue;
        done = 1;
        break;
    }
  }
  done = 0;
  for (j = i; !done; j++) {
    switch (v7->tok[j]) {
      default:
        done = 1;
        j--;
      case 'g':
      case 'i':
      case 'm':
        break;
    }
  }

  TRY(regex_xctor(v7, NULL, &v7->tok[1], i - 2, &v7->tok[i], j - i));
  v7->pstate.pc = &v7->tok[j];
  return V7_OK;
}

static enum v7_err parse_variable(struct v7 *v7) {
  struct v7_val key = str_to_val(v7->tok, v7->tok_len), *v = NULL;
  v7->key = v7->tok;
  v7->key_len = v7->tok_len;
  v = find(v7, &key);
  if (v == NULL) {
    TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
  } else {
    TRY(v7_push(v7, v));
  }
  return V7_OK;
}

static enum v7_err parse_precedence_0(struct v7 *v7) {
  enum v7_tok tok = v7->cur_tok;
  int ex = EXECUTING(v7->flags);

  switch (tok) {
    case TOK_OPEN_PAREN:
      next_tok(v7);
      TRY(parse_expression(v7));
      CHECK(v7->cur_tok == TOK_CLOSE_PAREN, V7_SYNTAX_ERROR);
      break;
    case TOK_OPEN_BRACKET:
      TRY(parse_array_literal(v7));
      break;
    case TOK_OPEN_CURLY:
      TRY(parse_object_literal(v7));
      break;
    case TOK_DIV:
      TRY(parse_regex(v7));
      break;
    case TOK_STRING_LITERAL:
      TRY(push_string_literal(v7));
      break;
    case TOK_THIS:
      if (ex) TRY(v7_push(v7, v7->this_obj));
      break;
    case TOK_NULL:
      if (ex) TRY(v7_make_and_push(v7, V7_TYPE_NULL));
      break;
    case TOK_UNDEFINED:
      if (ex) TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
      break;
    case TOK_TRUE:
      if (ex) TRY(push_bool(v7, 1));
      break;
    case TOK_FALSE:
      if (ex) TRY(push_bool(v7, 0));
      break;
#if 0
    case TOK_NAN: if (ex) TRY(push_number(v7, NAN)); break;
    case TOK_INFINITY: if (ex) TRY(push_number(v7, INFINITY)); break;
#endif
    case TOK_NUMBER:
      if (ex) TRY(push_number(v7, v7->cur_tok_dbl));
      break;
    case TOK_IDENTIFIER:
      if (ex) TRY(parse_variable(v7));
      break;
    case TOK_FUNCTION:
      TRY(parse_function_definition(v7, NULL, 0));
      break;
    default:
      return V7_SYNTAX_ERROR;
  }

  if (tok != TOK_FUNCTION) {
    next_tok(v7);
  }

  return V7_OK;
}

static enum v7_err parse_prop_accessor(struct v7 *v7, enum v7_tok op) {
  struct v7_val *ns = NULL, *cur_obj = NULL;

  if (EXECUTING(v7->flags)) {
    ns = v7_top(v7)[-1];
    v7->cur_obj = v7->this_obj = cur_obj = ns;
  }
  CHECK(!EXECUTING(v7->flags) || ns != NULL, V7_SYNTAX_ERROR);

  if (op == TOK_DOT) {
    CHECK(v7->cur_tok == TOK_IDENTIFIER, V7_SYNTAX_ERROR);
    v7->key = v7->tok;
    v7->key_len = v7->tok_len;
    if (EXECUTING(v7->flags)) {
      struct v7_val key = str_to_val(v7->tok, v7->tok_len);
      ns = get2(ns, &key);
    }
    next_tok(v7);
  } else {
    TRY(parse_expression(v7));
    EXPECT(v7, TOK_CLOSE_BRACKET);
    if (EXECUTING(v7->flags)) {
      struct v7_val *expr_val = v7_top_val(v7);

      ns = get2(ns, expr_val);

      /* If we're doing an assignment, */
      /* then parse_assign() looks at v7->key, v7->key_len for the key. */
      /* Initialize key properly for cases like "a.b['c'] = d;" */
      TRY(check_str_re_conv(v7, &expr_val, 0));
      v7->key = expr_val->v.str.buf;
      v7->key_len = expr_val->v.str.len;
    }
  }

  /* Set those again cause parse_expression() above could have changed it */
  v7->cur_obj = v7->this_obj = cur_obj;

  if (EXECUTING(v7->flags)) {
    if (NULL == ns) {
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
    } else {
      TRY(v7_push(v7, ns));
    }
  }

  return V7_OK;
}

/* Member Access            left-to-right    x . x */
/* Computed Member Access   left-to-right    x [ x ] */
/* new (with argument list) n/a              new x ( x ) */
static enum v7_err parse_precedence_1(struct v7 *v7, int has_new) {
  struct v7_val *old_this = v7->this_obj;

  TRY(parse_precedence_0(v7));

  while (v7->cur_tok == TOK_DOT || v7->cur_tok == TOK_OPEN_BRACKET ||
         v7->cur_tok == TOK_OPEN_PAREN) {
    enum v7_tok tok = v7->cur_tok;
    if (tok == TOK_OPEN_BRACKET || tok == TOK_DOT) {
      next_tok(v7);
      TRY(parse_prop_accessor(v7, tok));
    } else {
      TRY(parse_function_call(v7, v7->cur_obj, has_new));
    }
  }
  v7->this_obj = old_this;

  return V7_OK;
}

/* x . y () . z () () */

/* Function Call                 left-to-right     x ( x ) */
/* new (without argument list)   right-to-left     new x */
static enum v7_err parse_precedence_2(struct v7 *v7) {
  int has_new = 0;
  struct v7_val *old_this_obj = v7->this_obj, *cur_this = v7->this_obj;

  if (v7->cur_tok == TOK_NEW) {
    has_new++;
    next_tok(v7);
    if (EXECUTING(v7->flags)) {
      v7_make_and_push(v7, V7_TYPE_OBJ);
      cur_this = v7->this_obj = v7_top(v7)[-1];
      v7_set_class(cur_this, V7_CLASS_OBJECT);
    }
  }
  TRY(parse_precedence_1(v7, has_new));
#if 0
  while (*v7->pstate.pc == '(') {
    /* Use cur_this, not v7->this_obj: v7->this_obj could have been changed */
    TRY(parse_function_call(v7, cur_this, has_new));
  }
#endif
  if (has_new && EXECUTING(v7->flags)) {
    TRY(v7_push(v7, cur_this));
  }

  v7->this_obj = old_this_obj;

  return V7_OK;
}

/* Postfix Increment    n/a      x ++ */
/* Postfix Decrement    n/a      x -- */
static enum v7_err parse_postfix_inc_dec(struct v7 *v7) {
  TRY(parse_precedence_2(v7));
  if (v7->cur_tok == TOK_PLUS_PLUS || v7->cur_tok == TOK_MINUS_MINUS) {
    int increment = (v7->cur_tok == TOK_PLUS_PLUS) ? 1 : -1;
    next_tok(v7);
    if (EXECUTING(v7->flags)) {
      struct v7_val *v, *v1;
      v = v7_top(v7)[-1];
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
      v1 = v7_top(v7)[-1];
      if (v->fl.fl.prop_func) {
        v->v.prop_func.f(v->v.prop_func.o, NULL, v1);
        CHECK(v1->type == V7_TYPE_NUM, V7_TYPE_ERROR);
        v1->v.num += increment;
        v->v.prop_func.f(v->v.prop_func.o, v1, NULL);
        v1->v.num -= increment;
      } else {
        CHECK(v->type == V7_TYPE_NUM, V7_TYPE_ERROR);
        v7_init_num(v1, v->v.num);
        v->v.num += increment;
      }
      INC_REF_COUNT(v1);
      TRY(inc_stack(v7, -2));
      v7_push(v7, v1);
      DEC_REF_COUNT(v1);
    }
  }
  return V7_OK;
}

/* Logical NOT        right-to-left    ! x */
/* Bitwise NOT        right-to-left    ~ x */
/* Unary Plus         right-to-left    + x */
/* Unary Negation     right-to-left    - x */
/* Prefix Increment   right-to-left    ++ x */
/* Prefix Decrement   right-to-left    -- x */
/* typeof             right-to-left    typeof x */
/* void               right-to-left    void x */
/* delete             right-to-left    delete x */
static enum v7_err parse_unary(struct v7 *v7) {
  static const char *type_names[] = {"undefined", "object", "boolean",
                                     "string",    "number", "object"};
  const char *str;
  int unary = TOK_END_OF_INPUT;

  switch (v7->cur_tok) {
    case TOK_NOT:
    case TOK_TILDA:
    case TOK_PLUS:
    case TOK_MINUS:
    case TOK_PLUS_PLUS:
    case TOK_MINUS_MINUS:
    case TOK_TYPEOF:
    case TOK_VOID:
    case TOK_DELETE:
      unary = v7->cur_tok;
      next_tok(v7);
      break;
    default:
      unary = TOK_END_OF_INPUT;
      break;
  }

  TRY(parse_postfix_inc_dec(v7));

  if (EXECUTING(v7->flags) && unary != TOK_END_OF_INPUT) {
    struct v7_val *result = v7_top_val(v7);
    if (result->fl.fl.prop_func) {
      switch (unary) {
        case TOK_TILDA:
        case TOK_PLUS:
        case TOK_MINUS:
        case TOK_NOT:
        case TOK_TYPEOF:
          _prop_func_2_value(v7, &result);
          v7_push(v7, result);
      }
    }
    switch (unary) {
      case TOK_TILDA:
        TRY(push_number(v7, ~(long)_conv_to_num(v7, result)));
        break;
      case TOK_PLUS:
        TRY(push_number(v7, _conv_to_num(v7, result)));
        break;
      case TOK_MINUS:
        TRY(push_number(v7, -_conv_to_num(v7, result)));
        break;
      case TOK_NOT:
        TRY(push_bool(v7, !v7_is_true(result)));
        break;
      case TOK_TYPEOF:
        str = type_names[result->type];
        if (v7_is_class(result, V7_CLASS_FUNCTION)) str = "function";
        TRY(push_string(v7, str, strlen(str), 0));
        break;
      case TOK_VOID:
        TRY(inc_stack(v7, -1));
        TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
        break;
      default:
        break;
    }
  }

  return V7_OK;
}

static enum v7_err parse_mul_div_rem(struct v7 *v7) {
  TRY(parse_unary(v7));
  while (v7->cur_tok == TOK_MUL || v7->cur_tok == TOK_DIV ||
         v7->cur_tok == TOK_REM) {
    int sp1 = v7->sp;
    enum v7_tok tok = v7->cur_tok;
    next_tok(v7);
    TRY(parse_unary(v7));
    if (EXECUTING(v7->flags)) {
      TRY(arith_op(v7, tok, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err logical_op(struct v7 *v7, enum v7_tok op, int sp1, int sp2) {
  struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7->stack[sp2 - 1];
  int res = 0;
  double n1 = _conv_to_num(v7, v1), n2 = _conv_to_num(v7, v2);

  if (v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM) {
    switch (op) {
      case TOK_GT:
        res = n1 > n2;
        break;
      case TOK_GE:
        res = n1 >= n2;
        break;
      case TOK_LT:
        res = n1 < n2;
        break;
      case TOK_LE:
        res = n1 <= n2;
        break;
      case TOK_EQ: /* FALLTHROUGH */
      case TOK_EQ_EQ:
        res = cmp(v1, v2) == 0;
        break;
      case TOK_NE: /* FALLTHROUGH */
      case TOK_NE_NE:
        res = cmp(v1, v2) != 0;
        break;
      default:
        return V7_INTERNAL_ERROR;
    }
  } else if (op == TOK_EQ || op == TOK_EQ_EQ) {
    res = cmp(v1, v2) == 0;
  } else if (op == TOK_NE || op == TOK_NE_NE) {
    res = cmp(v1, v2) != 0;
  }
  TRY(push_bool(v7, res));
  return V7_OK;
}

static enum v7_err parse_add_sub(struct v7 *v7) {
  TRY(parse_mul_div_rem(v7));
  while (v7->cur_tok == TOK_PLUS || v7->cur_tok == TOK_MINUS) {
    int sp1 = v7->sp;
    enum v7_tok op = v7->cur_tok;
    next_tok(v7);
    TRY(parse_mul_div_rem(v7));
    if (EXECUTING(v7->flags)) {
      TRY(arith_op(v7, op, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err parse_relational(struct v7 *v7) {
  TRY(parse_add_sub(v7));
  while (v7->cur_tok >= TOK_LE && v7->cur_tok <= TOK_GT) {
    int sp1 = v7->sp;
    enum v7_tok op = v7->cur_tok;
    next_tok(v7);
    TRY(parse_add_sub(v7));
    if (EXECUTING(v7->flags)) {
      TRY(logical_op(v7, op, sp1, v7->sp));
    }
  }
  if (v7->cur_tok == TOK_INSTANCEOF) {
    CHECK(next_tok(v7) == TOK_IDENTIFIER, V7_SYNTAX_ERROR);
    if (EXECUTING(v7->flags)) {
      struct v7_val key = str_to_val(v7->tok, v7->tok_len);
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = instanceof(v7_top(v7)[-2], find(v7, &key));
    }
  }
  return V7_OK;
}

static enum v7_err parse_equality(struct v7 *v7) {
  TRY(parse_relational(v7));
  if (v7->cur_tok >= TOK_EQ && v7->cur_tok <= TOK_NE_NE) {
    int sp1 = v7->sp;
    enum v7_tok op = v7->cur_tok;
    next_tok(v7);
    TRY(parse_relational(v7));
    if (EXECUTING(v7->flags)) {
      TRY(logical_op(v7, op, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_and(struct v7 *v7) {
  TRY(parse_equality(v7));
  if (v7->cur_tok == TOK_AND) {
    int sp1 = v7->sp;
    next_tok(v7);
    TRY(parse_equality(v7));
    if (EXECUTING(v7->flags)) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = _conv_to_num(v7, v1), b = _conv_to_num(v7, v2);
      CHECK(v1->type == V7_TYPE_NUM && v1->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a & b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_xor(struct v7 *v7) {
  TRY(parse_bitwise_and(v7));
  if (v7->cur_tok == TOK_XOR) {
    int sp1 = v7->sp;
    next_tok(v7);
    TRY(parse_bitwise_and(v7));
    if (EXECUTING(v7->flags)) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = _conv_to_num(v7, v1), b = _conv_to_num(v7, v2);
      CHECK(v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a ^ b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_or(struct v7 *v7) {
  TRY(parse_bitwise_xor(v7));
  if (v7->cur_tok == TOK_OR) {
    int sp1 = v7->sp;
    next_tok(v7);
    TRY(parse_bitwise_xor(v7));
    if (EXECUTING(v7->flags)) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = _conv_to_num(v7, v1), b = _conv_to_num(v7, v2);
      CHECK(v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a | b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_logical_and(struct v7 *v7) {
  TRY(parse_bitwise_or(v7));
  while (v7->cur_tok == TOK_LOGICAL_AND) {
    int sp1 = v7->sp;
    next_tok(v7);
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
  if (v7->cur_tok == TOK_LOGICAL_OR) {
    int sp1 = v7->sp;
    next_tok(v7);
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

V7_PRIVATE enum v7_err parse_ternary(struct v7 *v7) {
  TRY(parse_logical_or(v7));

  if (v7->cur_tok == TOK_QUESTION) {
    int old_flags = v7->flags;
    int condition_true = 1;

    if (EXECUTING(v7->flags)) {
      CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
      condition_true = v7_is_true(v7_top(v7)[-1]);
      TRY(inc_stack(v7, -1)); /* Remove condition result */
    }

    EXPECT(v7, TOK_QUESTION);
    if (!condition_true || !EXECUTING(old_flags)) v7->flags |= V7_NO_EXEC;
    TRY(parse_expression(v7));
    EXPECT(v7, TOK_COLON);
    v7->flags = old_flags;
    if (condition_true || !EXECUTING(old_flags)) v7->flags |= V7_NO_EXEC;
    TRY(parse_expression(v7));
    v7->flags = old_flags;
  }

  return V7_OK;
}

static enum v7_err do_assign(struct v7 *v7, struct v7_val *obj, const char *key,
                             unsigned long key_len, enum v7_tok tok) {
  if (EXECUTING(v7->flags)) {
    struct v7_val **top = v7_top(v7), *a = top[-2], *b = top[-1];

    /* Stack layout at this point (assuming stack grows down): */
    /* */
    /*          | object's value (rvalue)    |    top[-2] */
    /*          +----------------------------+ */
    /*          | expression value (lvalue)  |    top[-1] */
    /*          +----------------------------+ */
    /* top -->  |       nothing yet          | */

    switch (tok) {
      case TOK_ASSIGN:
        CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
        TRY(v7_setv(v7, obj, V7_TYPE_STR, V7_TYPE_OBJ, key, key_len, 1,
                    b));  // TODO(vrz) ERROR
        return V7_OK;
      case TOK_PLUS_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_PLUS));
        break;
      case TOK_MINUS_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_MINUS));
        break;
      case TOK_MUL_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_MUL));
        break;
      case TOK_DIV_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_DIV));
        break;
      case TOK_REM_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_REM));
        break;
      case TOK_XOR_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_XOR));
        break;
      default:
        return V7_NOT_IMPLEMENTED;
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err parse_expression(struct v7 *v7) {
  int old_sp = v7->sp;

  /* Set up the reference to be the current execution context */
  v7->cur_obj = v7->ctx;
  v7->key = NULL;

  /* TODO(lsm): parse_yield() should be here, do we want to implement it? */
  TRY(parse_ternary(v7));

  /* Parse assignment. */
  if (v7->cur_tok >= TOK_ASSIGN && v7->cur_tok <= TOK_LOGICAL_OR_ASSING) {
    /* Remember current reference */
    const char *key = v7->key;
    unsigned long key_len = v7->key_len;
    struct v7_val *cur_obj = v7->cur_obj;
    enum v7_tok op = v7->cur_tok;

    next_tok(v7);

    /* This recursion implements right-to-left association for assignment */
    TRY(parse_expression(v7));
    if (EXECUTING(v7->flags)) {
      TRY(do_assign(v7, cur_obj, key, key_len, op));
    }
  }

  /* Collapse stack, leave only one value on top */
  if (EXECUTING(v7->flags)) {
    struct v7_val *result = v7_top(v7)[-1];
    INC_REF_COUNT(result);
    TRY(inc_stack(v7, old_sp - v7->sp));
    TRY(v7_push(v7, result));
    assert(result->ref_count > 1);
    v7_freeval(v7, result);
  } else {
    TRY(inc_stack(v7, old_sp - v7->sp));
  }

  return V7_OK;
}

static enum v7_err parse_declaration(struct v7 *v7) { /* <#parse_decl#> */
  int old_sp = v7_sp(v7);

  EXPECT(v7, TOK_VAR);
  do {
    const char *key = v7->tok;
    unsigned long key_len = v7->tok_len;

    inc_stack(v7, old_sp - v7_sp(v7));
    EXPECT(v7, TOK_IDENTIFIER);
    if (v7->cur_tok == TOK_ASSIGN) {
      next_tok(v7);
      TRY(parse_expression(v7));
      if (EXECUTING(v7->flags)) {
        TRY(do_assign(v7, v7->ctx, key, key_len, TOK_ASSIGN));
      }
    }
  } while (v7->cur_tok == TOK_IDENTIFIER);

  return V7_OK;
}

static enum v7_err parse_if_statement(struct v7 *v7, int *has_return) {
  int old_flags = v7->flags, condition_true;

  EXPECT(v7, TOK_IF);
  EXPECT(v7, TOK_OPEN_PAREN);
  TRY(parse_expression(v7)); /* Evaluate condition, pushed on stack */
  EXPECT(v7, TOK_CLOSE_PAREN);

  if (EXECUTING(old_flags)) {
    /* If condition is false, do not execute "if" body */
    CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
    condition_true = v7_is_true(v7_top_val(v7));
    if (!condition_true) v7->flags |= V7_NO_EXEC;
    TRY(inc_stack(v7, -1)); /* Cleanup condition result from the stack */
  }
  TRY(parse_compound_statement(v7, has_return));

  if (v7->cur_tok == TOK_ELSE) {
    next_tok(v7);
    v7->flags = old_flags;
    if (!EXECUTING(old_flags) || condition_true) v7->flags |= V7_NO_EXEC;
    TRY(parse_compound_statement(v7, has_return));
  }

  v7->flags = old_flags; /* Restore old execution flags */
  return V7_OK;
}

static enum v7_err parse_for_in_statement(struct v7 *v7, int has_var,
                                          int *has_return) {
  const char *tok = v7->tok;
  unsigned long tok_len = v7->tok_len;
  struct v7_pstate s_block;

  EXPECT(v7, TOK_IDENTIFIER);
  EXPECT(v7, TOK_IN);
  TRY(parse_expression(v7));
  EXPECT(v7, TOK_CLOSE_PAREN);
  s_block = v7->pstate;

  /* Execute loop body */
  if (!EXECUTING(v7->flags)) {
    TRY(parse_compound_statement(v7, has_return));
  } else {
    int old_sp = v7->sp;
    struct v7_val *obj = v7_top(v7)[-1];
    struct v7_val *scope = has_var ? v7->ctx : &v7->root_scope;
    struct v7_prop *prop;

    CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_ERROR);
    for (prop = obj->props; prop != NULL; prop = prop->next) {
      TRY(v7_setv(v7, scope, V7_TYPE_STR, V7_TYPE_OBJ, tok, tok_len, 1,
                  prop->key));
      v7->pstate = s_block;
      TRY(parse_compound_statement(v7, has_return)); /* Loop body */
      TRY(inc_stack(v7, old_sp - v7->sp));           /* Clean up stack */
    }
  }

  return V7_OK;
}

static enum v7_err parse_for_statement(struct v7 *v7, int *has_return) {
  int is_true, old_flags = v7->flags, has_var = 0;
  struct v7_pstate s2, s3, s_block, s_end;

  EXPECT(v7, TOK_FOR);
  EXPECT(v7, TOK_OPEN_PAREN);

  if (v7->cur_tok == TOK_VAR) {
    has_var++;
    next_tok(v7);
  }

  if (v7->cur_tok == TOK_IDENTIFIER && lookahead(v7) == TOK_IN) {
    return parse_for_in_statement(v7, has_var, has_return);
#if 0
  } else if (v7->cur_tok == TOK_IDENTIFIER && has_var) {
    printf("(%s)\n", "rr");
    TRY(parse_declaration(v7));
#endif
  } else {
    TRY(parse_expression(v7));
  }

  EXPECT(v7, TOK_SEMICOLON);

  /* Pass through the loop, don't execute it, just remember locations */
  v7->flags |= V7_NO_EXEC;
  get_v7_state(v7, &s2);
  TRY(parse_expression(v7)); /* expr2 (condition) */
  EXPECT(v7, TOK_SEMICOLON);

  get_v7_state(v7, &s3);
  TRY(parse_expression(v7)); /* expr3  (post-iteration) */
  EXPECT(v7, TOK_CLOSE_PAREN);

  get_v7_state(v7, &s_block);
  TRY(parse_compound_statement(v7, has_return));
  get_v7_state(v7, &s_end);

  v7->flags = old_flags;

  /* Execute loop */
  if (EXECUTING(v7->flags)) {
    int old_sp = v7->sp;
    for (;;) {
      set_v7_state(v7, &s2);
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_expression(v7)); /* Evaluate condition */
      assert(v7->sp > old_sp);
      is_true = !v7_is_true(v7_top(v7)[-1]);
      if (is_true) break;

      set_v7_state(v7, &s_block);
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_compound_statement(v7, has_return)); /* Loop body */
      assert(!EXECUTING(v7->flags) == 0);

      set_v7_state(v7, &s3);
      TRY(parse_expression(v7)); /* expr3  (post-iteration) */

      TRY(inc_stack(v7, old_sp - v7->sp)); /* Clean up stack */
    }
  }

  /* Jump to the code after the loop */
  set_v7_state(v7, &s_end);

  return V7_OK;
}

static enum v7_err parse_while_statement(struct v7 *v7, int *has_return) {
  int is_true, old_flags = v7->flags;
  struct v7_pstate s_cond, s_block, s_end;

  EXPECT(v7, TOK_WHILE);
  EXPECT(v7, TOK_OPEN_PAREN);
  get_v7_state(v7, &s_cond);
  v7->flags |= V7_NO_EXEC;
  TRY(parse_expression(v7));
  EXPECT(v7, TOK_CLOSE_PAREN);

  get_v7_state(v7, &s_block);
  TRY(parse_compound_statement(v7, has_return));
  get_v7_state(v7, &s_end);

  v7->flags = old_flags;

  /* Execute loop */
  if (EXECUTING(v7->flags)) {
    int old_sp = v7->sp;
    for (;;) {
      set_v7_state(v7, &s_cond);
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_expression(v7)); /* Evaluate condition */
      assert(v7->sp > old_sp);
      is_true = !v7_is_true(v7_top_val(v7));
      if (is_true) break;

      set_v7_state(v7, &s_block);
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_compound_statement(v7, has_return)); /* Loop body */
      assert(!EXECUTING(v7->flags) == 0);

      TRY(inc_stack(v7, old_sp - v7->sp)); /* Clean up stack */
    }
  }

  /* Jump to the code after the loop */
  set_v7_state(v7, &s_end);

  return V7_OK;
}

static enum v7_err parse_return_statement(struct v7 *v7, int *has_return) {
  if (EXECUTING(v7->flags)) {
    *has_return = 1;
  }
  EXPECT(v7, TOK_RETURN);
  if (v7->cur_tok != TOK_SEMICOLON && v7->cur_tok != TOK_CLOSE_CURLY) {
    TRY(parse_expression(v7));
  }
  return V7_OK;
}

static enum v7_err parse_try_statement(struct v7 *v7, int *has_return) {
  enum v7_err err_code;
  const char *old_pc = v7->pstate.pc;
  int old_flags = v7->flags, old_line_no = v7->pstate.line_no;

  EXPECT(v7, TOK_TRY);
  CHECK(v7->cur_tok == TOK_OPEN_CURLY, V7_SYNTAX_ERROR);
  err_code = parse_compound_statement(v7, has_return);

  if (!EXECUTING(old_flags) && err_code != V7_OK) {
    return err_code;
  }

  /* If exception has happened, skip the block */
  if (err_code != V7_OK) {
    v7->pstate.pc = old_pc;
    v7->pstate.line_no = old_line_no;
    v7->flags |= V7_NO_EXEC;
    TRY(parse_compound_statement(v7, has_return));
  }

  /* Process catch/finally blocks */
  CHECK(next_tok(v7) == TOK_IDENTIFIER, V7_SYNTAX_ERROR);

  /* if (test_token(v7, "catch", 5)) { */
  if (v7->cur_tok == TOK_CATCH) {
    const char *key;
    unsigned long key_len;

    EXPECT(v7, TOK_CATCH);
    EXPECT(v7, TOK_OPEN_PAREN);
    key = v7->tok;
    key_len = v7->tok_len;
    EXPECT(v7, TOK_IDENTIFIER);
    EXPECT(v7, TOK_CLOSE_PAREN);

    /* Insert error variable into the namespace */
    if (err_code != V7_OK) {
      TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
      v7_set_class(v7_top_val(v7), V7_CLASS_ERROR);
      v7_setv(v7, v7->ctx, V7_TYPE_STR, V7_TYPE_OBJ, key, key_len, 1,
              v7_top_val(v7));
    }

    /* If there was no exception, do not execute catch block */
    if (!EXECUTING(old_flags) || err_code == V7_OK) v7->flags |= V7_NO_EXEC;
    TRY(parse_compound_statement(v7, has_return));
    v7->flags = old_flags;

    if (v7->cur_tok == TOK_FINALLY) {
      TRY(parse_compound_statement(v7, has_return));
    }
  } else if (v7->cur_tok == TOK_FINALLY) {
    v7->flags = old_flags;
    TRY(parse_compound_statement(v7, has_return));
  } else {
    v7->flags = old_flags;
    return V7_SYNTAX_ERROR;
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err parse_statement(struct v7 *v7, int *has_return) {
  switch (v7->cur_tok) {
    case TOK_VAR:
      TRY(parse_declaration(v7));
      break;
    case TOK_RETURN:
      TRY(parse_return_statement(v7, has_return));
      break;
    case TOK_IF:
      TRY(parse_if_statement(v7, has_return));
      break;
    case TOK_FOR:
      TRY(parse_for_statement(v7, has_return));
      break;
    case TOK_WHILE:
      TRY(parse_while_statement(v7, has_return));
      break;
    case TOK_TRY:
      TRY(parse_try_statement(v7, has_return));
      break;
    case TOK_DELETE:
      TRY(parse_delete_statement(v7));
      break;
    default:
      TRY(parse_expression(v7));
      break;
  }

  /* Skip optional colons and semicolons. */
  /* TODO(lsm): follow automatic semicolon insertion rules */
  while (v7->cur_tok == TOK_COMMA || v7->cur_tok == TOK_SEMICOLON) {
    next_tok(v7);
  }

  return V7_OK;
}
