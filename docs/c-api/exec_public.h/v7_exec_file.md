---
title: v7 exec file()
signature: |
  enum v7_err v7_exec_file(struct v7 *v7, const char *path, v7_val_t *result);
---

Same as `v7_exec()`, but loads source code from `path` file. 

