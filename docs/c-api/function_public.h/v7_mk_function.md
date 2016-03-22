---
title: "v7_mk_function()"
decl_name: "v7_mk_function"
symbol_kind: "func"
signature: |
  v7_val_t v7_mk_function(struct v7 *, v7_cfunction_t *func);
---

Make a JS function object backed by a cfunction.

`func` is a C callback.

A function object is JS object having the Function prototype that holds a
cfunction value in a hidden property.

The function object will have a `prototype` property holding an object that
will be used as the prototype of objects created when calling the function
with the `new` operator. 

