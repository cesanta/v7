---
title: "v7_mk_foreign()"
decl_name: "v7_mk_foreign"
symbol_kind: "func"
signature: |
  NOINSTR v7_val_t v7_mk_foreign(struct v7 *v7, void *ptr);
---

Make JavaScript value that holds C/C++ `void *` pointer.

A foreign value is completely opaque and JS code cannot do anything useful
with it except holding it in properties and passing it around.
It behaves like a sealed object with no properties.

NOTE:
Only valid pointers (as defined by each supported architecture) will fully
preserved. In particular, all supported 64-bit architectures (x86_64, ARM-64)
actually define a 48-bit virtual address space.
Foreign values will be sign-extended as required, i.e creating a foreign
value of something like `(void *) -1` will work as expected. This is
important because in some 64-bit OSs (e.g. Solaris) the user stack grows
downwards from the end of the address space.

If you need to store exactly sizeof(void*) bytes of raw data where
`sizeof(void*)` >= 8, please use byte arrays instead. 

