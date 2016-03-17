---
title: v7_interrupt()
decl_name: v7_interrupt
symbol_kind: func
signature: |
  void v7_interrupt(struct v7 *v7);
---

Set an optional C stack limit.

It sets a flag that will cause the interpreter
to throw an InterruptedError.
It's safe to call it from signal handlers and ISRs
on single threaded environments. 

