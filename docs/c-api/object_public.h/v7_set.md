---
title: "v7_set()"
decl_name: "v7_set"
symbol_kind: "func"
signature: |
  int v7_set(struct v7 *v7, v7_val_t obj, const char *name, size_t len,
             v7_val_t val);
---

Set object property. Behaves just like JavaScript assignment.

See also `v7_def()`. 

