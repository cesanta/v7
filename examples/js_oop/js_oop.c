/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#include <stdio.h>
#include <string.h>
#include "v7.h"

/*
 * This example demonstrates how to do JS OOP in C.
 */

static enum v7_err MyThing_ctor(struct v7 *v7, v7_val_t *res) {
  v7_val_t this_obj = v7_get_this(v7);
  v7_val_t arg0 = v7_arg(v7, 0);
  v7_def(v7, this_obj, "__arg", ~0 /* = strlen */, V7_DESC_ENUMERABLE(0), arg0);

  /*
   * A constructor function can access the newly created object with
   * `v7_get_this()`. The constructor can override the result of the `new`
   * expression by explicitly setting `res` to another object. Any non-object
   * value set into `res` will be ignored. This matches the JavaScript
   * constructor return value semantics.
   */
  return V7_OK;
}

static enum v7_err MyThing_myMethod(struct v7 *v7, v7_val_t *res) {
  v7_val_t this_obj = v7_get_this(v7);
  *res = v7_get(v7, this_obj, "__arg", ~0);
  return V7_OK;
}

int main(void) {
  enum v7_err rcode = V7_OK;
  struct v7 *v7 = v7_create();
  v7_val_t ctor_func, proto, eval_result;

  proto = v7_mk_object(v7);
  ctor_func = v7_mk_constructor(v7, proto, MyThing_ctor);
  v7_def(v7, ctor_func, "MY_CONST", ~0,
      (V7_DESC_WRITABLE(0) | V7_DESC_CONFIGURABLE(0)),
      v7_mk_number(123));
  v7_set_method(v7, proto, "myMethod", &MyThing_myMethod);
  v7_set(v7, v7_get_global(v7), "MyThing", ~0, ctor_func);

  rcode = v7_exec(v7,
          "\
      print('MyThing.MY_CONST = ', MyThing.MY_CONST); \
      var t = new MyThing(456); \
      print('t.MY_CONST = ', t.MY_CONST); \
      print('t.myMethod = ', t.myMethod); \
      print('t.myMethod() = ', t.myMethod());",
          &eval_result);
  if (rcode != V7_OK) {
    fprintf(stderr, "exec error: %d\n", (int)rcode);
  }

  v7_destroy(v7);
  return (int)rcode;
}
