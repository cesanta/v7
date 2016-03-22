---
title: "v7_mk_cfunction()"
decl_name: "v7_mk_cfunction"
symbol_kind: "func"
signature: |
  v7_val_t v7_mk_cfunction(v7_cfunction_t *func);
---

Make a JS value that holds C/C++ callback pointer.

CAUTION: This is a low-level function value. It's not a real object and
cannot hold user defined properties. You should use `v7_mk_function` unless
you know what you're doing. 

