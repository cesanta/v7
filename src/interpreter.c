/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t i_eval_stmts(struct v7 *, struct ast *, ast_off_t *, ast_off_t,
                          val_t, int *);
static val_t i_eval_call(struct v7 *, struct ast *, ast_off_t *, val_t);

static void throw_exception(struct v7 *v7, const char *err_fmt, ...) {
  va_list ap;
  va_start(ap, err_fmt);
  vsnprintf(v7->error_msg, sizeof(v7->error_msg), err_fmt, ap);
  va_end(ap);
  longjmp(v7->jmp_buf, 1);
}

static void abort_exec(struct v7 *v7, const char *err_fmt, ...) {
  va_list ap;
  va_start(ap, err_fmt);
  vsnprintf(v7->error_msg, sizeof(v7->error_msg), err_fmt, ap);
  va_end(ap);
  longjmp(v7->abort_jmp_buf, 1);
}

static double i_as_num(struct v7 *v7, val_t v) {
  if (!v7_is_double(v) && !v7_is_boolean(v)) {
    if (v7_is_string(v)) {
      size_t n;
      char buf[20], *s = (char *) val_to_string(v7, &v, &n);
      snprintf(buf, sizeof(buf), "%.*s", (int) n, s);
      buf[sizeof(buf) - 1] = '\0';
      return strtod(buf, NULL);
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

static double i_num_unary_op(struct v7 *v7, enum ast_tag tag, double a) {
  switch (tag) {
    case AST_POSITIVE:
      return a;
    case AST_NEGATIVE:
      return -a;
    default:
      abort_exec(v7, "%s", __func__);  /* LCOV_EXCL_LINE */
      return 0;
  }
}

static double i_num_bin_op(struct v7 *v7, enum ast_tag tag, double a, double b) {
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
      abort_exec(v7, "%s", __func__);  /* LCOV_EXCL_LINE */
      return 0;
  }
}

static int i_bool_bin_op(struct v7 *v7, enum ast_tag tag, double a, double b) {
  switch (tag) {
    case AST_EQ:
    case AST_EQ_EQ:   /* TODO(lsm): fix this */
      return a == b;
    case AST_NE:
    case AST_NE_NE:   /* TODO(lsm): fix this */
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
      abort_exec(v7, "%s", __func__);  /* LCOV_EXCL_LINE */
      return 0;
  }
}

static val_t i_eval_expr(struct v7 *v7, struct ast *a, ast_off_t *pos,
                         val_t scope) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  const struct ast_node_def *def = &ast_node_defs[tag];
  ast_off_t end;
  val_t res = V7_UNDEFINED, v1, v2;
  double dv;
  int i;

  /*
   * TODO(mkm): put this temporary somewhere in the evaluation context
   * or use alloca.
   */
  char buf[512];
  char *name;
  size_t name_len;

  switch (tag) {
    case AST_NEGATIVE:
    case AST_POSITIVE:
      return v7_create_value(v7, V7_TYPE_NUMBER, i_num_unary_op(
          v7, tag, i_as_num(v7, i_eval_expr(v7, a, pos, scope))));
    case AST_ADD:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      if (!(v7_is_undefined(v1) || v7_is_double(v1) || v7_is_boolean(v1)) ||
          !(v7_is_undefined(v2) || v7_is_double(v2) || v7_is_boolean(v2))) {
        v7_stringify_value(v7, v1, buf, sizeof(buf));
        v7_stringify_value(v7, v2, buf + strlen(buf),
                           sizeof(buf) - strlen(buf));
        return v7_create_value(v7, V7_TYPE_STRING, buf, strlen(buf));
      }
      return v7_create_value(v7, V7_TYPE_NUMBER, i_num_bin_op(v7, tag,
                             i_as_num(v7, v1), i_as_num(v7, v2)));
    case AST_SUB:
    case AST_REM:
    case AST_MUL:
    case AST_DIV:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      return v7_create_value(v7, V7_TYPE_NUMBER, i_num_bin_op(v7, tag,
                             i_as_num(v7, v1), i_as_num(v7, v2)));
    case AST_EQ:
    case AST_NE:
    case AST_LT:
    case AST_LE:
    case AST_GT:
    case AST_GE:
    case AST_NE_NE:
    case AST_EQ_EQ:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      return v7_create_value(v7, V7_TYPE_BOOLEAN, i_bool_bin_op(v7, tag,
                             i_as_num(v7, v1), i_as_num(v7, v2)));
    case AST_ASSIGN:
      {
        struct v7_property *prop;
        val_t lval, root = v7->global_object;
        switch ((tag = ast_fetch_tag(a, pos))) {
          case AST_IDENT:
            lval = scope;
            name = ast_get_inlined_data(a, *pos, &name_len);
            ast_move_to_children(a, pos);
            break;
          case AST_MEMBER:
            name = ast_get_inlined_data(a, *pos, &name_len);
            ast_move_to_children(a, pos);
            lval = root = i_eval_expr(v7, a, pos, scope);
            break;
          case AST_INDEX:
            lval = root = i_eval_expr(v7, a, pos, scope);
            v1 = i_eval_expr(v7, a, pos, scope);
            name_len = v7_stringify_value(v7, v1, buf, sizeof(buf));
            name = buf;
          default:
            abort_exec(v7, "Invalid left-hand side in assignment");
            return V7_UNDEFINED;  /* unreachable */
        }

        res = i_eval_expr(v7, a, pos, scope);
        /*
         * TODO(mkm): this will incorrectly mutate an existing property in
         * Object.prototype instead of creating a new variable in `global`.
         * `get_property` should also return a pointer to the object where
         * the property is found.
         */
        prop = v7_get_property(lval, name, name_len);
        if (prop == NULL) {
          v7_set_property_value(v7, root, name, name_len, 0, res);
        } else {
          prop->value = res;
        }
        return res;
      }
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
      for (i = 0; *pos < end; i++) {
        ast_off_t lookahead = *pos;
        tag = ast_fetch_tag(a, &lookahead);
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
        tag = ast_fetch_tag(a, pos);
        V7_CHECK(v7, tag == AST_PROP);
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
      res = v7_create_value(v7, V7_TYPE_STRING, name, name_len, 1);
      return res;
    case AST_IDENT:
      {
        struct v7_property *p;
        name = ast_get_inlined_data(a, *pos, &name_len);
        ast_move_to_children(a, pos);
        if ((p = v7_get_property(scope, name, name_len)) == NULL) {
          throw_exception(v7, "ReferenceError: [%.*s] is not defined",
                          (int) name_len, name);
        }
        return v7_property_value(p);
      }
    case AST_FUNC:
      {
        val_t func = v7_create_value(v7, V7_TYPE_FUNCTION_OBJECT);
        struct v7_function *funcp = val_to_function(func);
        funcp->scope = val_to_object(scope);
        funcp->ast = a;
        funcp->ast_off = *pos - 1;
        ast_move_to_children(a, pos);
        tag = ast_fetch_tag(a, pos);
        if (tag == AST_IDENT) {
          name = ast_get_inlined_data(a, *pos, &name_len);
          v7_set_property_value(v7, scope, name, name_len, 0, func);
        }
        *pos = ast_get_skip(a, funcp->ast_off + 1, AST_END_SKIP);
        return func;
      }
    case AST_CALL:
      return i_eval_call(v7, a, pos, scope);
    case AST_VAR:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        tag = ast_fetch_tag(a, pos);
        V7_CHECK(v7, tag == AST_VAR_DECL);
        name = ast_get_inlined_data(a, *pos, &name_len);
        ast_move_to_children(a, pos);
        res = i_eval_expr(v7, a, pos, scope);
        v7_set_property_value(v7, scope, name, name_len, 0, res);
      }
      return res;
    case AST_THIS:
      /* TODO(lsm): fix this */
      return v7->global_object;
    default:
      abort_exec(v7, "%s: %s", __func__, def->name); /* LCOV_EXCL_LINE */
      return V7_UNDEFINED;
  }
}

static val_t i_eval_call(struct v7 *v7, struct ast *a, ast_off_t *pos, val_t scope) {
  ast_off_t end, fpos, fstart, fend, fargs, fvar, fvar_end, fbody;
  int fbrk = 0;
  val_t frame, res, v1;
  struct v7_function *func;
  enum ast_tag tag;
  char *name;
  size_t name_len;

  end = ast_get_skip(a, *pos, AST_END_SKIP);
  ast_move_to_children(a, pos);
  v1 = i_eval_expr(v7, a, pos, scope);

  if (v7_is_cfunction(v1)) {
    char buf[20];
    int n, i;
    val_t args = v7_create_value(v7, V7_TYPE_ARRAY_OBJECT);
    for (i = 0; *pos < end; i++) {
      res = i_eval_expr(v7, a, pos, scope);
      n = snprintf(buf, sizeof(buf), "%d", i);
      v7_set_property_value(v7, args, buf, n, 0, res);
    }
    return val_to_cfunction(v1)(v7, args);
  } if (!v7_is_function(v1)) {
    abort_exec(v7, "%s", "value is not a function"); /* LCOV_EXCL_LINE */
  }

  func = val_to_function(v1);
  fpos = func->ast_off;
  fstart = fpos;
  tag = ast_fetch_tag(func->ast, &fpos);
  V7_CHECK(v7, tag == AST_FUNC);
  fend = ast_get_skip(func->ast, fpos, AST_END_SKIP);
  fbody = ast_get_skip(func->ast, fpos, AST_FUNC_BODY_SKIP);
  fvar = ast_get_skip(func->ast, fpos, AST_FUNC_FIRST_VAR_SKIP) - 1;
  ast_move_to_children(func->ast, &fpos);
  ast_skip_tree(a, &fpos);
  fargs = fpos;

  frame = v7_create_value(v7, V7_TYPE_GENERIC_OBJECT);
  val_to_object(frame)->prototype = func->scope;
  /* populate the call frame with a property for each local variable */
  if (fvar != fstart) {
    ast_off_t next;
    fpos = fbody;

    do {
      tag = ast_fetch_tag(a, &fvar);
      V7_CHECK(v7, tag == AST_VAR);
      next = ast_get_skip(a, fvar, AST_VAR_NEXT_SKIP);
      if (next == fvar) {
        next = 0;
      }
      V7_CHECK(v7, next < 1000);

      fvar_end = ast_get_skip(a, fvar, AST_END_SKIP);
      ast_move_to_children(a, &fvar);
      while (fvar < fvar_end) {
        tag = ast_fetch_tag(a, &fvar);
        V7_CHECK(v7, tag == AST_VAR_DECL);
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
    tag = ast_fetch_tag(a, &fpos);
    V7_CHECK(v7, tag == AST_IDENT);
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
      {
        int percolate = 0;
        jmp_buf old_jmp;
        memcpy(old_jmp, v7->jmp_buf, sizeof(old_jmp));

        end = ast_get_skip(a, *pos, AST_END_SKIP);
        catch = ast_get_skip(a, *pos, AST_TRY_CATCH_SKIP);
        finally = ast_get_skip(a, *pos, AST_TRY_FINALLY_SKIP);
        ast_move_to_children(a, pos);
        if (setjmp(v7->jmp_buf) == 0) {
          res = i_eval_stmts(v7, a, pos, catch, scope, brk);
        } else if (catch != finally) {
          tag = ast_fetch_tag(a, &catch);
          V7_CHECK(v7, tag == AST_IDENT);
          ast_move_to_children(a, &catch);
          memcpy(v7->jmp_buf, old_jmp, sizeof(old_jmp));
          res = i_eval_stmts(v7, a, &catch, finally, scope, brk);
        } else {
          percolate = 1;
        }

        memcpy(v7->jmp_buf, old_jmp, sizeof(old_jmp));
        if (finally) {
          res = i_eval_stmts(v7, a, &finally, end, scope, brk);
          if (!*brk && percolate) {
            longjmp(v7->jmp_buf, 1);
          }
        }
        *pos = end;
        return res;
      }
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
  if (setjmp(v7->abort_jmp_buf) != 0) {
    #if 0
    fprintf(stderr, "Exec abort: %s\n", v7->error_msg);
    #endif
    return V7_UNDEFINED;
  }
  if (setjmp(v7->jmp_buf) != 0) {
    #if 0
    fprintf(stderr, "Exec error: %s\n", v7->error_msg);
    #endif
    return V7_UNDEFINED;
  }
  if (aparse(a, src, 1) != V7_OK) {
    #if 0
    fprintf(stderr, "Error parsing\n");
    #endif
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
