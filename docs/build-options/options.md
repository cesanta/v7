---
title: Options
---

- `-DV7_ALLOW_ARGLESS_MAIN` - When v7 is invoked without any arguments,
  continue working instead of printing usage help and exiting. This is useful
  if we invoke `v7_main()` with some `pre_init()` and/or `post_init()`
  functions, which may implement some kind of REPL, or whatever.
- `-DV7_FORCE_STRICT_MODE` - Execute all scripts in the "strict mode",
  independently of the `"use strict"` literal.
- `-DV7_LARGE_AST` - Use 32 bits for relative AST offsets instead of 16 bits.
  It allows v7 to handle much bigger scripts, but consumes more RAM to compile
  scripts (once script is compiled and is ready to execute, AST data is freed)
- `-DV7_MMAP_EXEC` - If defined, v7 uses `mmap()` instead of `fread()` to read
  script files. Usage of `mmap()` saves RAM, but, obviously, it can be used if
  only `mmap()` is implemented on the target platform.
- `-DV7_TEMP_OFF` - If defined, use manually-written double-to-string
  conversion; otherwise, use `snprintf`. TODO: find better name.
