---
title: "v7_get_own_prop_desc_cb_t"
decl_name: "v7_get_own_prop_desc_cb_t"
symbol_kind: "typedef"
signature: |
  typedef int(v7_get_own_prop_desc_cb_t)(struct v7 *v7, v7_val_t name,
                                         struct v7_property *res_prop);
---

C callback, analogue of JS callback `getOwnPropertyDescriptor()`.
Callbacks of this type are used for C API only, see `m7_mk_proxy()`.

`name` is the name of the property, and the function should fill
`res_prop` with the property data; namely, fields `attributes` and `value`.
Other fields are ignored. The structure is zeroed out before the callback
is called.

It should return non-zero if the property should be considered existing, or
zero otherwise. 

