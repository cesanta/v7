---
title: "v7_get_ptr()"
decl_name: "v7_get_ptr"
symbol_kind: "func"
signature: |
  void *v7_get_ptr(v7_val_t v);
---

Returns `void *` pointer stored in `v7_val_t`.

Returns NULL if the value is not a foreign pointer. 

