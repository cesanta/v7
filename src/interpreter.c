/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t i_eval_stmts(struct v7 *, struct ast *, ast_off_t *, ast_off_t,
                          val_t, int *);
static val_t i_eval_call(struct v7 *, struct ast *, ast_off_t *, val_t);

static double i_as_num(val_t v) {
  char *tmp;
  double dbl;
  if (!v7_is_double(v) && !v7_is_boolean(v)) {
    if (v7_is_string(v)) {
      struct v7_str *str = val_to_string(v);
      tmp = strndup(str->buf, str->len);
      dbl = strtod(tmp, NULL);
      free(tmp);
      return dbl;
    } else {
      return NAN;
    }
  } else {
    if(v7_is_boolean(v)) {
      return (double) val_to_boolean(v);
    }
    return val_to_double(v);
  }
}

static int i_is_true(val_t v) {
  /* TODO(mkm): real stuff */
  return (v7_is_double(v) && val_to_double(v) > 0.0) ||
      (v7_is_boolean(v) && val_to_boolean(v));
}

static double i_num_unary_op(enum ast_tag tag, double a) {
  switch (tag) {
    case AST_POSITIVE:
      return a;
    case AST_NEGATIVE:
      return -a;
    default:
      printf("NOT IMPLEMENTED");  /* LCOV_EXCL_LINE */
      abort();
  }
}

static double i_num_bin_op(enum ast_tag tag, double a, double b) {
  switch (tag) {
    case AST_ADD:  /* simple fixed width nodes with no payload */
      return a + b;
    case AST_SUB:
      return a - b;
    case AST_REM:
      return (int) a % (int) b;
    case AST_MUL:
      return a * b;
    case AST_DIV:
      return a / b;
    default:
      printf("NOT IMPLEMENTED");  /* LCOV_EXCL_LINE */
      abort();
  }
}

static int i_bool_bin_op(enum ast_tag tag, double a, double b) {
  switch (tag) {
    case AST_EQ:
      return a == b;
    case AST_NE:
      return a != b;
    case AST_LT:
      return a < b;
    case AST_LE:
      return a <= b;
    case AST_GT:
      return a > b;
    case AST_GE:
      return a >= b;
    default:
      printf("NOT IMPLEMENTED");  /* LCOV_EXCL_LINE */
      abort();
  }
}

