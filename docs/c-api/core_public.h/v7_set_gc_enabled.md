---
title: "v7_set_gc_enabled()"
decl_name: "v7_set_gc_enabled"
symbol_kind: "func"
signature: |
  void v7_set_gc_enabled(struct v7 *v7, int enabled);
---

Enable or disable GC.

Must be called before invoking v7_exec or v7_apply
from within a cfunction unless you know what you're doing.

GC is disabled during execution of cfunctions in order to simplify
memory management of simple cfunctions.
However executing even small snippets of JS code causes a lot of memory
pressure. Enabling GC solves that but forces you to take care of the
reachability of your temporary V7 v7_val_t variables, as the GC needs
to know where they are since objects and strings can be either reclaimed
or relocated during a GC pass. 

