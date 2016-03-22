---
title: "v7_exec_with()"
decl_name: "v7_exec_with"
symbol_kind: "func"
signature: |
  enum v7_err v7_exec_with(struct v7 *v7, const char *js_code, v7_val_t this_obj,
                           v7_val_t *result);
---

Same as `v7_exec()`, but passes `this_obj` as `this` to the execution
context. 

