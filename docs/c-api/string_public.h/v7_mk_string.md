---
title: v7_mk_string()
decl_name: v7_mk_string
symbol_kind: func
signature: |
  v7_val_t v7_mk_string(struct v7 *v7, const char *str, size_t len, int copy);
---

Creates a string primitive value.
`str` must point to the utf8 string of length `len`.
If `len` is ~0, `str` is assumed to be NUL-terminated and `strlen(str)` is
used.

If `copy` is non-zero, the string data is copied and owned by the GC. The
caller can free the string data afterwards. Otherwise (`copy` is zero), the
caller owns the string data, and is responsible for not freeing it while it
is used. 

