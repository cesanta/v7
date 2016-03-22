---
title: "v7_get_thrown_value()"
decl_name: "v7_get_thrown_value"
symbol_kind: "func"
signature: |
  v7_val_t v7_get_thrown_value(struct v7 *v7, unsigned char *is_thrown);
---

Returns the value that is being thrown at the moment, or `undefined` if
nothing is being thrown. If `is_thrown` is not `NULL`, it will be set
to either 0 or 1, depending on whether something is thrown at the moment. 

