/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#ifdef _WIN32
#define siglongjmp longjmp
#define sigsetjmp(buf, mask) setjmp(buf)
#endif

static enum ast_tag assign_op_map[] = {
  AST_REM, AST_MUL, AST_DIV, AST_XOR, AST_ADD, AST_SUB, AST_OR,
  AST_AND, AST_LSHIFT, AST_RSHIFT, AST_URSHIFT
};

enum i_break {
  B_RUN,
  B_RETURN,
  B_BREAK,
  B_CONTINUE
};

static val_t i_eval_stmts(struct v7 *, struct ast *, ast_off_t *, ast_off_t,
                          val_t, enum i_break *);
static val_t i_eval_call(struct v7 *, struct ast *, ast_off_t *, val_t);
static val_t i_find_this(struct v7 *, struct ast *, ast_off_t, val_t);

static void throw_exception(struct v7 *v7, const char *err_fmt, ...) {
  va_list ap;
  va_start(ap, err_fmt);
  vsnprintf(v7->error_msg, sizeof(v7->error_msg), err_fmt, ap);
  va_end(ap);
  siglongjmp(v7->jmp_buf, 1);
}  /* LCOV_EXCL_LINE */

static void abort_exec(struct v7 *v7, const char *err_fmt, ...) {
  va_list ap;
  va_start(ap, err_fmt);
  vsnprintf(v7->error_msg, sizeof(v7->error_msg), err_fmt, ap);
  va_end(ap);
  siglongjmp(v7->abort_jmp_buf, 1);
}  /* LCOV_EXCL_LINE */

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

