---
title: v7_next_prop()
decl_name: v7_next_prop
symbol_kind: func
signature: |
  void *v7_next_prop(void *handle, v7_val_t obj, v7_val_t *name, v7_val_t *value,
                     v7_prop_attr_t *attrs);
---

Iterate over the `obj`'s properties.

Usage example:

    void *h = NULL;
    v7_val_t name, val;
    unsigned int attrs;
    while ((h = v7_next_prop(h, obj, &name, &val, &attrs)) != NULL) {
      ...
    } 

