---
title: "v7_apply()"
decl_name: "v7_apply"
symbol_kind: "func"
signature: |
  enum v7_err v7_apply(struct v7 *v7, v7_val_t func, v7_val_t this_obj,
                       v7_val_t args, v7_val_t *res);
---

Call function `func` with arguments `args`, using `this_obj` as `this`.
`args` should be an array containing arguments or `undefined`.

`res` can be `NULL` if return value is not required. 

