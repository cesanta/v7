---
title: v7_exec_file()
decl_name: v7_exec_file
symbol_kind: func
signature: |
  enum v7_err v7_exec_file(struct v7 *v7, const char *path, v7_val_t *result);
---

Same as `v7_exec()`, but loads source code from `path` file. 

