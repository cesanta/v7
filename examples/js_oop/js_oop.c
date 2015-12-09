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

static v7_val_t MyThing_ctor(struct v7 *v7) {
  v7_val_t this_obj = v7_get_this(v7);
  v7_val_t arg0 = v7_arg(v7, 0);
  v7_set(v7, this_obj, "__arg", ~0 /* = strlen */, V7_PROPERTY_DONT_ENUM, arg0);
  return this_obj;
}

static v7_val_t MyThing_myMethod(struct v7 *v7) {
  v7_val_t this_obj = v7_get_this(v7);
  return v7_get(v7, this_obj, "__arg", ~0);
}

int main(void) {
  struct v7 *v7 = v7_create();
  v7_val_t ctor_func, proto, eval_result;

  proto = v7_create_object(v7);
  ctor_func = v7_create_constructor_nargs(v7, proto, MyThing_ctor, 1);
  v7_set(v7, ctor_func, "MY_CONST", ~0,
         V7_PROPERTY_READ_ONLY | V7_PROPERTY_DONT_DELETE,
         v7_create_number(123));
  v7_set_method(v7, proto, "myMethod", &MyThing_myMethod);
  v7_set(v7, v7_get_global(v7), "MyThing", ~0, 0, ctor_func);

  v7_exec(v7,
          "\
      print('MyThing.MY_CONST = ', MyThing.MY_CONST); \
      var t = new MyThing(456); \
      print('t.MY_CONST = ', t.MY_CONST); \
      print('t.myMethod = ', t.myMethod); \
      print('t.myMethod() = ', t.myMethod());",
          &eval_result);
  v7_destroy(v7);
  return 0;
}
