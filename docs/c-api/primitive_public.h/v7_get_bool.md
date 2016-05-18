---
title: "v7_get_bool()"
decl_name: "v7_get_bool"
symbol_kind: "func"
signature: |
  NOINSTR int v7_get_bool(struct v7 *v7, v7_val_t v);
---

Returns boolean stored in `v7_val_t`:
 0 for `false` or non-boolean, non-0 for `true` 

