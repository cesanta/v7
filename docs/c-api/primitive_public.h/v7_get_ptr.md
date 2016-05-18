---
title: "v7_get_ptr()"
decl_name: "v7_get_ptr"
symbol_kind: "func"
signature: |
  NOINSTR void *v7_get_ptr(struct v7 *v7, v7_val_t v);
---

Returns `void *` pointer stored in `v7_val_t`.

Returns NULL if the value is not a foreign pointer. 

