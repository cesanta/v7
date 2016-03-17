---
title: v7_set_method()
decl_name: v7_set_method
symbol_kind: func
signature: |
  int v7_set_method(struct v7 *, v7_val_t obj, const char *name,
                    v7_cfunction_t *func);
---

A helper function to define object's method backed by a C function `func`.
`name` must be NUL-terminated.

Return value is the same as for `v7_set()`. 

