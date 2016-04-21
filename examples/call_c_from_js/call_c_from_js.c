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

static enum v7_err js_sum(struct v7 *v7, v7_val_t *res) {
  double arg0 = v7_to_number(v7_arg(v7, 0));
  double arg1 = v7_to_number(v7_arg(v7, 1));
  double result = sum(arg0, arg1);

  *res = v7_mk_number(result);
  return V7_OK;
}

int v7_example(void) {
  enum v7_err rcode = V7_OK;
  struct v7 *v7 = v7_create();
  v7_val_t result;
  v7_set_method(v7, v7_get_global(v7), "sum", &js_sum);

  rcode = v7_exec(v7, "print('sum = ' + sum(1.2, 3.4))", &result);
  if (rcode != V7_OK) {
    v7_print_error(stderr, v7, "Evaluation error", result);
  }

  v7_destroy(v7);
  return (int) rcode;
}
