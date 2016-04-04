---
title: "v7_get_string_data()"
decl_name: "v7_get_string_data"
symbol_kind: "func"
signature: |
  const char *v7_get_string_data(struct v7 *v7, v7_val_t *v, size_t *len);
---

Returns a pointer to the string stored in `v7_val_t`.

String length returned in `len`. Returns NULL if the value is not a string.

JS strings can contain embedded NUL chars and may or may not be NUL
terminated.

CAUTION: creating new JavaScript object, array, or string may kick in a
garbage collector, which in turn may relocate string data and invalidate
pointer returned by `v7_get_string_data()`.

Short JS strings are embedded inside the `v7_val_t` value itself. This is why
a pointer to a `v7_val_t` is required. It also means that the string data
will become invalid once that `v7_val_t` value goes out of scope. 

