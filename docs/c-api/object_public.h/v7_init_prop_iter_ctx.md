---
title: "v7_init_prop_iter_ctx()"
decl_name: "v7_init_prop_iter_ctx"
symbol_kind: "func"
signature: |
  enum v7_err v7_init_prop_iter_ctx(struct v7 *v7, v7_val_t obj,
                                    struct prop_iter_ctx *ctx);
---

Initialize the property iteration context `ctx`, see `v7_next_prop()` for
usage example. 

