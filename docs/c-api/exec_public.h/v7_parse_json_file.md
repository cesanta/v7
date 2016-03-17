---
title: v7_parse_json_file()
decl_name: v7_parse_json_file
symbol_kind: func
signature: |
  enum v7_err v7_parse_json_file(struct v7 *v7, const char *path, v7_val_t *res);
---

Same as `v7_parse_json()`, but loads JSON string from `path`. 

