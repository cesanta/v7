---
title: v7_def()
decl_name: v7_def
symbol_kind: func
signature: |
  int v7_def(struct v7 *v7, v7_val_t obj, const char *name, size_t name_len,
             v7_prop_attr_desc_t attrs_desc, v7_val_t v);
---

Define object property, similar to JavaScript `Object.defineProperty()`.

`name`, `name_len` specify property name, `val` is a property value.
`attrs_desc` is a set of flags which can affect property's attributes,
see comment of `v7_prop_attr_desc_t` for details.

If `name_len` is ~0, `name` is assumed to be NUL-terminated and
`strlen(name)` is used.

Returns non-zero on success, 0 on error (e.g. out of memory).

See also `v7_set()`. 

