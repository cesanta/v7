---
title: "v7_gc()"
decl_name: "v7_gc"
symbol_kind: "func"
signature: |
  void v7_gc(struct v7 *v7, int full);
---

Perform garbage collection.
Pass true to full in order to reclaim unused heap back to the OS. 

