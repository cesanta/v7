---
title: v7 mk function with proto()
signature: |
  v7_val_t v7_mk_function_with_proto(struct v7 *v7, v7_cfunction_t *f,
                                     v7_val_t proto);
---

Make f a JS function with specified prototype `proto`, so that the resulting
function is better suited for the usage as a constructor. 

