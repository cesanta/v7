/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#include <stdio.h>
#include "v7.h"

static void call_sum(struct v7 *v7) {
  v7_val_t func, result, args;

  func = v7_get(v7, v7_get_global(v7), "sum", 3);

  args = v7_mk_array(v7);
  v7_array_push(v7, args, v7_mk_number(123.0));
  v7_array_push(v7, args, v7_mk_number(456.789));

  if (v7_apply(v7, func, v7_mk_undefined(), args, &result) == V7_OK) {
    printf("Result: %g\n", v7_to_number(result));
  } else {
    v7_print_error(stderr, v7, "Error while calling sum", result);
  }
}

int main(void) {
  enum v7_err rcode = V7_OK;
  v7_val_t result;
  struct v7 *v7 = v7_create();
  rcode = v7_exec(v7, "var sum = function(a, b) { return a + b; };", &result);
  if (rcode != V7_OK) {
    v7_print_error(stderr, v7, "Evaluation error", result);
    goto clean;
  }

  call_sum(v7);

clean:
  v7_destroy(v7);
  return (int) rcode;
}
