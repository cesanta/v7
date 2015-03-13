/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"
#include "gc.h"

#ifdef _WIN32
#define siglongjmp longjmp
#define sigsetjmp(buf, mask) setjmp(buf)
#endif

static enum ast_tag assign_op_map[] = {
    AST_REM, AST_MUL, AST_DIV,    AST_XOR,    AST_ADD,    AST_SUB,
    AST_OR,  AST_AND, AST_LSHIFT, AST_RSHIFT, AST_URSHIFT};

enum i_break { B_RUN, B_RETURN, B_BREAK, B_CONTINUE };

enum jmp_type { NO_JMP, THROW_JMP, BREAK_JMP, CONTINUE_JMP };

static val_t i_eval_stmts(struct v7 *, struct ast *, ast_off_t *, ast_off_t,
                          val_t, enum i_break *);
static val_t i_eval_expr(struct v7 *, struct ast *, ast_off_t *, val_t);
static val_t i_eval_call(struct v7 *, struct ast *, ast_off_t *, val_t, val_t,
                         int);
static val_t i_find_this(struct v7 *, struct ast *, ast_off_t, val_t);

V7_PRIVATE void throw_value(struct v7 *v7, val_t v) {
  v7->thrown_error = v;
  siglongjmp(v7->jmp_buf, THROW_JMP);
} /* LCOV_EXCL_LINE */

static val_t create_exception(struct v7 *v7, const char *ex, const char *msg) {
  char buf[40];
  val_t e;
  if (v7->creating_exception) {
    fprintf(stderr, "Exception creation throws an exception %s: %s\n", ex, msg);
    return V7_UNDEFINED;
  }
  snprintf(buf, sizeof(buf), "new %s(this)", ex);
  v7->creating_exception++;
  v7_exec_with(v7, &e, buf, v7_create_string(v7, msg, strlen(msg), 1));
  v7->creating_exception--;
  return e;
}

V7_PRIVATE void throw_exception(struct v7 *v7, const char *type,
                                const char *err_fmt, ...) {
  va_list ap;
  va_start(ap, err_fmt);
  vsnprintf(v7->error_msg, sizeof(v7->error_msg), err_fmt, ap);
  va_end(ap);
  throw_value(v7, create_exception(v7, type, v7->error_msg));
} /* LCOV_EXCL_LINE */

V7_PRIVATE val_t i_value_of(struct v7 *v7, val_t v) {
  val_t f = v7_create_undefined();
  struct gc_tmp_frame tf = new_tmp_frame(v7);
  tmp_stack_push(&tf, &v);
  tmp_stack_push(&tf, &f);

  if (v7_is_object(v) && (f = v7_get(v7, v, "valueOf", 7)) != V7_UNDEFINED) {
    v = v7_apply(v7, f, v, v7_create_array(v7));
  }
  tmp_frame_cleanup(&tf);
  return v;
}

V7_PRIVATE double i_as_num(struct v7 *v7, val_t v) {
  double res = 0.0;
  struct gc_tmp_frame tf = new_tmp_frame(v7);
  tmp_stack_push(&tf, &v);

  v = i_value_of(v7, v);
  if (!v7_is_double(v) && !v7_is_boolean(v)) {
    if (v7_is_string(v)) {
      size_t n;
      char buf[20], *e, *s = (char *)v7_to_string(v7, &v, &n);
      if (n != 0) {
        snprintf(buf, sizeof(buf), "%.*s", (int)n, s);
        buf[sizeof(buf) - 1] = '\0';
        res = strtod(buf, &e);
        if (e - n != buf) {
          res = NAN;
        }
      }
    } else if (v7_is_null(v)) {
      res = 0.0;
    } else {
      res = NAN;
    }
  } else {
    if (v7_is_boolean(v)) {
      res = (double)v7_to_boolean(v);
    } else {
      res = v7_to_double(v);
    }
  }

  tmp_frame_cleanup(&tf);
  return res;
}

static double i_num_unary_op(struct v7 *v7, enum ast_tag tag, double a) {
  switch (tag) {
    case AST_POSITIVE:
      return a;
    case AST_NEGATIVE:
      return -a;
    default:
      throw_exception(v7, "InternalError", "%s", __func__); /* LCOV_EXCL_LINE */
      return 0;                                             /* LCOV_EXCL_LINE */
  }
}

static double i_int_bin_op(struct v7 *v7, enum ast_tag tag, double a,
                           double b) {
  int32_t ia = isnan(a) || isinf(a) ? 0 : (int32_t)(int64_t)a;
  int32_t ib = isnan(b) || isinf(b) ? 0 : (int32_t)(int64_t)b;

  switch (tag) {
    case AST_LSHIFT:
      return (int32_t)((uint32_t)ia << ((uint32_t)ib & 31));
    case AST_RSHIFT:
      return ia >> ((uint32_t)ib & 31);
    case AST_URSHIFT:
      return (uint32_t)ia >> ((uint32_t)ib & 31);
    case AST_OR:
      return ia | ib;
    case AST_XOR:
      return ia ^ ib;
    case AST_AND:
      return ia & ib;
    default:
      throw_exception(v7, "InternalError", "%s", __func__); /* LCOV_EXCL_LINE */
      return 0;                                             /* LCOV_EXCL_LINE */
  }
}

#ifdef V7_WINDOWS
static int signbit(double x) {
  return x > 0;
}
#endif

static double i_num_bin_op(struct v7 *v7, enum ast_tag tag, double a,
                           double b) {
  switch (tag) {
    case AST_ADD: /* simple fixed width nodes with no payload */
      return a + b;
    case AST_SUB:
      return a - b;
    case AST_REM:
      if (b == 0 || isnan(b) || isnan(a) || isinf(b) || isinf(a)) {
        return NAN;
      }
      return (int)a % (int)b;
    case AST_MUL:
      return a * b;
    case AST_DIV:
      if (b == 0) {
        return (!signbit(a) == !signbit(b)) ? INFINITY : -INFINITY;
      }
      return a / b;
    case AST_LSHIFT:
    case AST_RSHIFT:
    case AST_URSHIFT:
    case AST_OR:
    case AST_XOR:
    case AST_AND:
      return i_int_bin_op(v7, tag, isnan(a) || isinf(a) ? 0.0 : a,
                          isnan(b) || isinf(b) ? 0.0 : b);
    default:
      throw_exception(v7, "InternalError", "%s", __func__); /* LCOV_EXCL_LINE */
      return 0;                                             /* LCOV_EXCL_LINE */
  }
}

