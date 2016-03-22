---
title: "v7_array_set_throwing()"
decl_name: "v7_array_set_throwing"
symbol_kind: "func"
signature: |
  enum v7_err v7_array_set_throwing(struct v7 *v7, v7_val_t arr,
                                    unsigned long index, v7_val_t v, int *res);
---

Like `v7_array_set()`, but "returns" value through the `res` pointer
argument. `res` is allowed to be `NULL`.

Caller should check the error code returned, and if it's something other
than `V7_OK`, perform cleanup and return this code further. 

