---
title: v7 get()
signature: |
  v7_val_t v7_get(struct v7 *v7, v7_val_t obj, const char *name, size_t name_len);
---

Lookup property `name` in object `obj`. If `obj` holds no such property,
an `undefined` value is returned.

If `name_len` is ~0, `name` is assumed to be NUL-terminated and
`strlen(name)` is used. 