static int i_bool_bin_op(struct v7 *v7, enum ast_tag tag, double a, double b) {
  switch (tag) {
    case AST_EQ:
    case AST_EQ_EQ:
      return a == b;
    case AST_NE:
    case AST_NE_NE:
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
      throw_exception(v7, "InternalError", "%s", __func__); /* LCOV_EXCL_LINE */
      return 0;                                             /* LCOV_EXCL_LINE */
  }
}

static val_t i_eval_expr(struct v7 *v7, struct ast *a, ast_off_t *pos,
                         val_t scope) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  const struct ast_node_def *def = &ast_node_defs[tag];
  ast_off_t end;
  val_t res = v7_create_undefined(), v1 = v7_create_undefined();
  val_t v2 = v7_create_undefined();
  double d1, d2, dv;
  int i;
  /*
   * TODO(mkm): put this temporary somewhere in the evaluation context
   * or use alloca.
   */
  char buf[512];
  char *name, *p;
  size_t name_len;
  struct gc_tmp_frame tf = new_tmp_frame(v7);

  tmp_stack_push(&tf, &res);
  tmp_stack_push(&tf, &v1);
  tmp_stack_push(&tf, &v2);

  switch (tag) {
    case AST_NEGATIVE:
    case AST_POSITIVE:
      res = v7_create_number(i_num_unary_op(
          v7, tag, i_as_num(v7, i_eval_expr(v7, a, pos, scope))));
      break;
    case AST_ADD:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      v1 = i_value_of(v7, v1);
      v2 = i_value_of(v7, v2);
      if (!(v7_is_undefined(v1) || v7_is_double(v1) || v7_is_boolean(v1)) ||
          !(v7_is_undefined(v2) || v7_is_double(v2) || v7_is_boolean(v2))) {
        v7_stringify_value(v7, v1, buf, sizeof(buf));
        v1 = v7_create_string(v7, buf, strlen(buf), 1);
        v7_stringify_value(v7, v2, buf, sizeof(buf));
        v2 = v7_create_string(v7, buf, strlen(buf), 1);
        res = s_concat(v7, v1, v2);
      } else {
        res = v7_create_number(
            i_num_bin_op(v7, tag, i_as_num(v7, v1), i_as_num(v7, v2)));
      }
      break;
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
      d1 = i_as_num(v7, i_eval_expr(v7, a, pos, scope));
      d2 = i_as_num(v7, i_eval_expr(v7, a, pos, scope));
      res = v7_create_number(i_num_bin_op(v7, tag, d1, d2));
      break;
    case AST_EQ_EQ:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      if (v7_is_string(v1) && v7_is_string(v2)) {
        res = v7_create_boolean(s_cmp(v7, v1, v2) == 0);
      } else if (v1 == v2 && v1 == V7_TAG_NAN) {
        res = v7_create_boolean(0);
      } else {
        res = v7_create_boolean(v1 == v2);
      }
      break;
    case AST_NE_NE:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      if (v7_is_string(v1) && v7_is_string(v2)) {
        res = v7_create_boolean(s_cmp(v7, v1, v2) != 0);
      } else if (v1 == v2 && v1 == V7_TAG_NAN) {
        res = v7_create_boolean(1);
      } else {
        res = v7_create_boolean(v1 != v2);
      }
      break;
    case AST_EQ:
    case AST_NE:
    case AST_LT:
    case AST_LE:
    case AST_GT:
    case AST_GE:
      v1 = i_value_of(v7, i_eval_expr(v7, a, pos, scope));
      v2 = i_value_of(v7, i_eval_expr(v7, a, pos, scope));
      if (tag == AST_EQ || tag == AST_NE) {
        if (((v7_is_object(v1) || v7_is_object(v2)) && v1 == v2)) {
          res = v7_create_boolean(tag == AST_EQ);
          break;
        } else if (v7_is_undefined(v1) || v7_is_null(v1)) {
          res = v7_create_boolean((tag != AST_EQ) ^
                                  (v7_is_undefined(v2) || v7_is_null(v2)));
          break;
        } else if (v7_is_undefined(v2) || v7_is_null(v2)) {
          res = v7_create_boolean((tag != AST_EQ) ^
                                  (v7_is_undefined(v1) || v7_is_null(v1)));
          break;
        }
      }
      if (v7_is_string(v1) && v7_is_string(v2)) {
        int cmp = s_cmp(v7, v1, v2);
        switch (tag) {
          case AST_EQ:
            res = v7_create_boolean(cmp == 0);
            break;
          case AST_NE:
            res = v7_create_boolean(cmp != 0);
            break;
          case AST_LT:
            res = v7_create_boolean(cmp < 0);
            break;
          case AST_LE:
            res = v7_create_boolean(cmp <= 0);
            break;
          case AST_GT:
            res = v7_create_boolean(cmp > 0);
            break;
          case AST_GE:
            res = v7_create_boolean(cmp >= 0);
            break;
          default:
            throw_exception(v7, "InternalError", "Unhandled op");
        }
      } else {
        res = v7_create_boolean(
            i_bool_bin_op(v7, tag, i_as_num(v7, v1), i_as_num(v7, v2)));
      }
      break;
    case AST_LOGICAL_OR:
      v1 = i_eval_expr(v7, a, pos, scope);
      if (v7_is_true(v7, v1)) {
        ast_skip_tree(a, pos);
        res = v1;
      } else {
        res = i_eval_expr(v7, a, pos, scope);
      }
      break;
    case AST_LOGICAL_AND:
      v1 = i_eval_expr(v7, a, pos, scope);
      if (!v7_is_true(v7, v1)) {
        ast_skip_tree(a, pos);
        res = v1;
      } else {
        res = i_eval_expr(v7, a, pos, scope);
      }
      break;
    case AST_LOGICAL_NOT:
      v1 = i_eval_expr(v7, a, pos, scope);
      res = v7_create_boolean(!(int64_t)v7_is_true(v7, v1));
      break;
    case AST_NOT:
      v1 = i_eval_expr(v7, a, pos, scope);
      d1 = i_as_num(v7, v1);
      if (isnan(d1) || isinf(d1)) {
        res = v7_create_number(-1);
      } else {
        res = v7_create_number(~(int64_t)d1);
      }
      break;
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
    case AST_POSTDEC: {
      struct v7_property *prop;
      enum ast_tag op = tag;
      val_t lval = v7_create_undefined(), root = v7->global_object;
      tmp_stack_push(&tf, &lval);
      tmp_stack_push(&tf, &root);
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
          break;
        default:
          throw_exception(v7, "ReferenceError",
                          "Invalid left-hand side in assignment");
          /* unreacheable */
          return v7_create_undefined(); /* LCOV_EXCL_LINE */
      }

      /*
       * TODO(mkm): this will incorrectly mutate an existing property in
       * Object.prototype instead of creating a new variable in `global`.
       * `get_property` should also return a pointer to the object where
       * the property is found.
       */
      v1 = v7_create_undefined();
      prop = v7_get_property(v7, lval, name, name_len);
      if (prop != NULL) {
        v1 = prop->value;
      }

      switch (op) {
        case AST_PREINC:
          v1 = res = v7_create_number(i_as_num(v7, v1) + 1.0);
          break;
        case AST_PREDEC:
          v1 = res = v7_create_number(i_as_num(v7, v1) - 1.0);
          break;
        case AST_POSTINC:
          res = i_value_of(v7, v1);
          v1 = v7_create_number(i_as_num(v7, v1) + 1.0);
          break;
        case AST_POSTDEC:
          res = i_value_of(v7, v1);
          v1 = v7_create_number(i_as_num(v7, v1) - 1.0);
          break;
        case AST_ASSIGN:
          v1 = res = i_eval_expr(v7, a, pos, scope);
          break;
        case AST_PLUS_ASSIGN:
          res = i_eval_expr(v7, a, pos, scope);
          v1 = i_value_of(v7, v1);
          res = i_value_of(v7, res);
          if (!(v7_is_undefined(v1) || v7_is_double(v1) || v7_is_boolean(v1)) ||
              !(v7_is_undefined(res) || v7_is_double(res) ||
                v7_is_boolean(res))) {
            v7_stringify_value(v7, v1, buf, sizeof(buf));
            v1 = v7_create_string(v7, buf, strlen(buf), 1);
            v7_stringify_value(v7, res, buf, sizeof(buf));
            res = v7_create_string(v7, buf, strlen(buf), 1);
            v1 = res = s_concat(v7, v1, res);
            break;
          }
          res = v1 = v7_create_number(
              i_num_bin_op(v7, AST_ADD, i_as_num(v7, v1), i_as_num(v7, res)));
          break;
        default:
          op = assign_op_map[op - AST_ASSIGN - 1];
          res = i_eval_expr(v7, a, pos, scope);
          d1 = i_as_num(v7, v1);
          d2 = i_as_num(v7, res);
          res = v1 = v7_create_number(i_num_bin_op(v7, op, d1, d2));
      }

      /* variables are modified where they are found in the scope chain */
      if (prop != NULL && tag == AST_IDENT) {
        prop->value = v1;
      } else if (prop != NULL && prop->attributes & V7_PROPERTY_READ_ONLY) {
        /* nop */
      } else if (prop != NULL && prop->attributes & V7_PROPERTY_SETTER) {
        v7_invoke_setter(v7, prop, root, v1);
      } else {
        v7_set_property(v7, root, name, name_len, 0, v1);
      }
      break;
    }
    case AST_INDEX:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      v7_stringify_value(v7, v2, buf, sizeof(buf));
      res = v7_get(v7, v1, buf, -1);
      break;
    case AST_MEMBER:
      name = ast_get_inlined_data(a, *pos, &name_len);
      ast_move_to_children(a, pos);
      v1 = i_eval_expr(v7, a, pos, scope);
      res = v7_get(v7, v1, name, name_len);
      break;
    case AST_SEQ:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        res = i_eval_expr(v7, a, pos, scope);
      }
      break;
    case AST_ARRAY:
      res = v7_create_array(v7);
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      for (i = 0; *pos < end; i++) {
        ast_off_t lookahead = *pos;
        tag = ast_fetch_tag(a, &lookahead);
        v1 = i_eval_expr(v7, a, pos, scope);
        if (tag != AST_NOP) {
          snprintf(buf, sizeof(buf), "%d", i);
          v7_set_property(v7, res, buf, -1, 0, v1);
        }
      }
      break;
    case AST_OBJECT:
      res = v7_create_object(v7);
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        tag = ast_fetch_tag(a, pos);
        switch (tag) {
          case AST_PROP:
            name = ast_get_inlined_data(a, *pos, &name_len);
            ast_move_to_children(a, pos);
            v1 = i_eval_expr(v7, a, pos, scope);
            if (v7->strict_mode &&
                v7_get_own_property(v7, res, name, name_len) != NULL) {
              /* Ideally this should be thrown at parse time */
              throw_exception(v7, "SyntaxError",
                              "duplicate data property in object literal "
                              "not allowed in strict mode");
            }
            v7_set_property(v7, res, name, name_len, 0, v1);
            break;
          case AST_GETTER:
          case AST_SETTER: {
            ast_off_t func = *pos;
            unsigned int attr =
                tag == AST_GETTER ? V7_PROPERTY_GETTER : V7_PROPERTY_SETTER;
            unsigned int other =
                tag == AST_GETTER ? V7_PROPERTY_SETTER : V7_PROPERTY_GETTER;
            struct v7_property *p;
            V7_CHECK(v7, ast_fetch_tag(a, &func) == AST_FUNC);
            ast_move_to_children(a, &func);
            V7_CHECK(v7, ast_fetch_tag(a, &func) == AST_IDENT);
            name = ast_get_inlined_data(a, func, &name_len);
            v1 = i_eval_expr(v7, a, pos, scope);
            if ((p = v7_get_property(v7, res, name, name_len)) &&
                p->attributes & other) {
              val_t arr = v7_create_array(v7);
              tmp_stack_push(&tf, &arr);
              v7_set(v7, arr, tag == AST_GETTER ? "1" : "0", 1, p->value);
              v7_set(v7, arr, tag == AST_SETTER ? "1" : "0", 1, v1);
              p->value = arr;
              p->attributes |= attr;
            } else {
              v7_set_property(v7, res, name, name_len, attr, v1);
            }
            break;
          }
          default:
            throw_exception(v7, "InternalError",
                            "Expecting AST_(PROP|GETTER|SETTER) got %d", tag);
        }
      }
      break;
    case AST_TRUE:
      res = v7_create_boolean(1);
      break;
    case AST_FALSE:
      res = v7_create_boolean(0);
      break;
    case AST_NULL:
      res = v7_create_null();
      break;
    case AST_USE_STRICT:
    case AST_NOP:
    case AST_UNDEFINED:
      res = v7_create_undefined();
      break;
    case AST_NUM:
      ast_get_num(a, *pos, &dv);
      ast_move_to_children(a, pos);
      res = v7_create_number(dv);
      break;
    case AST_STRING:
      name = ast_get_inlined_data(a, *pos, &name_len);
      ast_move_to_children(a, pos);
      res = v7_create_string(v7, name, name_len, 1);
      break;
    case AST_REGEX:
      name = ast_get_inlined_data(a, *pos, &name_len);
      ast_move_to_children(a, pos);
      for (p = name + name_len - 1; *p != '/';) p--;
      res = v7_create_regexp(v7, name + 1, p - (name + 1), p + 1,
                             (name + name_len) - p - 1);
      break;
    case AST_IDENT: {
      struct v7_property *p;
      name = ast_get_inlined_data(a, *pos, &name_len);
      ast_move_to_children(a, pos);
      if ((p = v7_get_property(v7, scope, name, name_len)) == NULL) {
        throw_exception(v7, "ReferenceError", "[%.*s] is not defined",
                        (int)name_len, name);
      }
      res = v7_property_value(v7, scope, p);
      break;
    }
    case AST_FUNC: {
      ast_off_t fbody;
      struct v7_function *funcp;
      res = v7_create_function(v7);
      funcp = v7_to_function(res);
      tmp_stack_push(&tf, &res);
      funcp->scope = v7_to_object(scope);
      funcp->ast = a;
      funcp->ast_off = *pos - 1;
      ast_move_to_children(a, pos);
      tag = ast_fetch_tag(a, pos);
      if (tag == AST_IDENT) {
        name = ast_get_inlined_data(a, *pos, &name_len);
        v7_set_property(v7, scope, name, name_len, 0, res);
      }
      *pos = ast_get_skip(a, funcp->ast_off + 1, AST_END_SKIP);
      fbody = ast_get_skip(a, funcp->ast_off + 1, AST_FUNC_BODY_SKIP);
      if (fbody < *pos && (tag = ast_fetch_tag(a, &fbody)) == AST_USE_STRICT) {
        funcp->attributes |= V7_FUNCTION_STRICT;
      }
      break;
    }
    case AST_CALL: {
      ast_off_t pp = *pos;
      ast_move_to_children(a, &pp);
      res = i_eval_call(v7, a, pos, scope, i_find_this(v7, a, pp, scope), 0);
      break;
    }
    case AST_NEW:
      v1 = v7_create_object(v7);
      res = i_eval_call(v7, a, pos, scope, v1, 1);
      if (v7_is_undefined(res) || v7_is_null(res)) {
        res = v1;
      }
      break;
    case AST_COND:
      if (v7_is_true(v7, i_eval_expr(v7, a, pos, scope))) {
        res = i_eval_expr(v7, a, pos, scope);
        ast_skip_tree(a, pos); /* TODO(mkm): change AST to include skips ? */
      } else {
        ast_skip_tree(a, pos);
        res = i_eval_expr(v7, a, pos, scope);
      }
      break;
    case AST_IN:
      v1 = i_eval_expr(v7, a, pos, scope);
      v7_stringify_value(v7, v1, buf, sizeof(buf));
      v2 = i_eval_expr(v7, a, pos, scope);
      res = v7_create_boolean(v7_get_property(v7, v2, buf, -1) != NULL);
      break;
    case AST_VAR:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        struct v7_property *prop;
        tag = ast_fetch_tag(a, pos);
        /* func declarations are already set during hoisting */
        if (tag == AST_FUNC_DECL) {
          ast_move_to_children(a, pos);
          ast_skip_tree(a, pos);
          continue;
        }

        V7_CHECK(v7, tag == AST_VAR_DECL);
        name = ast_get_inlined_data(a, *pos, &name_len);
        ast_move_to_children(a, pos);
        res = i_eval_expr(v7, a, pos, scope);
        /*
         * Var decls are hoisted when the function frame is created. Vars
         * declared inside a `with` or `catch` block belong to the function
         * lexical scope, and although those clauses create an inner frame
         * no new variables should be created in it. A var decl thus
         * behaves as a normal assignment at runtime.
         */
        if ((prop = v7_get_property(v7, scope, name, name_len)) != NULL) {
          prop->value = res;
        } else {
          v7_set_property(v7, v7->global_object, name, name_len, 0, res);
        }
      }
      break;
    case AST_THIS:
      res = v7->this_object;
      break;
    case AST_TYPEOF: {
      ast_off_t peek = *pos;
      if ((tag = ast_fetch_tag(a, &peek)) == AST_IDENT) {
        name = ast_get_inlined_data(a, peek, &name_len);
        if (v7_get_property(v7, scope, name, name_len) == NULL) {
          ast_move_to_children(a, &peek);
          *pos = peek;
          /* TODO(mkm): use interned strings */
          res = v7_create_string(v7, "undefined", 9, 1);
          break;
        }
      }
      /* for some reason lcov doesn't mark the following lines as executing */
      res = i_eval_expr(v7, a, pos, scope); /* LCOV_EXCL_LINE */
      switch (val_type(v7, res)) {          /* LCOV_EXCL_LINE */
        case V7_TYPE_NUMBER:
          res = v7_create_string(v7, "number", 6, 1);
          break;
        case V7_TYPE_STRING:
          res = v7_create_string(v7, "string", 6, 1);
          break;
        case V7_TYPE_BOOLEAN:
          res = v7_create_string(v7, "boolean", 7, 1);
          break;
        case V7_TYPE_FUNCTION_OBJECT:
        case V7_TYPE_CFUNCTION_OBJECT:
        case V7_TYPE_CFUNCTION:
          res = v7_create_string(v7, "function", 8, 1);
          break;
        default:
          res = v7_create_string(v7, "object", 6, 1);
          break;
      }
      break;
    }
    case AST_DELETE: {
      struct v7_property *prop;
      val_t lval = v7_create_null(), root = v7->global_object;
      ast_off_t start = *pos;
      tmp_stack_push(&tf, &lval);
      tmp_stack_push(&tf, &root);
      switch ((tag = ast_fetch_tag(a, pos))) {
        case AST_IDENT:
          name = ast_get_inlined_data(a, *pos, &name_len);
          ast_move_to_children(a, pos);
          if (v7_get_property(v7, scope, name, name_len) ==
              v7_get_property(v7, root, name, name_len)) {
            lval = root;
          }
          if (v7->strict_mode) {
            throw_exception(v7, "SyntaxError", "Delete in strict");
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
          return v7_create_boolean(1);
      }

      prop = v7_get_property(v7, lval, name, name_len);
      if (prop != NULL) {
        if (prop->attributes & V7_PROPERTY_DONT_DELETE) {
          res = v7_create_boolean(0);
          break;
        }
        v7_del_property(v7, lval, name, name_len);
      }
      res = v7_create_boolean(1);
      break;
    }
    case AST_INSTANCEOF:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      if (!v7_is_function(v2) && !v7_is_cfunction(i_value_of(v7, v2))) {
        throw_exception(v7, "TypeError",
                        "Expecting a function in instanceof check");
      }
      res = v7_create_boolean(
          is_prototype_of(v7, v1, v7_get(v7, v2, "prototype", 9)));
      break;
    case AST_VOID:
      i_eval_expr(v7, a, pos, scope);
      res = v7_create_undefined();
      break;
    default:
      throw_exception(v7, "InternalError", "%s: %s", __func__,
                      def->name); /* LCOV_EXCL_LINE */
      /* unreacheable */
      break;
  }

  tmp_frame_cleanup(&tf);
  return res;
}

