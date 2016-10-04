---
title: "v7_next_prop()"
decl_name: "v7_next_prop"
symbol_kind: "func"
signature: |
  int v7_next_prop(struct v7 *v7, struct prop_iter_ctx *ctx, v7_val_t *name,
                   v7_val_t *value, v7_prop_attr_t *attrs);
---

Iterate over the `obj`'s properties.

Usage example (here we assume we have some `v7_val_t obj`):

    struct prop_iter_ctx ctx;
    v7_val_t name, val;
    v7_prop_attr_t attrs;

    v7_init_prop_iter_ctx(v7, obj, &ctx);
    while (v7_next_prop(v7, &ctx, &name, &val, &attrs)) {
      if (V7_PROP_ATTR_IS_ENUMERABLE(attrs)) continue;
      ...
    }
    v7_destruct_prop_iter_ctx(v7, &ctx);

As you see, v7_next_prop will iterate through all properties, including
non-enumerable ones, and it's your responsibility to test the attributes
with the provided `V7_PROP_ATTR_*` macros and proceed as you see fit. 

