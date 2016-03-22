---
title: "v7_mk_regexp()"
decl_name: "v7_mk_regexp"
symbol_kind: "func"
signature: |
  enum v7_err v7_mk_regexp(struct v7 *v7, const char *regex, size_t regex_len,
                           const char *flags, size_t flags_len, v7_val_t *res);
---

Make RegExp object.
`regex`, `regex_len` specify a pattern, `flags` and `flags_len` specify
flags. Both utf8 encoded. For example, `regex` is `(.+)`, `flags` is `gi`.
If `regex_len` is ~0, `regex` is assumed to be NUL-terminated and
`strlen(regex)` is used. 