static val_t i_find_this(struct v7 *v7, struct ast *a, ast_off_t pos,
                         val_t scope) {
  enum ast_tag tag = ast_fetch_tag(a, &pos);
  switch (tag) {
    case AST_MEMBER:
      ast_move_to_children(a, &pos);
      return i_eval_expr(v7, a, &pos, scope);
    case AST_INDEX:
      return i_eval_expr(v7, a, &pos, scope);
    default:
      return V7_UNDEFINED;
  }
}

static void i_populate_local_vars(struct v7 *v7, struct ast *a, ast_off_t start,
                                  ast_off_t fvar, val_t frame) {
  enum ast_tag tag;
  ast_off_t next, fvar_end;
  char *name;
  size_t name_len;
  val_t val = v7_create_undefined();
  struct gc_tmp_frame tf = new_tmp_frame(v7);

  if (fvar == start) {
    tmp_frame_cleanup(&tf);
    return;
  }

  tmp_stack_push(&tf, &val);

  do {
    tag = ast_fetch_tag(a, &fvar);
    V7_CHECK(v7, tag == AST_VAR);
    next = ast_get_skip(a, fvar, AST_VAR_NEXT_SKIP);
    if (next == fvar) {
      next = 0;
    }
    V7_CHECK(v7, next < 1024 * 128);

    fvar_end = ast_get_skip(a, fvar, AST_END_SKIP);
    ast_move_to_children(a, &fvar);
    while (fvar < fvar_end) {
      val = v7_create_undefined();
      tag = ast_fetch_tag(a, &fvar);
      V7_CHECK(v7, tag == AST_VAR_DECL || tag == AST_FUNC_DECL);
      name = ast_get_inlined_data(a, fvar, &name_len);
      ast_move_to_children(a, &fvar);
      if (tag == AST_VAR_DECL) {
        ast_skip_tree(a, &fvar);
      } else {
        val = i_eval_expr(v7, a, &fvar, frame);
      }
      v7_set_property(v7, frame, name, name_len, 0, val);
    }
    if (next > 0) {
      fvar = next - 1; /* TODO(mkm): cleanup */
    }
  } while (next != 0);

  tmp_frame_cleanup(&tf);
}

