---
title: "v7_own()"
decl_name: "v7_own"
symbol_kind: "func"
signature: |
  void v7_own(struct v7 *v7, v7_val_t *v);
---

Tells the GC about a JS value variable/field owned
by C code.

User C code should own v7_val_t variables
if the value's lifetime crosses any invocation
to the v7 runtime that creates new objects or new
properties and thus can potentially trigger GC.

The registration of the variable prevents the GC from mistakenly treat
the object as garbage. The GC might be triggered potentially
allows the GC to update pointers

User code should also explicitly disown the variables with v7_disown once
it goes out of scope or the structure containing the v7_val_t field is freed.

Example:

 ```
   struct v7_val cb;
   v7_own(v7, &cb);
   cb = v7_array_get(v7, args, 0);
   // do something with cb
   v7_disown(v7, &cb);
 ``` 