static val_t i_eval_expr(struct v7 *v7, struct ast *a, ast_off_t *pos,
                         val_t scope) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  const struct ast_node_def *def = &ast_node_defs[tag];
  ast_off_t end;
  val_t res, v1, v2;
  double dv;
  int i;

  /*
   * TODO(mkm): put this temporary somewhere in the evaluation context
   * or use alloca.
   */
  char buf[512];
  char *name;
  v7_strlen_t name_len;

  switch (tag) {
    case AST_NEGATIVE:
    case AST_POSITIVE:
      return v7_create_value(v7, V7_TYPE_NUMBER, i_num_unary_op(
          tag, i_as_num(i_eval_expr(v7, a, pos, scope))));
    case AST_ADD:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      if (!(v7_is_undefined(v1) || v7_is_double(v1) || v7_is_boolean(v1)) ||
          !(v7_is_undefined(v2) || v7_is_double(v2) || v7_is_boolean(v2))) {
        v7_stringify_value(v7, v1, buf, sizeof(buf));
        v7_stringify_value(v7, v2, buf + strlen(buf),
                           sizeof(buf) - strlen(buf));
        return v7_create_value(v7, V7_TYPE_STRING, buf,
                               (v7_strlen_t) strlen(buf));
      }
      return v7_create_value(v7, V7_TYPE_NUMBER, i_num_bin_op(tag, i_as_num(v1),
                                                              i_as_num(v2)));
    case AST_SUB:
    case AST_REM:
    case AST_MUL:
    case AST_DIV:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      return v7_create_value(v7, V7_TYPE_NUMBER, i_num_bin_op(tag, i_as_num(v1),
                                                              i_as_num(v2)));
    case AST_EQ:
    case AST_NE:
    case AST_LT:
    case AST_LE:
    case AST_GT:
    case AST_GE:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      return v7_create_value(v7, V7_TYPE_BOOLEAN,
                             i_bool_bin_op(tag, i_as_num(v1), i_as_num(v2)));
    case AST_ASSIGN:
      /* for now only simple assignment */
      assert((tag = ast_fetch_tag(a, pos)) == AST_IDENT);
      name = ast_get_inlined_data(a, *pos, &name_len);
      ast_move_to_children(a, pos);
      res = i_eval_expr(v7, a, pos, scope);
      v7_set_property_value(v7, scope, name, name_len, 0, res);
      return res;
    case AST_INDEX:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      v7_stringify_value(v7, v2, buf, sizeof(buf));
      return v7_property_value(v7_get_property(v1, buf, -1));
    case AST_MEMBER:
      name = ast_get_inlined_data(a, *pos, &name_len);
      ast_move_to_children(a, pos);
      v1 = i_eval_expr(v7, a, pos, scope);
      return v7_property_value(v7_get_property(v1, name, name_len));
    case AST_SEQ:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        res = i_eval_expr(v7, a, pos, scope);
      }
      return res;
    case AST_ARRAY:
      res = v7_create_value(v7, V7_TYPE_ARRAY_OBJECT);
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      for(i = 0; *pos < end; i++) {
        ast_off_t lookahead = *pos;
        enum ast_tag tag = ast_fetch_tag(a, &lookahead);
        v1 = i_eval_expr(v7, a, pos, scope);
        if (tag != AST_NOP) {
          snprintf(buf, sizeof(buf), "%d", i);
          v7_set_property_value(v7, res, buf, -1, 0, v1);
        }
      }
      return res;
    case AST_OBJECT:
      res = v7_create_value(v7, V7_TYPE_GENERIC_OBJECT);
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        assert((tag = ast_fetch_tag(a, pos)) == AST_PROP);
        name = ast_get_inlined_data(a, *pos, &name_len);
        ast_move_to_children(a, pos);
        v1 = i_eval_expr(v7, a, pos, scope);
        v7_set_property_value(v7, res, name, name_len, 0, v1);
      }
      return res;
    case AST_TRUE:
      return v7_create_value(v7, V7_TYPE_BOOLEAN, 1);
    case AST_FALSE:
      return v7_create_value(v7, V7_TYPE_BOOLEAN, 0);
    case AST_NULL:
      return v7_create_value(v7, V7_TYPE_NULL);
    case AST_NOP:
    case AST_UNDEFINED:
      return v7_create_value(v7, V7_TYPE_UNDEFINED);
    case AST_NUM:
      ast_get_num(a, *pos, &dv);
      ast_move_to_children(a, pos);
      return v7_create_value(v7, V7_TYPE_NUMBER, dv);
    case AST_STRING:
      name = ast_get_inlined_data(a, *pos, &name_len);
      ast_move_to_children(a, pos);
      res = v7_create_value(v7, V7_TYPE_STRING, name,
                            name_len, 1);
      return res;
    case AST_IDENT:
      {
        struct v7_property *p;
        name = ast_get_inlined_data(a, *pos, &name_len);
        ast_move_to_children(a, pos);
        if ((p = v7_get_property(scope, name, name_len)) == NULL) {
          fprintf(stderr, "ReferenceError: %s is not defined\n", buf);
          abort();
        }
        return v7_property_value(p);
      }
    case AST_FUNC:
      {
        val_t func = v7_create_value(v7, V7_TYPE_FUNCTION_OBJECT);
        struct v7_function *funcp = val_to_function(func);
        funcp->ast = a;
        funcp->ast_off = *pos - 1;
        *pos = ast_get_skip(a, *pos, AST_END_SKIP);
        return func;
      }
    case AST_CALL:
      return i_eval_call(v7, a, pos, scope);
    default:
      printf("NOT IMPLEMENTED: %s\n", def->name); /* LCOV_EXCL_LINE */
      abort();
  }
}