V7_PRIVATE val_t i_prepare_call(struct v7 *v7, struct v7_function *func,
                                ast_off_t *pos, ast_off_t *body,
                                ast_off_t *end) {
  val_t frame;
  enum ast_tag tag;
  ast_off_t fstart, fvar;

  *pos = func->ast_off;
  fstart = *pos;
  tag = ast_fetch_tag(func->ast, pos);
  V7_CHECK(v7, tag == AST_FUNC);
  *end = ast_get_skip(func->ast, *pos, AST_END_SKIP);
  *body = ast_get_skip(func->ast, *pos, AST_FUNC_BODY_SKIP);
  fvar = ast_get_skip(func->ast, *pos, AST_FUNC_FIRST_VAR_SKIP) - 1;
  ast_move_to_children(func->ast, pos);
  ast_skip_tree(func->ast, pos);

  frame = v7_create_object(v7);
  v7_to_object(frame)->prototype = func->scope;

  i_populate_local_vars(v7, func->ast, fstart, fvar, frame);
  return frame;
}

V7_PRIVATE val_t i_invoke_function(struct v7 *v7, struct v7_function *func,
                                   val_t frame, ast_off_t body, ast_off_t end) {
  int saved_strict_mode = v7->strict_mode;
  enum i_break brk = B_RUN;
  val_t res = v7_create_undefined(), saved_call_stack = v7->call_stack;
  struct gc_tmp_frame tf = new_tmp_frame(v7);

  tmp_stack_push(&tf, &res);
  if (func->attributes & V7_FUNCTION_STRICT) {
    v7->strict_mode = 1;
  }
  v7->call_stack = frame; /* ensure GC knows about this call frame */
  res = i_eval_stmts(v7, func->ast, &body, end, frame, &brk);
  if (brk != B_RETURN) {
    res = v7_create_undefined();
  }
  v7->strict_mode = saved_strict_mode;
  v7->call_stack = saved_call_stack;

  tmp_frame_cleanup(&tf);
  return res;
}