static double i_num_unary_op(struct v7 *v7, enum ast_tag tag, double a) {
  switch (tag) {
    case AST_POSITIVE:
      return a;
    case AST_NEGATIVE:
      return -a;
    default:
      abort_exec(v7, "%s", __func__);  /* LCOV_EXCL_LINE */
      return 0;                        /* LCOV_EXCL_LINE */
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
    case AST_LSHIFT:
      return (int) a << (int) b;
    case AST_RSHIFT:
      return (int) a >> (int) b;
    case AST_URSHIFT:
      return (unsigned int) a >> (int) b;
    case AST_OR:
      return (int) a | (int) b;
    case AST_XOR:
      return (int) a ^ (int) b;
    case AST_AND:
      return (int) a & (int) b;
    default:
      abort_exec(v7, "%s", __func__);  /* LCOV_EXCL_LINE */
      return 0;                        /* LCOV_EXCL_LINE */
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
    case AST_LOGICAL_OR:
      return a || b;
    case AST_LOGICAL_AND:
      return a && b;
    default:
      abort_exec(v7, "%s", __func__);  /* LCOV_EXCL_LINE */
      return 0;                        /* LCOV_EXCL_LINE */
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
    case AST_LSHIFT:
    case AST_RSHIFT:
    case AST_URSHIFT:
    case AST_OR:
    case AST_XOR:
    case AST_AND:
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
    case AST_LOGICAL_OR:
    case AST_LOGICAL_AND:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      return v7_create_value(v7, V7_TYPE_BOOLEAN, i_bool_bin_op(v7, tag,
                             i_as_num(v7, v1), i_as_num(v7, v2)));
    case AST_LOGICAL_NOT:
      v1 = i_eval_expr(v7, a, pos, scope);
      return v7_boolean_to_value(! (int) v7_is_true(v7, v1));
    case AST_NOT:
      v1 = i_eval_expr(v7, a, pos, scope);
      return v7_double_to_value(~ (int) i_as_num(v7, v1));
    case AST_ASSIGN:
    case AST_REM_ASSIGN:
    case AST_MUL_ASSIGN:
    case AST_DIV_ASSIGN:
    case AST_XOR_ASSIGN:
    case AST_PLUS_ASSIGN:
    case AST_MINUS_ASSIGN:
    case AST_OR_ASSIGN:
    case AST_AND_ASSIGN:
    case AST_LSHIFT_ASSIGN:
    case AST_RSHIFT_ASSIGN:
    case AST_URSHIFT_ASSIGN:
    case AST_PREINC:
    case AST_PREDEC:
    case AST_POSTINC:
    case AST_POSTDEC:
      {
        struct v7_property *prop;
        enum ast_tag op = tag;
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
            return V7_UNDEFINED;  /* LCOV_EXCL_LINE */
        }

        /*
         * TODO(mkm): this will incorrectly mutate an existing property in
         * Object.prototype instead of creating a new variable in `global`.
         * `get_property` should also return a pointer to the object where
         * the property is found.
         */
        v1 = V7_UNDEFINED;
        prop = v7_get_property(lval, name, name_len);
        if (prop != NULL) {
          v1 = prop->value;
        }

        switch (op) {
          case AST_PREINC:
            v1 = res = v7_double_to_value(i_as_num(v7, v1) + 1.0);
            break;
          case AST_PREDEC:
            v1 = res = v7_double_to_value(i_as_num(v7, v1) - 1.0);
            break;
          case AST_POSTINC:
            res = v1;
            v1 = v7_double_to_value(i_as_num(v7, v1) + 1.0);
            break;
          case AST_POSTDEC:
            res = v1;
            v1 = v7_double_to_value(i_as_num(v7, v1) - 1.0);
            break;
          case AST_ASSIGN:
            v1 = res = i_eval_expr(v7, a, pos, scope);
            break;
          default:
            op = assign_op_map[op - AST_ASSIGN - 1];
            res = i_eval_expr(v7, a, pos, scope);
            res = v1 = v7_double_to_value(i_num_bin_op(
                v7, op, i_as_num(v7, v1), i_as_num(v7, res)));
        }

        /* variables are modified where they are found in the scope chain */
        if (prop != NULL && tag == AST_IDENT) {
          prop->value = v1;
        } else {
          v7_set_property_value(v7, root, name, name_len, 0, v1);
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
      {
        val_t old_this = v7->this_object;
        ast_off_t pp = *pos;
        ast_move_to_children(a, &pp);
        v7->this_object = i_find_this(v7, a, pp, scope);
        res = i_eval_call(v7, a, pos, scope);
        v7->this_object = old_this;
        return res;
      }
    case AST_NEW:
      {
        val_t old_this = v7->this_object;
        v1 = v7->this_object = v7_create_value(v7, V7_TYPE_GENERIC_OBJECT);
        res = i_eval_call(v7, a, pos, scope);
        if (v7_is_undefined(res) || v7_is_null(res)) {
          res = v1;
        }
        v7->this_object = old_this;
        return res;
      }
    case AST_COND:
      if (v7_is_true(v7, i_eval_expr(v7, a, pos, scope))) {
        res = i_eval_expr(v7, a, pos, scope);
        ast_skip_tree(a, pos); /* TODO(mkm): change AST to include skips ? */
      } else {
        ast_skip_tree(a, pos);
        res = i_eval_expr(v7, a, pos, scope);
      }
      return res;
    case AST_IN:
      v1 = i_eval_expr(v7, a, pos, scope);
      v7_stringify_value(v7, v1, buf, sizeof(buf));
      v2 = i_eval_expr(v7, a, pos, scope);
      return v7_boolean_to_value(v7_get_property(v2, buf, -1) != NULL);
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
      return v7->this_object;
    case AST_TYPEOF:
      {
        ast_off_t peek = *pos;
        if ((tag = ast_fetch_tag(a, &peek)) == AST_IDENT) {
          name = ast_get_inlined_data(a, peek, &name_len);
          if (v7_get_property(scope, name, name_len) == NULL) {
            ast_move_to_children(a, &peek);
            *pos = peek;
            /* TODO(mkm): use interned strings*/
            return v7_string_to_value(v7, "undefined", 9, 1);
          }
        }
        /* for some reason lcov doesn't mark the following lines as executing */
        res = i_eval_expr(v7, a, pos, scope);  /* LCOV_EXCL_LINE */
        switch (val_type(v7, res)) {           /* LCOV_EXCL_LINE */
          case V7_TYPE_NUMBER:
            return v7_string_to_value(v7, "number", 6, 1);
          case V7_TYPE_STRING:
            return v7_string_to_value(v7, "string", 6, 1);
          case V7_TYPE_BOOLEAN:
            return v7_string_to_value(v7, "boolean", 7, 1);
          case V7_TYPE_FUNCTION_OBJECT:
            return v7_string_to_value(v7, "function", 8, 1);
          default:
            return v7_string_to_value(v7, "object", 6, 1);
        }
      }
    case AST_DELETE:
      {
        struct v7_property *prop;
        val_t lval = V7_NULL, root = v7->global_object;
        ast_off_t start = *pos;
        switch ((tag = ast_fetch_tag(a, pos))) {
          case AST_IDENT:
            name = ast_get_inlined_data(a, *pos, &name_len);
            ast_move_to_children(a, pos);
            if (v7_get_property(scope, name, name_len) ==
                v7_get_property(root, name, name_len)) {
              lval = root;
            }
            break;
          case AST_MEMBER:
            name = ast_get_inlined_data(a, *pos, &name_len);
            ast_move_to_children(a, pos);
            lval = root = i_eval_expr(v7, a, pos, scope);
            break;
          case AST_INDEX:
            lval = root = i_eval_expr(v7, a, pos, scope);
            res = i_eval_expr(v7, a, pos, scope);
            name_len = v7_stringify_value(v7, res, buf, sizeof(buf));
            name = buf;
            break;
          default:
            *pos = start;
            i_eval_expr(v7, a, pos, scope);
            return v7_boolean_to_value(1);
        }

        prop = v7_get_property(lval, name, name_len);
        if (prop != NULL) {
          v7_del_property(lval, name, name_len);
        }
        return v7_boolean_to_value(1);
      }
    case AST_VOID:
      i_eval_expr(v7, a, pos, scope);
      return V7_UNDEFINED;
    default:
      abort_exec(v7, "%s: %s", __func__, def->name); /* LCOV_EXCL_LINE */
      return V7_UNDEFINED;
  }
}

static val_t i_find_this(struct v7 *v7, struct ast *a, ast_off_t pos, val_t scope) {
  enum ast_tag tag = ast_fetch_tag(a, &pos);
  switch (tag) {
    case AST_MEMBER:
      ast_move_to_children(a, &pos);
      return i_eval_expr(v7, a, &pos, scope);
    case AST_INDEX:
      return i_eval_expr(v7, a, &pos, scope);
    default:
      return v7->global_object;
  }
}

static val_t i_eval_call(struct v7 *v7, struct ast *a, ast_off_t *pos, val_t scope) {
  ast_off_t end, fpos, fstart, fend, fargs, fvar, fvar_end, fbody;
  enum i_break fbrk = B_RUN;
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
  ast_skip_tree(func->ast, &fpos);
  fargs = fpos;

  frame = v7_create_value(v7, V7_TYPE_GENERIC_OBJECT);
  val_to_object(frame)->prototype = func->scope;
  /* populate the call frame with a property for each local variable */
  if (fvar != fstart) {
    ast_off_t next;
    fpos = fbody;

    do {
      tag = ast_fetch_tag(func->ast, &fvar);
      V7_CHECK(v7, tag == AST_VAR);
      next = ast_get_skip(func->ast, fvar, AST_VAR_NEXT_SKIP);
      if (next == fvar) {
        next = 0;
      }
      V7_CHECK(v7, next < 65535);

      fvar_end = ast_get_skip(func->ast, fvar, AST_END_SKIP);
      ast_move_to_children(func->ast, &fvar);
      while (fvar < fvar_end) {
        tag = ast_fetch_tag(func->ast, &fvar);
        V7_CHECK(v7, tag == AST_VAR_DECL);
        name = ast_get_inlined_data(func->ast, fvar, &name_len);
        ast_move_to_children(func->ast, &fvar);
        ast_skip_tree(func->ast, &fvar);

        v7_set_property_value(v7, frame, name, name_len, 0, V7_UNDEFINED);
      }
      fvar = next - 1; /* TODO(mkm): cleanup */
    } while (next != 0);
  }

  /* scan actual and formal arguments and updates the value in the frame */
  fpos = fargs;
  while (fpos < fbody) {
    tag = ast_fetch_tag(func->ast, &fpos);
    V7_CHECK(v7, tag == AST_IDENT);
    name = ast_get_inlined_data(func->ast, fpos, &name_len);
    ast_move_to_children(func->ast, &fpos);

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
  if (fbrk == B_RETURN) {
    return res;
  }
  return V7_UNDEFINED;
}

static val_t i_eval_stmt(struct v7 *, struct ast *, ast_off_t *, val_t, enum i_break *);

static val_t i_eval_stmts(struct v7 *v7, struct ast *a, ast_off_t *pos,
                          ast_off_t end, val_t scope, enum i_break *brk) {
  val_t res = V7_NULL;
  while (*pos < end && !*brk) {
    res = i_eval_stmt(v7, a, pos, scope, brk);
  }
  return res;
}

static val_t i_eval_stmt(struct v7 *v7, struct ast *a, ast_off_t *pos,
                         val_t scope, enum i_break *brk) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  val_t res = V7_NULL;
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
      if (v7_is_true(v7, i_eval_expr(v7, a, pos, scope))) {
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
        if (v7_is_true(v7, i_eval_expr(v7, a, pos, scope))) {
          res = i_eval_stmts(v7, a, pos, end, scope, brk);
          switch (*brk) {
            case B_RUN:
              break;
            case B_CONTINUE:
              *brk = B_RUN;
              break;
            case B_BREAK:
              *brk = B_RUN; /* fall through */
            case B_RETURN:
              *pos = end;
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
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      iter_end = ast_get_skip(a, *pos, AST_DO_WHILE_COND_SKIP);
      ast_move_to_children(a, pos);
      loop = *pos;
      for (;;) {
        res = i_eval_stmts(v7, a, pos, iter_end, scope, brk);
        switch (*brk) {
          case B_RUN:
            break;
          case B_CONTINUE:
            *pos = iter_end;
            *brk = B_RUN;
            break;
          case B_BREAK:
            *brk = B_RUN; /* fall through */
          case B_RETURN:
            *pos = end;
            return res;
        }
        if (!v7_is_true(v7, i_eval_expr(v7, a, pos, scope))) {
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
        if (!v7_is_true(v7, i_eval_expr(v7, a, &loop, scope))) {
          *pos = end;
          return v7_create_value(v7, V7_TYPE_UNDEFINED);
        }
        iter = loop;
        loop = iter_end;
        res = i_eval_stmts(v7, a, &loop, end, scope, brk);
        switch (*brk) {
          case B_RUN:
            break;
          case B_CONTINUE:
            *brk = B_RUN;
            break;
          case B_BREAK:
            *brk = B_RUN; /* fall through */
          case B_RETURN:
            *pos = end;
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
        if (sigsetjmp(v7->jmp_buf, 0) == 0) {
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
            siglongjmp(v7->jmp_buf, 1);
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
      *brk = B_RETURN;
      return res;
    case AST_RETURN:
      *brk = B_RETURN;
      return V7_UNDEFINED;
    case AST_BREAK:
      *brk = B_BREAK;
      return V7_UNDEFINED;
    case AST_CONTINUE:
      *brk = B_CONTINUE;
      return V7_UNDEFINED;
    case AST_THROW:
      /* TODO(mkm): store exception value */
      i_eval_expr(v7, a, pos, scope);
      siglongjmp(v7->jmp_buf, 1);
      break; /* unreachable */
    default:
      (*pos)--;
      return i_eval_expr(v7, a, pos, scope);
  }
  return v7_create_value(v7, V7_TYPE_UNDEFINED);
}

V7_PRIVATE val_t v7_exec(struct v7 *v7, const char* src) {
  /* TODO(mkm): use GC pool */
  struct ast *a = (struct ast *) malloc(sizeof(struct ast));
  val_t res;
  enum i_break brk = B_RUN;
  ast_off_t pos = 0;
  char debug[1024];

  ast_init(a, 0);
  if (sigsetjmp(v7->abort_jmp_buf, 0) != 0) {
    #if 0
    fprintf(stderr, "Exec abort: %s\n", v7->error_msg);
    #endif
    return V7_UNDEFINED;
  }
  if (sigsetjmp(v7->jmp_buf, 0) != 0) {
    #if 0
    fprintf(stderr, "Exec error: %s\n", v7->error_msg);
    #endif
    return V7_UNDEFINED;
  }
  if (parse(v7, a, src, 1) != V7_OK) {
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

val_t v7_exec_file(struct v7 *v7, const char *path) {
  FILE *fp;
  char *p;
  long file_size;
  val_t res = V7_UNDEFINED;

  if ((fp = fopen(path, "r")) == NULL) {
    snprintf(v7->error_msg, sizeof(v7->error_msg), "cannot open file [%s]", path);
  } else if (fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0) {
    fclose(fp);
  } else if ((p = (char *) calloc(1, (size_t) file_size + 1)) == NULL) {
    snprintf(v7->error_msg, sizeof(v7->error_msg), "cannot allocate %ld bytes", file_size + 1);
    fclose(fp);
  } else {
    rewind(fp);
    fread(p, 1, (size_t) file_size, fp);
    fclose(fp);
    res = v7_exec(v7, p);
    free(p);
  }

  return res;
}
