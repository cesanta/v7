---
title: v7 stack stat()
signature: |
  int v7_stack_stat(struct v7 *v7, enum v7_stack_stat_what what);
---

Available if only `V7_ENABLE_STACK_TRACKING` is defined.

Returns stack metric specified by the metric id `what`. See
`v7_stack_stat_clean()` 

