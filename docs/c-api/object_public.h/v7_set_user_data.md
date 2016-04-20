---
title: "v7_set_user_data()"
decl_name: "v7_set_user_data"
symbol_kind: "func"
signature: |
  void v7_set_user_data(struct v7 *v7, v7_val_t obj, void *ud);
---

Associates an opaque C value (anything that can be casted to a `void * )
with an object.

You can achieve a similar effect by just setting a special property with
a foreign value (see `v7_mk_foreign`), except user data offers the following
advantages:

1. You don't have to come up with some arbitrary "special" property name.
2. JS scripts cannot access user data by mistake via property lookup.
3. The user data is available to the destructor. When the desctructor is
   invoked you cannot access any of its properties.
4. Allows the implementation to use a more compact encoding

Does nothing if `obj` is not a mutable object. 

