---
title: "v7_get_user_data()"
decl_name: "v7_get_user_data"
symbol_kind: "func"
signature: |
  void *v7_get_user_data(struct v7 *v7, v7_val_t obj);
---

Get the opaque user data set with `v7_set_user_data`.

Returns NULL if there is no user data set or if `obj` is not an object. 

