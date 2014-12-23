/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static struct v7_val *i_new_array(struct v7 *v7) {
  struct v7_val * v = v7_push_new_object(v7);
  v7_set_class(v, V7_CLASS_ARRAY);
  return v;
}

static double i_as_num(struct v7_val *v) {
  if (!is_num(v) && !is_bool(v)) {
    if (is_string(v)) {
      return strtod(v->v.str.buf, NULL);
    } else {
      return NAN;
    }
  } else {
    return v7_number(v);
  }
}

static int i_is_true(struct v7_val *v) {
  /* TODO(mkm): real stuff */
  return v7_type(v) == V7_TYPE_NUM && v7_number(v) == 1.0;
}

static double i_num_unary_op(enum ast_tag tag, double a) {
  switch (tag) {
    case AST_POSITIVE:
      return a;
    case AST_NEGATIVE:
      return -a;
    default:
      printf("NOT IMPLEMENTED");
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
      printf("NOT IMPLEMENTED");
      abort();
  }
}

static struct v7_val *i_eval_expr(struct v7 *v7, struct ast *a, ast_off_t *pos,
                                  struct v7_val *scope) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  const struct ast_node_def *def = &ast_node_defs[tag];
  ast_off_t end;
  struct v7_val *res, *v1, *v2;
  double dv;

  /*
   * TODO(mkm): put this temporary somewhere in the evaluation context
   * or use alloca.
   */
  char buf[512];

  switch (tag) {
    case AST_NEGATIVE:
    case AST_POSITIVE:
      return v7_push_number(v7, i_num_unary_op(
          tag, i_as_num(i_eval_expr(v7, a, pos, scope))));
    case AST_ADD:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      if (!is_num(v1) || !is_num(v2)) {
        v7_stringify(v1, buf, sizeof(buf));
        v7_stringify(v2, buf + strlen(buf), sizeof(buf) - strlen(buf));
        return v7_push_string(v7, buf, strlen(buf), 1);
      }
      return v7_push_number(v7, i_num_bin_op(tag, i_as_num(v1), i_as_num(v2)));
    case AST_SUB:
    case AST_REM:
    case AST_MUL:
    case AST_DIV:
    case AST_EQ:
    case AST_NE:
    case AST_LT:
    case AST_LE:
    case AST_GT:
    case AST_GE:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      return v7_push_number(v7, i_num_bin_op(tag, i_as_num(v1), i_as_num(v2)));
    case AST_ASSIGN:
      /* for now only simple assignment */
      assert((tag = ast_fetch_tag(a, pos)) == AST_IDENT);
      ast_get_inlined_data(a, *pos, buf, sizeof(buf));
      ast_move_to_children(a, pos);
      res = i_eval_expr(v7, a, pos, scope);
      v7_set(v7, scope, buf, res);
      return res;
    case AST_INDEX:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      return get2(v1, v2);
    case AST_MEMBER:
      ast_get_inlined_data(a, *pos, buf, sizeof(buf));
      ast_move_to_children(a, pos);
      v1 = i_eval_expr(v7, a, pos, scope);
      return v7_get(v1, buf);
    case AST_SEQ:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        res = i_eval_expr(v7, a, pos, scope);
      }
      return res;
    case AST_ARRAY:
      res = i_new_array(v7);
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        v7_append(v7, res, i_eval_expr(v7, a, pos, scope));
      }
      return res;
    case AST_OBJECT:
      res = v7_push_new_object(v7);
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        assert((tag = ast_fetch_tag(a, pos)) == AST_PROP);
        ast_get_inlined_data(a, *pos, buf, sizeof(buf));
        ast_move_to_children(a, pos);
        v1 = i_eval_expr(v7, a, pos, scope);
        v7_set(v7, res, buf, v1);
      }
      return res;
    case AST_TRUE:
      return v7_push_number(v7, 1.0);  /* TODO(mkm): bool */
    case AST_FALSE:
      return v7_push_number(v7, 0.0);  /* TODO(mkm): bool */
    case AST_NUM:
      ast_get_num(a, *pos, &dv);
      ast_move_to_children(a, pos);
      return v7_push_number(v7, dv);
    case AST_STRING:
      ast_get_inlined_data(a, *pos, buf, sizeof(buf));
      ast_move_to_children(a, pos);
      res = v7_push_string(v7, buf, strlen(buf), 1);
      return res;
    case AST_IDENT:
      ast_get_inlined_data(a, *pos, buf, sizeof(buf));
      ast_move_to_children(a, pos);
      res = v7_get(scope, buf);
      if (res == NULL) {
        fprintf(stderr, "ReferenceError: %s is not defined\n", buf);
        abort();
      }
      return res;
    default:
      printf("NOT IMPLEMENTED: %s\n", def->name);
      abort();
  }
}

