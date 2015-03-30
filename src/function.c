/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t Function_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  long i, num_args = v7_array_length(v7, args);
  size_t size;
  const char *s;
  struct mbuf m;
  val_t tmp;
  enum v7_err ret;

  (void) this_obj;

  if (num_args <= 0) return v7_create_undefined();

  mbuf_init(&m, 0);
  mbuf_append(&m, "(function(", 10);

  for (i = 0; i < num_args - 1; i++) {
    tmp = i_value_of(v7, v7_array_get(v7, args, i));
    if (v7_is_string(tmp)) {
      if (i > 0) mbuf_append(&m, ",", 1);
      s = v7_to_string(v7, &tmp, &size);
      mbuf_append(&m, s, size);
    }
  }
  mbuf_append(&m, "){", 2);
  tmp = i_value_of(v7, v7_array_get(v7, args, num_args - 1));
  if (v7_is_string(tmp)) {
    s = v7_to_string(v7, &tmp, &size);
    mbuf_append(&m, s, size);
  }
  mbuf_append(&m, "})\0", 3);

  ret = v7_exec(v7, &tmp, m.buf);
  mbuf_free(&m);
  if (ret != V7_OK) {
    throw_exception(v7, SYNTAX_ERROR, "Invalid function body");
  }
  return tmp;
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