static val_t i_eval_call(struct v7 *v7, struct ast *a, ast_off_t *pos,
                         val_t scope, val_t this_object, int is_constructor) {
  ast_off_t end, fpos, fend, fbody;
  val_t frame = v7_create_undefined(), res = v7_create_undefined();
  val_t v1 = v7_create_undefined(), args = v7_create_undefined();
  val_t old_this = v7->this_object;
  struct v7_function *func;
  enum ast_tag tag;
  char *name;
  size_t name_len;
  char buf[20];
  int i, n;

  struct gc_tmp_frame tf = new_tmp_frame(v7);
  tmp_stack_push(&tf, &frame);
  tmp_stack_push(&tf, &res);
  tmp_stack_push(&tf, &v1);
  tmp_stack_push(&tf, &args);
  tmp_stack_push(&tf, &old_this);

  end = ast_get_skip(a, *pos, AST_END_SKIP);
  ast_move_to_children(a, pos);
  v1 = i_eval_expr(v7, a, pos, scope);
  if (!v7_is_cfunction(v1) && !v7_is_function(v1)) {
    v1 = i_value_of(v7, v1);
  }

  if (v7_is_cfunction(v1)) {
    args = v7_create_array(v7);
    for (i = 0; *pos < end; i++) {
      res = i_eval_expr(v7, a, pos, scope);
      n = snprintf(buf, sizeof(buf), "%d", i);
      v7_set_property(v7, args, buf, n, 0, res);
    }
    res = v7_to_cfunction(v1)(v7, this_object, args);
    goto cleanup;
  }
  if (!v7_is_function(v1)) {
    throw_exception(v7, "TypeError", "%s",
                    "value is not a function"); /* LCOV_EXCL_LINE */
  }

  func = v7_to_function(v1);
  if (is_constructor) {
    val_t fun_proto = v7_get(v7, v1, "prototype", 9);
    tmp_stack_push(&tf, &fun_proto);
    if (!v7_is_object(fun_proto)) {
      /* TODO(mkm): box primitive value */
      throw_exception(v7, "TypeError",
                      "Cannot set a primitive value as object prototype");
    }
    v7_to_object(this_object)->prototype = v7_to_object(fun_proto);
  } else if (v7_is_undefined(this_object) &&
             !(func->attributes & V7_FUNCTION_STRICT)) {
    /*
     * null and undefined are replaced with `global` in non-strict mode,
     * as per ECMA-262 6th, 19.2.3.3.
     */
    this_object = v7->global_object;
  }

  frame = i_prepare_call(v7, func, &fpos, &fbody, &fend);

  /*
   * TODO(mkm): don't create args array if the parser didn't see
   * any `arguments` or `eval` identifier being referenced in the function.
   */
  args = v7_create_array(v7);

  /* scan actual and formal arguments and updates the value in the frame */
  for (i = 0; fpos < fbody; i++) {
    tag = ast_fetch_tag(func->ast, &fpos);
    V7_CHECK(v7, tag == AST_IDENT);
    name = ast_get_inlined_data(func->ast, fpos, &name_len);
    ast_move_to_children(func->ast, &fpos);

    if (*pos < end) {
      res = i_eval_expr(v7, a, pos, scope);
      if (!v7_is_undefined(args)) {
        n = snprintf(buf, sizeof(buf), "%d", i);
        v7_set_property(v7, args, buf, n, 0, res);
      }
    } else {
      res = v7_create_undefined();
    }

    v7_set_property(v7, frame, name, name_len, 0, res);
  }

  /* evaluate trailing actual arguments for side effects */
  for (; *pos < end; i++) {
    res = i_eval_expr(v7, a, pos, scope);
    if (!v7_is_undefined(args)) {
      n = snprintf(buf, sizeof(buf), "%d", i);
      v7_set_property(v7, args, buf, n, 0, res);
    }
  }

  if (!v7_is_undefined(args)) {
#ifndef V7_DISABLE_PREDEFINED_STRINGS
    v7_set_v(v7, frame, v7->predefined_strings[PREDEFINED_STR_ARGUMENTS], args);
#else
    v7_set(v7, frame, "arguments", 9, args);
#endif
  }

  v7->this_object = this_object;
  res = i_invoke_function(v7, func, frame, fbody, fend);
  v7->this_object = old_this;

cleanup:
  tmp_frame_cleanup(&tf);
  return res;
}

