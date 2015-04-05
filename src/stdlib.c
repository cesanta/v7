/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

V7_PRIVATE v7_val_t Std_print(struct v7 *v7, v7_val_t this_obj, v7_val_t args) {
  char *p, buf[1024];
  int i, num_args = v7_array_length(v7, args);

  (void) this_obj;
  for (i = 0; i < num_args; i++) {
    v7_val_t arg = v7_array_get(v7, args, i);
    if (v7_is_string(arg)) {
      size_t n;
      const char *s = v7_to_string(v7, &arg, &n);
      printf("%s", s);
    } else {
      p = v7_to_json(v7, arg, buf, sizeof(buf));
      printf("%s", p);
      if (p != buf) {
        free(p);
      }
    }
  }
  putchar('\n');

  return v7_create_null();
}

V7_PRIVATE v7_val_t Std_eval(struct v7 *v7, v7_val_t t, v7_val_t args) {
  v7_val_t res = v7_create_undefined(), arg = v7_array_get(v7, args, 0);
  (void) t;
  if (arg != V7_UNDEFINED) {
    char buf[100], *p;
    p = v7_to_json(v7, arg, buf, sizeof(buf));
    if (p[0] == '"') {
      p[0] = p[strlen(p) - 1] = ' ';
    }
    if (v7_exec(v7, &res, p) != V7_OK) {
      throw_value(v7, res);
    }
    if (p != buf) {
      free(p);
    }
  }
  return res;
}

static v7_val_t Std_exit(struct v7 *v7, v7_val_t t, v7_val_t args) {
  int exit_code = arg_long(v7, args, 0, 0);
  (void) t;
  exit(exit_code);
  return v7_create_undefined();
}

V7_PRIVATE void init_stdlib(struct v7 *v7) {
  /*
   * Ensure the first call to v7_create_value will use a null proto:
   * {}.__proto__.__proto__ == null
   */
  v7->object_prototype = create_object(v7, V7_NULL);
  v7->array_prototype = v7_create_object(v7);
  v7->boolean_prototype = v7_create_object(v7);
  v7->string_prototype = v7_create_object(v7);
  v7->regexp_prototype = v7_create_object(v7);
  v7->number_prototype = v7_create_object(v7);
  v7->error_prototype = v7_create_object(v7);
  v7->global_object = v7_create_object(v7);
  v7->this_object = v7->global_object;
  v7->date_prototype = v7_create_object(v7);
  v7->function_prototype = v7_create_object(v7);

  set_cfunc_prop(v7, v7->global_object, "print", Std_print);
  set_cfunc_prop(v7, v7->global_object, "eval", Std_eval);
  set_cfunc_prop(v7, v7->global_object, "exit", Std_exit);

  v7_set_property(v7, v7->global_object, "Infinity", 8, 0,
                  v7_create_number(INFINITY));
  v7_set_property(v7, v7->global_object, "global", 6, 0, v7->global_object);

  init_object(v7);
  init_array(v7);
  init_error(v7);
  init_boolean(v7);
#if V7_ENABLE__Math
  init_math(v7);
#endif
  init_string(v7);
#ifndef V7_DISABLE_REGEX
  init_regex(v7);
#endif
  init_number(v7);
  init_json(v7);
  init_date(v7);
  init_function(v7);
  init_js_stdlib(v7);
}
