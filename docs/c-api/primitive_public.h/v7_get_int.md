---
title: "v7_get_int()"
decl_name: "v7_get_int"
symbol_kind: "func"
signature: |
  NOINSTR int v7_get_int(struct v7 *v7, v7_val_t v);
---

Returns number value stored in `v7_val_t` as `int`. If the number value is
not an integer, the fraction part will be discarded.

If the given value is a non-number, or NaN, the result is undefined. 

