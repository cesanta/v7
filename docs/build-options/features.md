---
title: Features
---

- `-DV7_BUILD_PROFILE=1` - Build minimal configuration: disable Date, UTF8,
  RegExp.
- `-DV7_BUILD_PROFILE=2` - Build medium-sized configuration: disable all Date
  functions but `Date.now()`.
- `-DV7_BUILD_PROFILE=3` - Build full configuration.
- `-DV7_ENABLE_CRYPTO` - Enable builtin Crypto API.
- `-DV7_ENABLE_FILE` - Enable builtin File API.
- `-DV7_ENABLE_JS_GETTERS` - Enabje JS getters.
- `-DV7_ENABLE_JS_SETTERS` - Enable JS setters.
- `-DV7_ENABLE_SOCKET` - Enable builtin Socket API.
- `-DV7_EXE` - Export `main()` function, i.e. create V7 executable.
- `-DV7_NO_FS` - Disable all functions that access filesystem, like
  `v7_exec_file()`, `File`, etc.
- `-DV7_NO_COMPILER` - Disable JavaScript compiler and all related modules:
  tokenizer, parser, AST. V7 will only be able to evaluate a precompiled JS
  bytecode, not a real JS code. NOTE: currently it's only possible to use this
  option together with `V7_THAW` (see "Freezing" section below), since normal
  v7 initialization routine currently includes evaluating some real JavaScript
  code.