static val_t i_eval_call(struct v7 *v7, struct ast *a, ast_off_t *pos, val_t scope) {
  ast_off_t end, fpos, fstart, fend, fargs, fvar, fvar_end, fbody;
  int fbrk = 0;
  val_t frame, res, v1;
  struct v7_function *func;
  char *name;
  v7_strlen_t name_len;

  end = ast_get_skip(a, *pos, AST_END_SKIP);
  ast_move_to_children(a, pos);
  v1 = i_eval_expr(v7, a, pos, scope);
  if (!v7_is_function(v1)) {
    fprintf(stderr, "value is not a function\n"); /* LCOV_EXCL_LINE */
    abort();
  }

  func = val_to_function(v1);
  fpos = func->ast_off;
  fstart = fpos;
  assert(ast_fetch_tag(func->ast, &fpos) == AST_FUNC);
  fend = ast_get_skip(func->ast, fpos, AST_END_SKIP);
  fbody = ast_get_skip(func->ast, fpos, AST_FUNC_BODY_SKIP);
  fvar = ast_get_skip(func->ast, fpos, AST_FUNC_FIRST_VAR_SKIP) - 1;
  ast_move_to_children(func->ast, &fpos);
  ast_skip_tree(a, &fpos);
  fargs = fpos;

  frame = v7_create_value(v7, V7_TYPE_GENERIC_OBJECT);
  /* populate the call frame with a property for each local variable */
  if (fvar != fstart) {
    ast_off_t next;
    fpos = fbody;

    do {
      assert(ast_fetch_tag(a, &fvar) == AST_VAR);
      next = ast_get_skip(a, fvar, AST_VAR_NEXT_SKIP);
      if (next == fvar) {
        next = 0;
      }
      assert(next < 1000);

      fvar_end = ast_get_skip(a, fvar, AST_END_SKIP);
      ast_move_to_children(a, &fvar);
      while (fvar < fvar_end) {
        assert(ast_fetch_tag(a, &fvar) == AST_VAR_DECL);
        name = ast_get_inlined_data(a, fvar, &name_len);
        ast_move_to_children(a, &fvar);
        ast_skip_tree(a, &fvar);

        v7_set_property_value(v7, frame, name, name_len, 0, V7_UNDEFINED);
      }
      fvar = next - 1; /* TODO(mkm): cleanup */
    } while (next != 0);
  }

  /* scan actual and formal arguments and updates the value in the frame */
  fpos = fargs;
  while (fpos < fbody) {
    assert(ast_fetch_tag(a, &fpos) == AST_IDENT);
    name = ast_get_inlined_data(a, fpos, &name_len);
    ast_move_to_children(a, &fpos);

    if (*pos < end) {
      res = i_eval_expr(v7, a, pos, scope);
    } else {
      res = V7_UNDEFINED;
    }
    v7_set_property_value(v7, frame, name, name_len, 0, res);
  }

  /* evaluate trailing actual arguments for side effects */
  while (*pos < end) {
    i_eval_expr(v7, a, pos, scope);
  }

  res = i_eval_stmts(v7, func->ast, &fpos, fend, frame, &fbrk);
  if (fbrk != 0) {
    return res;
  }
  return V7_UNDEFINED;
}

static val_t i_eval_stmt(struct v7 *, struct ast *, ast_off_t *, val_t, int *);

static val_t i_eval_stmts(struct v7 *v7, struct ast *a, ast_off_t *pos,
                          ast_off_t end, val_t scope, int *brk) {
  val_t res;
  while (*pos < end && !*brk) {
    res = i_eval_stmt(v7, a, pos, scope, brk);
  }
  return res;
}

