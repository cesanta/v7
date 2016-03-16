---
title: v7 main()
signature: |
  int v7_main(int argc, char *argv[], void (*pre_freeze_init);
---

V7 executable main function.

There are various callbacks available:

`pre_freeze_init()` and `pre_init()` are optional intialization functions,
aimed to export any extra functionality into vanilla v7 engine. They are
called after v7 initialization, before executing given files or inline
expressions. `pre_freeze_init()` is called before "freezing" v7 state;
whereas `pre_init` called afterwards.

`post_init()`, if provided, is called after executing files and expressions,
before destroying v7 instance and exiting. 

