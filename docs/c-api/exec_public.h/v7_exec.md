---
title: v7_exec()
decl_name: v7_exec
symbol_kind: func
signature: |
  enum v7_err v7_exec(struct v7 *v7, const char *js_code, v7_val_t *result);
---

Execute JavaScript `js_code`. The result of evaluation is stored in
the `result` variable.

Return:

 - V7_OK on success. `result` contains the result of execution.
 - V7_SYNTAX_ERROR if `js_code` in not a valid code. `result` is undefined.
 - V7_EXEC_EXCEPTION if `js_code` threw an exception. `result` stores
   an exception object.
 - V7_AST_TOO_LARGE if `js_code` contains an AST segment longer than 16 bit.
   `result` is undefined. To avoid this error, build V7 with V7_LARGE_AST. 

