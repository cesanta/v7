---
title: v7_get_throwing()
decl_name: v7_get_throwing
symbol_kind: func
signature: |
  enum v7_err v7_get_throwing(struct v7 *v7, v7_val_t obj, const char *name,
                              size_t name_len, v7_val_t *res);
---

Like `v7_get()`, but "returns" value through `res` pointer argument.
`res` must not be `NULL`.

Caller should check the error code returned, and if it's something other
than `V7_OK`, perform cleanup and return this code further. 

