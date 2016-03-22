---
title: "struct v7_create_opts"
decl_name: "struct v7_create_opts"
symbol_kind: "struct"
signature: |
  struct v7_create_opts {
    size_t object_arena_size;
    size_t function_arena_size;
    size_t property_arena_size;
  #ifdef V7_STACK_SIZE
    void *c_stack_base;
  #endif
  #ifdef V7_FREEZE
    /* if not NULL, dump JS heap after init */
    char *freeze_file;
  #endif
  };
---

Customizations of initial V7 state; used by `v7_create_opt()`. 