static val_t i_eval_stmt(struct v7 *, struct ast *, ast_off_t *, val_t,
                         enum i_break *);

static val_t i_eval_stmts(struct v7 *v7, struct ast *a, ast_off_t *pos,
                          ast_off_t end, val_t scope, enum i_break *brk) {
  val_t res = v7_create_undefined();
  while (*pos < end && !*brk) {
    res = i_eval_stmt(v7, a, pos, scope, brk);
  }
  return res;
}

static val_t i_eval_stmt(struct v7 *v7, struct ast *a, ast_off_t *pos,
                         val_t scope, enum i_break *brk) {
  ast_off_t maybe_strict, start = *pos;
  enum ast_tag tag = ast_fetch_tag(a, pos);
  val_t res = v7_create_undefined();
  ast_off_t end, end_true, cond, iter_end, loop, iter, finally, acatch, fvar;
  int saved_strict_mode = v7->strict_mode;
  struct gc_tmp_frame tf = new_tmp_frame(v7);
  tmp_stack_push(&tf, &res);

  switch (tag) {
    case AST_SCRIPT:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      fvar = ast_get_skip(a, *pos, AST_FUNC_FIRST_VAR_SKIP) - 1;
      ast_move_to_children(a, pos);
      maybe_strict = *pos;
      if (*pos < end &&
          (tag = ast_fetch_tag(a, &maybe_strict)) == AST_USE_STRICT) {
        v7->strict_mode = 1;
        *pos = maybe_strict;
      }
      i_populate_local_vars(v7, a, start, fvar, scope);
      res = i_eval_stmts(v7, a, pos, end, scope, brk);
      v7->strict_mode = saved_strict_mode;
      break;
    case AST_IF:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      end_true = ast_get_skip(a, *pos, AST_END_IF_TRUE_SKIP);
      ast_move_to_children(a, pos);
      if (v7_is_true(v7, i_eval_expr(v7, a, pos, scope))) {
        res = i_eval_stmts(v7, a, pos, end_true, scope, brk);
        if (*brk != B_RUN) {
          break;
        }
      } else {
        res = i_eval_stmts(v7, a, &end_true, end, scope, brk);
        if (*brk != B_RUN) {
          break;
        }
      }
      *pos = end;
      break;
    case AST_WHILE:
      v7->lab_cont = 0;
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
              goto cleanup;
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
      /* skip to condition if coming from a labeled continue */
      if (v7->lab_cont) {
        *pos = iter_end;
        v7->lab_cont = 0;
      }
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
            goto cleanup;
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
      if (!v7->lab_cont) {
        i_eval_expr(v7, a, pos, scope);
      } else {
        ast_skip_tree(a, pos);
        iter = *pos;
        ast_skip_tree(a, &iter);
        i_eval_expr(v7, a, &iter, scope);
        v7->lab_cont = 0;
      }
      for (;;) {
        loop = *pos;
        if (!v7_is_true(v7, i_eval_expr(v7, a, &loop, scope))) {
          *pos = end;
          goto cleanup;
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
            goto cleanup;
        }
        i_eval_expr(v7, a, &iter, scope);
      }
    case AST_FOR_IN: {
      char *name;
      size_t name_len;
      val_t obj, key;
      ast_off_t loop;
      struct v7_property *p, *var;
      tmp_stack_push(&tf, &obj);
      tmp_stack_push(&tf, &key);

      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      tag = ast_fetch_tag(a, pos);
      /* TODO(mkm) accept any l-value */
      if (tag == AST_VAR) {
        ast_move_to_children(a, pos);
        tag = ast_fetch_tag(a, pos);
        V7_CHECK(v7, tag == AST_VAR_DECL);
        name = ast_get_inlined_data(a, *pos, &name_len);
        ast_move_to_children(a, pos);
        ast_skip_tree(a, pos);
      } else {
        V7_CHECK(v7, tag == AST_IDENT);
        name = ast_get_inlined_data(a, *pos, &name_len);
        ast_move_to_children(a, pos);
      }

      obj = i_eval_expr(v7, a, pos, scope);
      if (!v7_is_object(obj)) {
        *pos = end;
        goto cleanup;
      }
      ast_skip_tree(a, pos);
      loop = *pos;

      for (p = v7_to_object(obj)->properties; p; p = p->next, *pos = loop) {
        if (p->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM)) {
          continue;
        }
        key = p->name;
        if ((var = v7_get_property(v7, scope, name, name_len)) != NULL) {
          var->value = key;
        } else {
          v7_set_property(v7, v7->global_object, name, name_len, 0, key);
        }

        /* for some reason lcov doesn't mark the following lines executing */
        res = i_eval_stmts(v7, a, pos, end, scope, brk); /* LCOV_EXCL_LINE */
        switch (*brk) {                                  /* LCOV_EXCL_LINE */
          case B_RUN:
            break;
          case B_CONTINUE:
            *brk = B_RUN;
            break;
          case B_BREAK:
            *brk = B_RUN; /* fall through */
          case B_RETURN:
            *pos = end;
            goto cleanup;
        }
      }
      *pos = end;
      break;
    }
    case AST_DEFAULT:
      /* handle fallthroughs */
      ast_move_to_children(a, pos);
      break;
    case AST_CASE:
      /* handle fallthroughs */
      ast_move_to_children(a, pos);
      ast_skip_tree(a, pos);
      break;
    case AST_SWITCH: {
      int found = 0;
      val_t test = v7_create_undefined(), val = v7_create_undefined();
      ast_off_t case_end, default_pos = 0;
      enum ast_tag case_tag;
      tmp_stack_push(&tf, &test);
      tmp_stack_push(&tf, &val);

      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      test = i_eval_expr(v7, a, pos, scope);
      while (*pos < end) {
        switch (case_tag = ast_fetch_tag(a, pos)) {
          default:
            throw_exception(v7, "InternalError", /* LCOV_EXCL_LINE */
                            "invalid ast node %d", case_tag);
          case AST_DEFAULT:
            default_pos = *pos;
            *pos = ast_get_skip(a, *pos, AST_END_SKIP);
            break;
          case AST_CASE:
            case_end = ast_get_skip(a, *pos, AST_END_SKIP);
            ast_move_to_children(a, pos);
            val = i_eval_expr(v7, a, pos, scope);
            /* TODO(mkm): factor out equality check from eval_expr */
            if (test != val || val == V7_TAG_NAN) {
              *pos = case_end;
              break;
            }
            res = i_eval_stmts(v7, a, pos, end, scope, brk);
            if (*brk == B_BREAK) {
              *brk = B_RUN;
            }
            *pos = end;
            found = 1;
            break;
        }
      }

      if (!found && default_pos != 0) {
        ast_move_to_children(a, &default_pos);
        res = i_eval_stmts(v7, a, &default_pos, end, scope, brk);
        if (*brk == B_BREAK) {
          *brk = B_RUN;
        }
      }
      break;
    }
    case AST_LABEL: {
      jmp_buf old_jmp;
      char *name;
      size_t name_len;
      ast_off_t saved_pos;
      size_t saved_tmp_stack_pos = v7->tmp_stack.len;
      volatile enum jmp_type j;
      memcpy(old_jmp, v7->jmp_buf, sizeof(old_jmp));
      name = ast_get_inlined_data(a, *pos, &name_len);

      ast_move_to_children(a, pos);
      saved_pos = *pos;
    /*
     * Percolate up all exceptions and labeled breaks
     * not matching the current label.
     */
    cont:
      if ((j = (enum jmp_type)sigsetjmp(v7->jmp_buf, 0)) == 0) {
        res = i_eval_stmt(v7, a, pos, scope, brk);
      } else if ((j == BREAK_JMP || j == CONTINUE_JMP) &&
                 name_len == v7->label_len &&
                 memcmp(name, v7->label, name_len) == 0) {
        v7->tmp_stack.len = saved_tmp_stack_pos;
        *pos = saved_pos;
        if (j == CONTINUE_JMP) {
          v7->lab_cont = 1;
          goto cont;
        }
        ast_skip_tree(a, pos);
      } else {
        siglongjmp(old_jmp, j);
      }
      memcpy(v7->jmp_buf, old_jmp, sizeof(old_jmp));
      break;
    }
    case AST_TRY: {
      int percolate = 0;
      jmp_buf old_jmp;
      char *name;
      size_t name_len;
      size_t saved_tmp_stack_pos = v7->tmp_stack.len;
      volatile enum jmp_type j;
      memcpy(old_jmp, v7->jmp_buf, sizeof(old_jmp));

      end = ast_get_skip(a, *pos, AST_END_SKIP);
      acatch = ast_get_skip(a, *pos, AST_TRY_CATCH_SKIP);
      finally = ast_get_skip(a, *pos, AST_TRY_FINALLY_SKIP);
      ast_move_to_children(a, pos);
      if ((j = (enum jmp_type)sigsetjmp(v7->jmp_buf, 0)) == 0) {
        res = i_eval_stmts(v7, a, pos, acatch, scope, brk);
      } else if (j == THROW_JMP && acatch != finally) {
        val_t catch_scope;
        v7->tmp_stack.len = saved_tmp_stack_pos;
        catch_scope = create_object(v7, scope);
        tmp_stack_push(&tf, &catch_scope);
        tag = ast_fetch_tag(a, &acatch);
        V7_CHECK(v7, tag == AST_IDENT);
        name = ast_get_inlined_data(a, acatch, &name_len);
        v7_set_property(v7, catch_scope, name, name_len, 0, v7->thrown_error);
        ast_move_to_children(a, &acatch);
        memcpy(v7->jmp_buf, old_jmp, sizeof(old_jmp));
        res = i_eval_stmts(v7, a, &acatch, finally, catch_scope, brk);
      } else {
        percolate = 1;
      }

      memcpy(v7->jmp_buf, old_jmp, sizeof(old_jmp));
      if (finally != end) {
        enum i_break fin_brk = B_RUN;
        res = i_eval_stmts(v7, a, &finally, end, scope, &fin_brk);
        if (fin_brk != B_RUN) {
          *brk = fin_brk;
        }
        if (!*brk && percolate) {
          siglongjmp(v7->jmp_buf, j);
        }
      }
      *pos = end;
      break;
    }
    case AST_WITH: {
      val_t with_scope = v7_create_undefined();
      tmp_stack_push(&tf, &with_scope);
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      /*
       * TODO(mkm) make an actual scope chain. Possibly by mutating
       * the with expression and adding the 'outer environment
       * reference' hidden property.
       */
      with_scope = i_eval_expr(v7, a, pos, scope);
      if (!v7_is_object(with_scope)) {
        throw_exception(v7, "InternalError",
                        "with statement is not really implemented yet");
      }
      i_eval_stmts(v7, a, pos, end, with_scope, brk);
      break;
    }
    case AST_VALUE_RETURN:
      res = i_eval_expr(v7, a, pos, scope);
      *brk = B_RETURN;
      break;
    case AST_RETURN:
      *brk = B_RETURN;
      break;
    case AST_BREAK:
      *brk = B_BREAK;
      break;
    case AST_CONTINUE:
      *brk = B_CONTINUE;
      break;
    case AST_LABELED_BREAK:
      V7_CHECK(v7, ast_fetch_tag(a, pos) == AST_IDENT);
      v7->label = ast_get_inlined_data(a, *pos, &v7->label_len);
      siglongjmp(v7->jmp_buf, BREAK_JMP);
      break; /* unreachable */
    case AST_LABELED_CONTINUE:
      V7_CHECK(v7, ast_fetch_tag(a, pos) == AST_IDENT);
      v7->label = ast_get_inlined_data(a, *pos, &v7->label_len);
      siglongjmp(v7->jmp_buf, CONTINUE_JMP);
      break; /* unreachable */
    case AST_THROW:
      v7->thrown_error = i_eval_expr(v7, a, pos, scope);
      siglongjmp(v7->jmp_buf, THROW_JMP);
      break; /* unreachable */
    default:
      (*pos)--;
      res = i_eval_expr(v7, a, pos, scope);
      break;
  }

