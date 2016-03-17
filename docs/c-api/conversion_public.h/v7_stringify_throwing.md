---
title: v7_stringify_throwing()
decl_name: v7_stringify_throwing
symbol_kind: func
signature: |
  enum v7_err v7_stringify_throwing(struct v7 *v7, v7_val_t v, char *buf,
                                    size_t size, enum v7_stringify_mode mode,
                                    char **res);
---

Like `v7_stringify()`, but "returns" value through the `res` pointer
argument. `res` must not be `NULL`.

Caller should check the error code returned, and if it's something other
than `V7_OK`, perform cleanup and return this code further. 

