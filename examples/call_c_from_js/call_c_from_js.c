/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#include <stdio.h>
#include <string.h>
#include "v7.h"

static double sum(double a, double b) {
  return a + b;
}

static v7_val_t js_sum(struct v7 *v7, v7_val_t this_obj, v7_val_t args) {
  double arg0 = v7_to_number(v7_array_get(v7, args, 0));
  double arg1 = v7_to_number(v7_array_get(v7, args, 1));
  double result = sum(arg0, arg1);
  (void) this_obj;  /* unused */
  return v7_create_number(result);
}

int main(void) {
  struct v7 *v7 = v7_create();
  v7_val_t result;
  v7_set_method(v7, v7_get_global_object(v7), "sum", &js_sum);
  v7_exec(v7, "print(sum(1.2, 3.4))", &result);
  v7_destroy(v7);
  return 0;
}
