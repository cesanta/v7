---
title: "struct v7_exec_opts"
decl_name: "struct v7_exec_opts"
symbol_kind: "struct"
signature: |
  struct v7_exec_opts {
    /* Filename, used for stack traces only */
    const char *filename;
  
    /*
     * Object to be used as `this`. Note: when it is zeroed out, i.e. it's a
     * number `0`, the `undefined` value is assumed. It means that it's
     * impossible to actually use the number `0` as `this` object, but it makes
     * little sense anyway.
     */
    v7_val_t this_obj;
  
    /* Whether the given `js_code` should be interpreted as JSON, not JS code */
    unsigned is_json : 1;
  };
---

Options for `v7_exec_opt()`. To get default options, like `v7_exec()` uses,
just zero out this struct. 

