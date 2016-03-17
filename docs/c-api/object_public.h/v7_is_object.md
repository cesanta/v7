---
title: v7_is_object()
decl_name: v7_is_object
symbol_kind: func
signature: |
  int v7_is_object(v7_val_t v);
---

Returns true if the given value is an object or function.
i.e. it returns true if the value holds properties and can be
used as argument to `v7_get`, `v7_set` and `v7_def`. 

