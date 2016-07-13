---
title: "v7_get_own_prop_desc_cb_t"
decl_name: "v7_get_own_prop_desc_cb_t"
symbol_kind: "typedef"
signature: |
  typedef int(v7_get_own_prop_desc_cb_t)(struct v7 *v7, v7_val_t name,
                                         v7_prop_attr_t *attrs, v7_val_t *value);
---

C callback, analogue of JS callback `getOwnPropertyDescriptor()`.
Callbacks of this type are used for C API only, see `m7_mk_proxy()`.

`name` is the name of the property, and the function should fill `attrs` and
`value` with the property data. Before this callback is called, `attrs` is
set to 0, and `value` is `V7_UNDEFINED`.

It should return non-zero if the property should be considered existing, or
zero otherwise. 

