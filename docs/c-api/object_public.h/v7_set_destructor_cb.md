---
title: "v7_set_destructor_cb()"
decl_name: "v7_set_destructor_cb"
symbol_kind: "func"
signature: |
  void v7_set_destructor_cb(struct v7 *v7, v7_val_t obj, v7_destructor_cb_t *d);
---

Register a callback which will be invoked when a given object gets
reclaimed by the garbage collector.

The callback will be invoked while garbage collection is still in progress
and hence the internal state of the JS heap is in an undefined state.
The callback thus cannot perform any calls to the V7 API and will receive
only the user data associated with the destructed object.

The intended use case is to reclaim resources allocated by C code. 