cleanup:
  tmp_frame_cleanup(&tf);
  return res;
}

/* Invoke a function applying the argument array */
val_t v7_apply(struct v7 *v7, val_t f, val_t this_object, val_t args) {
  struct v7_function *func;
  ast_off_t pos, body, end;
  enum ast_tag tag;
  val_t frame = v7_create_undefined(), res = v7_create_undefined();
  val_t arguments = v7_create_undefined(), saved_this = v7->this_object;
  char *name;
  size_t name_len;
  char buf[20];
  int i, n;

  struct gc_tmp_frame vf = new_tmp_frame(v7);
  tmp_stack_push(&vf, &frame);
  tmp_stack_push(&vf, &res);
  tmp_stack_push(&vf, &arguments);
  tmp_stack_push(&vf, &saved_this);
  /*
   * Since v7_apply can be called from user code
   * we have to treat all arguments as roots.
   */
  tmp_stack_push(&vf, &args);
  tmp_stack_push(&vf, &f);
  tmp_stack_push(&vf, &this_object);

  if (v7_is_cfunction(f)) {
    res = v7_to_cfunction(f)(v7, this_object, args);
    goto cleanup;
  }
  if (!v7_is_function(f)) {
    throw_exception(v7, "TypeError", "value is not a function");
  }
  func = v7_to_function(f);
  frame = i_prepare_call(v7, func, &pos, &body, &end);

  /*
   * TODO(mkm): don't create arguments array if the parser didn't see
   * any `arguments` or `eval` identifier being referenced in the function.
   */
  arguments = v7_create_array(v7);

  for (i = 0; pos < body; i++) {
    tag = ast_fetch_tag(func->ast, &pos);
    V7_CHECK(v7, tag == AST_IDENT);
    name = ast_get_inlined_data(func->ast, pos, &name_len);
    ast_move_to_children(func->ast, &pos);
    res = v7_array_get(v7, args, i);
    v7_set_property(v7, frame, name, name_len, 0, res);
    if (!v7_is_undefined(arguments)) {
      n = snprintf(buf, sizeof(buf), "%d", i);
      v7_set_property(v7, arguments, buf, n, 0, res);
    }
  }

  if (!v7_is_undefined(arguments)) {
#ifndef V7_DISABLE_PREDEFINED_STRINGS
    v7_set_v(v7, frame, v7->predefined_strings[PREDEFINED_STR_ARGUMENTS],
             arguments);
#else
    v7_set(v7, frame, "arguments", 9, arguments);
#endif
  }

  v7->this_object = this_object;
  res = i_invoke_function(v7, func, frame, body, end);
  v7->this_object = saved_this;

cleanup:
  tmp_frame_cleanup(&vf);
  return res;
}

