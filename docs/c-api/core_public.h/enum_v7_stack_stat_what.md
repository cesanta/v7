---
title: enum v7_stack_stat_what
decl_name: enum v7_stack_stat_what
symbol_kind: enum
signature: |
  enum v7_stack_stat_what {
    /* max stack size consumed by `i_exec()` */
    V7_STACK_STAT_EXEC,
    /* max stack size consumed by `parse()` (which is called from `i_exec()`) */
    V7_STACK_STAT_PARSER,
  
    V7_STACK_STATS_CNT
  };
---

Available if only `V7_ENABLE_STACK_TRACKING` is defined.

Stack metric id. See `v7_stack_stat()` 

