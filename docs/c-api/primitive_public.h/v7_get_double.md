---
title: "v7_get_double()"
decl_name: "v7_get_double"
symbol_kind: "func"
signature: |
  NOINSTR double v7_get_double(struct v7 *v7, v7_val_t v);
---

Returns number value stored in `v7_val_t` as `double`.

Returns NaN for non-numbers. 

