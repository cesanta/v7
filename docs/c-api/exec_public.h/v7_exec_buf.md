---
title: "v7_exec_buf()"
decl_name: "v7_exec_buf"
symbol_kind: "func"
signature: |
  enum v7_err v7_exec_buf(struct v7 *v7, const char *js_code, size_t len,
                          v7_val_t *result);
---

Like v7_exec but it expects an explicit length instead of treating the code
as a null terminated string.

The code can be either a JS source or a precompiled bytecode. 