enum v7_err v7_exec_with(struct v7 *v7, val_t *res, const char *src, val_t w) {
  /* TODO(mkm): use GC pool */
  struct ast *a = (struct ast *)malloc(sizeof(struct ast));
  val_t old_this = v7->this_object;
  enum i_break brk = B_RUN;
  ast_off_t pos = 0;
  jmp_buf saved_jmp_buf, saved_label_buf;
  size_t saved_tmp_stack_pos = v7->tmp_stack.len;
  enum v7_err err = V7_OK;
  val_t r = v7_create_undefined();

  /* Make v7_exec() reentrant: save exception environments */
  memcpy(&saved_jmp_buf, &v7->jmp_buf, sizeof(saved_jmp_buf));
  memcpy(&saved_label_buf, &v7->label_jmp_buf, sizeof(saved_label_buf));

  ast_init(a, 0);
  mbuf_append(&v7->allocated_asts, (char *)&a, sizeof(a));
  if (sigsetjmp(v7->jmp_buf, 0) != 0) {
    v7->tmp_stack.len = saved_tmp_stack_pos;
    r = v7->thrown_error;
    err = V7_EXEC_EXCEPTION;
    goto cleanup;
  }
  if (parse(v7, a, src, 1) != V7_OK) {
    v7_exec_with(v7, &r, "new SyntaxError(this)",
                 v7_create_string(v7, v7->error_msg, strlen(v7->error_msg), 1));
    err = V7_SYNTAX_ERROR;
    goto cleanup;
  }
  ast_optimize(a);

  v7->this_object = v7_is_undefined(w) ? v7->global_object : w;
  r = i_eval_stmt(v7, a, &pos, v7->global_object, &brk);

cleanup:
  if (res != NULL) {
    *res = r;
  }
  v7->this_object = old_this;
  memcpy(&v7->jmp_buf, &saved_jmp_buf, sizeof(saved_jmp_buf));
  memcpy(&v7->label_jmp_buf, &saved_label_buf, sizeof(saved_label_buf));

  return err;
}

enum v7_err v7_exec(struct v7 *v7, val_t *res, const char *src) {
  return v7_exec_with(v7, res, src, V7_UNDEFINED);
}

enum v7_err v7_exec_file(struct v7 *v7, val_t *res, const char *path) {
  FILE *fp;
  char *p;
  long file_size;
  enum v7_err err = V7_EXEC_EXCEPTION;
  *res = v7_create_undefined();

  if ((fp = fopen(path, "r")) == NULL) {
    snprintf(v7->error_msg, sizeof(v7->error_msg), "cannot open file [%s]",
             path);
    *res = create_exception(v7, "Error", v7->error_msg);
  } else if (fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0) {
    snprintf(v7->error_msg, sizeof(v7->error_msg), "fseek(%s): %s", path,
             strerror(errno));
    *res = create_exception(v7, "Error", v7->error_msg);
    fclose(fp);
  } else if ((p = (char *)calloc(1, (size_t)file_size + 1)) == NULL) {
    snprintf(v7->error_msg, sizeof(v7->error_msg), "cannot allocate %ld bytes",
             file_size + 1);
    fclose(fp);
  } else {
    rewind(fp);
    fread(p, 1, (size_t)file_size, fp);
    fclose(fp);
    err = v7_exec(v7, res, p);
    free(p);
  }

  return err;
}
