---
title: Call JavaScript function from C/C++
---

This is the reverse example. Now we have a JavaScript function `sum()` that calculates sum of two numbers. Here is how we call that function from C/C++:

```c
#include <stdio.h>
#include "v7.h"

static void call_sum(struct v7 *v7) {
  v7_val_t func, result, args;

  /* Lookup JavaScript function `sum()` */
  func = v7_get(v7, v7_get_global(v7), "sum", 3);

  /* Create arguments array with two numbers */
  args = v7_mk_array(v7);
  v7_array_push(v7, args, v7_mk_number(123.0));
  v7_array_push(v7, args, v7_mk_number(456.789));

  /* Call JavaScript function. Pass `undefined` as `this` */
  result = v7_apply(v7, func, v7_mk_undefined(), args);
  printf("Result: %g\n", v7_to_number(result));
}

int main(void) {
  enum v7_err rcode = V7_OK;
  v7_val_t result;
  struct v7 *v7 = v7_create();

  /* Define JavaScript function `sum()` */
  rcode = v7_exec(v7, "var sum = function(a, b) { return a + b; };", &result);
  if (rcode != V7_OK) {
    fprintf(stderr, "exec error: %d\n", (int)rcode);
  } else {
    call_sum(v7);
  }

  v7_destroy(v7);
  return (int)rcode;
}
```