static val_t i_eval_stmt(struct v7 *v7, struct ast *a, ast_off_t *pos,
                         val_t scope, int *brk) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  val_t res;
  ast_off_t end, cond, iter_end, loop, iter, finally, catch;

  switch (tag) {
    case AST_SCRIPT: /* TODO(mkm): push up */
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        res = i_eval_stmt(v7, a, pos, scope, brk);
        /* TODO(mkm): handle illegal returns and breaks in SCRIPT node */
      }
      return res;
    case AST_IF:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      if (i_is_true(i_eval_expr(v7, a, pos, scope))) {
        res = i_eval_stmts(v7, a, pos, end, scope, brk);
        if (*brk) {
          return res;
        }
      }
      *pos = end;
      break;
    case AST_WHILE:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      cond = *pos;
      for (;;) {
        if (i_is_true(i_eval_expr(v7, a, pos, scope))) {
          res = i_eval_stmts(v7, a, pos, end, scope, brk);
          if (*brk) {
            return res;
          }
        } else {
          *pos = end;
          break;
        }
        *pos = cond;
      }
      break;
    case AST_DOWHILE:
      end = ast_get_skip(a, *pos, AST_DO_WHILE_COND_SKIP);
      ast_move_to_children(a, pos);
      loop = *pos;
      for (;;) {
        res = i_eval_stmts(v7, a, pos, end, scope, brk);
        if (*brk) {
          return res;
        }
        if (!i_is_true(i_eval_expr(v7, a, pos, scope))) {
          break;
        }
        *pos = loop;
      }
      break;
    case AST_FOR:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      iter_end = ast_get_skip(a, *pos, AST_FOR_BODY_SKIP);
      ast_move_to_children(a, pos);
      /* initializer */
      i_eval_expr(v7, a, pos, scope);
      for (;;) {
        loop = *pos;
        if (!i_is_true(i_eval_expr(v7, a, &loop, scope))) {
          *pos = end;
          return v7_create_value(v7, V7_TYPE_UNDEFINED);
        }
        iter = loop;
        loop = iter_end;
        res = i_eval_stmts(v7, a, &loop, end, scope, brk);
        if (*brk) {
          return res;
        }
        i_eval_expr(v7, a, &iter, scope);
      }
    case AST_TRY:
      /* Dummy no catch impl */
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      catch = ast_get_skip(a, *pos, AST_TRY_CATCH_SKIP);
      finally = ast_get_skip(a, *pos, AST_TRY_FINALLY_SKIP);
      ast_move_to_children(a, pos);
      res = i_eval_stmts(v7, a, pos, catch, scope, brk);
      if (finally) {
        int brk = 0;
        val_t res = i_eval_stmts(v7, a, &finally, end, scope, &brk);
        if (brk) {
          return res;
        }
      }
      *pos = end;
      return res;
    case AST_WITH:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      /*
       * TODO(mkm) make an actual scope chain. Possibly by mutating
       * the with expression and adding the 'outer environment
       * reference' hidden property.
       */
      i_eval_stmts(v7, a, pos, end, i_eval_expr(v7, a, pos, scope),
                   brk);
      break;
    case AST_VALUE_RETURN:
      res = i_eval_expr(v7, a, pos, scope);
      *brk = 1;
      return res;
    default:
      (*pos)--;
      return i_eval_expr(v7, a, pos, scope);
  }
  return v7_create_value(v7, V7_TYPE_UNDEFINED);
}

V7_PRIVATE val_t v7_exec_2(struct v7 *v7, const char* src) {
  /* TODO(mkm): use GC pool */
  struct ast *a = (struct ast *) malloc(sizeof(struct ast));
  val_t res;
  int brk = 0;
  ast_off_t pos = 0;
  char debug[1024];

  ast_init(a, 0);
  if (aparse(a, src, 1) != V7_OK) {
    printf("Error parsing\n");
    return V7_UNDEFINED;
  }
  ast_optimize(a);

#if 0
  ast_dump(stdout, a, 0);
#endif

  res = i_eval_stmt(v7, a, &pos, v7->global_object, &brk);
  v7_to_json(v7, res, debug, sizeof(debug));
#if 0
  fprintf(stderr, "Eval res: %s .\n", debug);
#endif
  return res;
}
