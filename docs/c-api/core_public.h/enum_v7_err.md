---
title: "enum v7_err"
decl_name: "enum v7_err"
symbol_kind: "enum"
signature: |
  enum v7_err {
    V7_OK,
    V7_SYNTAX_ERROR,
    V7_EXEC_EXCEPTION,
    V7_AST_TOO_LARGE,
    V7_INTERNAL_ERROR,
  };
  
---

Code which is returned by some of the v7 functions. If something other than
`V7_OK` is returned from some function, the caller function typically should
either immediately cleanup and return the code further, or handle the error. 