static struct v7_val *i_eval_stmt(struct v7 *, struct ast *, ast_off_t *,
                                  struct v7_val *);

static struct v7_val *i_eval_stmts(struct v7 *v7, struct ast *a, ast_off_t *pos,
                                   ast_off_t end, struct v7_val *scope) {
  struct v7_val *res;
  while (*pos < end) {
    res = i_eval_stmt(v7, a, pos, scope);
  }
  return res;
}

static struct v7_val *i_eval_stmt(struct v7 *v7, struct ast *a, ast_off_t *pos,
                                  struct v7_val *scope) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  struct v7_val *res;
  ast_off_t end, cond, iter_end, loop, iter, finally, catch;

  switch (tag) {
    case AST_SCRIPT: /* TODO(mkm): push up */
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        res = i_eval_stmt(v7, a, pos, scope);
      }
      return res;
    case AST_IF:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      if (i_is_true(i_eval_expr(v7, a, pos, scope))) {
        i_eval_stmts(v7, a, pos, end, scope);
      }
      *pos = end;
      break;
    case AST_WHILE:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      cond = *pos;
      for (;;) {
        if (i_is_true(i_eval_expr(v7, a, pos, scope))) {
          i_eval_stmts(v7, a, pos, end, scope);
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
        i_eval_stmts(v7, a, pos, end, scope);
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
          return make_value(v7, V7_TYPE_UNDEF);
        }
        iter = loop;
        loop = iter_end;
        i_eval_stmts(v7, a, &loop, end, scope);
        i_eval_expr(v7, a, &iter, scope);
      }
    case AST_TRY:
      /* Dummy no catch impl */
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      catch = ast_get_skip(a, *pos, AST_TRY_CATCH_SKIP);
      finally = ast_get_skip(a, *pos, AST_TRY_FINALLY_SKIP);
      ast_move_to_children(a, pos);
      i_eval_stmts(v7, a, pos, catch, scope);
      if (finally) {
        i_eval_stmts(v7, a, &finally, end, scope);
      }
      *pos = end;
      break;
    case AST_WITH:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      /*
       * TODO(mkm) make an actual scope chain. Possibly by mutating
       * the with expression and adding the 'outer environment
       * reference' hidden property.
       */
      i_eval_stmts(v7, a, pos, end, i_eval_expr(v7, a, pos, scope));
      break;
    default:
      (*pos)--;
      return i_eval_expr(v7, a, pos, scope);
  }
  return make_value(v7, V7_TYPE_UNDEF);
}

V7_PRIVATE struct v7_val *v7_exec_2(struct v7 *v7, const char* src) {
  struct ast a;
  struct v7_val *res;
  ast_off_t pos = 0;
  char debug[1024];

  ast_init(&a, 0);
  if (aparse(&a, src, 1) != V7_OK) {
    printf("Error parsing\n");
    return NULL;
  }
  ast_trim(&a);

#if 0
  ast_dump(stdout, &a, 0);
#endif

  res = i_eval_stmt(v7, &a, &pos, v7_global(v7));
  v7_stringify(res, debug, sizeof(debug));
#if 0
  fprintf(stderr, "Eval res: %s .\n", debug);
#endif
  return res;
}
