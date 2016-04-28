---
title: "v7_exec_opt()"
decl_name: "v7_exec_opt"
symbol_kind: "func"
signature: |
  enum v7_err v7_exec_opt(struct v7 *v7, const char *js_code,
                          const struct v7_exec_opts *opts, v7_val_t *res);
---

Customizable version of `v7_exec()`: allows to specify various options, see
`struct v7_exec_opts`. 

