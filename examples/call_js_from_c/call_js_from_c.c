/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#include <stdio.h>
#include "v7.h"

static void call_sum(struct v7 *v7) {
  v7_val_t func, result, args;

  func = v7_get(v7, v7_get_global_object(v7), "sum", 3);

  args = v7_create_array(v7);
  v7_array_push(v7, args, v7_create_number(123.0));
  v7_array_push(v7, args, v7_create_number(456.789));

  v7_apply(v7, &result, func, v7_create_undefined(), args);
  printf("Result: %g\n", v7_to_number(result));
}

int main(void) {
  v7_val_t result;
  struct v7 *v7 = v7_create();
  v7_exec(v7, "var sum = function(a, b) { return a + b; };", &result);
  call_sum(v7);
  v7_destroy(v7);
  return 0;
}
