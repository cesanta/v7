/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t Function_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  long i, n = 0, num_args = v7_array_length(v7, args);
  size_t size;
  char buf[200];
  const char *s;
  val_t param, body, res = v7_create_undefined();

  (void) this_obj;

  if (num_args <= 0) return res;

  /* TODO(lsm): Constructing function source code here. Optimize this. */
  n += snprintf(buf + n, sizeof(buf) - n, "%s", "(function(");

  for (i = 0; i < num_args - 1; i++) {
    param = i_value_of(v7, v7_array_get(v7, args, i));
    if (v7_is_string(param)) {
      s = v7_to_string(v7, &param, &size);
      if (i > 0) {
        n += snprintf(buf + n, sizeof(buf) - n, "%s", ",");
      }
      n += snprintf(buf + n, sizeof(buf) - n, "%.*s", (int) size, s);
    }
  }
  n += snprintf(buf + n, sizeof(buf) - n, "%s", "){");
  body = i_value_of(v7, v7_array_get(v7, args, num_args - 1));
  if (v7_is_string(body)) {
    s = v7_to_string(v7, &body, &size);
    n += snprintf(buf + n, sizeof(buf) - n, "%.*s", (int) size, s);
  }
  n += snprintf(buf + n, sizeof(buf) - n, "%s", "})");

  if (v7_exec_with(v7, &res, buf, V7_UNDEFINED) != V7_OK) {
    throw_exception(v7, "SyntaxError", "Invalid function body");
  }

  return res;
}

static val_t Function_length(struct v7 *v7, val_t this_obj, val_t args) {
  struct v7_function *func = v7_to_function(this_obj);
  ast_off_t body, pos = func->ast_off;
  struct ast *a = func->ast;
  int argn = 0;

  (void) args;

  V7_CHECK(v7, ast_fetch_tag(a, &pos) == AST_FUNC);
  body = ast_get_skip(a, pos, AST_FUNC_BODY_SKIP);

  ast_move_to_children(a, &pos);
  if (ast_fetch_tag(a, &pos) == AST_IDENT) {
    ast_move_to_children(a, &pos);
  }
  while (pos < body) {
    V7_CHECK(v7, ast_fetch_tag(a, &pos) == AST_IDENT);
    ast_move_to_children(a, &pos);
    argn++;
  }

  return v7_create_number(argn);
}

static val_t Function_apply(struct v7 *v7, val_t this_obj, val_t args) {
  val_t f = i_value_of(v7, this_obj);
  val_t this_arg = v7_array_get(v7, args, 0);
  val_t func_args = v7_array_get(v7, args, 1);
  return v7_apply(v7, f, this_arg, func_args);
}

V7_PRIVATE void init_function(struct v7 *v7) {
  val_t ctor = v7_create_cfunction_object(v7, Function_ctor, 1);
  v7_set_property(v7, ctor, "prototype", 9, 0, v7->function_prototype);
  v7_set_property(v7, v7->global_object, "Function", 8, 0, ctor);
  set_cfunc_obj_prop(v7, v7->function_prototype, "apply", Function_apply, 1);
  v7_set_property(v7, v7->function_prototype, "length", 6, V7_PROPERTY_GETTER,
                  v7_create_cfunction(Function_length));
}
