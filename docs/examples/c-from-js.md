---
title: Call C/C++ function from JavaScript
---

This example demonstrates how to export existing C/C++ functions to
the JavaScript environment. Here, the C function `sum()` that
calculates a sum of two numbers gets exported to JavaScript. The C application
`my_app.c` executes a JavaScript file `my_js_code.js` which
calls `sum()` that invokes the C function `sum()`.

```c

#include <stdio.h>
#include <string.h>
#include "v7.h"

/*
 * a C function which we want to export to JavaScript
 */
static double sum(double a, double b) {
  return a + b;
}

/*
 * a C function that glues C function `sum()` and JavaScript function `sum()`
 * together
 */
static enum v7_err js_sum(struct v7 *v7, v7_val_t *res) {

  /*
   * When JavaScript function `sum()` is called, V7 creates an arguments array 
   * and then calls the `js_sum()` glue function. Here we extract argument values  
   * from the args array via `v7_get_arguments()`.
   */
  v7_val_t args = v7_get_arguments(v7);	
  double arg0 = v7_get_double(v7, v7_array_get(v7, args, 0));
  double arg1 = v7_get_double(v7, v7_array_get(v7, args, 1));

  /* Call C function `sum()` */
  *res = v7_mk_number(v7, sum(arg0, arg1));

  /* Return without errors */
  return V7_OK;
}

int main(void) {
  enum v7_err rcode = V7_OK;
  struct v7 *v7 = v7_create();
  v7_val_t result;

  /*
   * Export function `js_sum()` to JavaScript under the name `sum`. From that
   * point on, `v7` instance has global function `sum` defined.
   */
  v7_set_method(v7, v7_get_global(v7), "sum", &js_sum);

  /* Execute JavaScript code that calls `sum()` */
  rcode = v7_exec(v7, "print(sum(1.2, 3.4))", &result);
  if (rcode != V7_OK) {
    fprintf(stderr, "exec error: %d\n", (int)rcode);
  }
  v7_destroy(v7);
  return (int)rcode;
}
```
