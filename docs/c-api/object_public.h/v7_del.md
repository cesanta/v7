---
title: v7_del()
decl_name: v7_del
symbol_kind: func
signature: |
  int v7_del(struct v7 *v7, v7_val_t obj, const char *name, size_t name_len);
---

Delete own property `name` of the object `obj`. Does not follow the
prototype chain.

If `name_len` is ~0, `name` is assumed to be NUL-terminated and
`strlen(name)` is used.

Returns 0 on success, -1 on error. 

