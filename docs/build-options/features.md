---
title: Features
---

- `-DV7_BUILD_PROFILE=1` - Builds minimal configuration: disable Date, UTF8,
  RegExp.
- `-DV7_BUILD_PROFILE=2` - Builds medium-sized configuration: disable all Date
  functions but `Date.now()`.
- `-DV7_BUILD_PROFILE=3` - Builds full configuration.
- `-DV7_ENABLE_CRYPTO` - Enables builtin Crypto API.
- `-DV7_ENABLE_FILE` - Enables built-in File API.
- `-DV7_ENABLE_JS_GETTERS` - Enables JS getters.
- `-DV7_ENABLE_JS_SETTERS` - Enables JS setters.
- `-DV7_ENABLE_SOCKET` - Enables built-in Socket API.
- `-DV7_EXE` - Exports `main()` function, i.e. create V7 executable.
- `-DV7_NO_FS` - Disables all functions that access the filesystem, like
  `v7_exec_file()`, `File`, etc.
- `-DV7_NO_COMPILER` - Disables JavaScript compiler and all related modules:
  tokenizer, parser, AST. V7 will only be able to evaluate a precompiled JS
  bytecode, not a real JS code. NOTE: currently it's only possible to use this
  option together with `V7_THAW` (see "Freezing" section below), since the normal
  V7 initialisation routine currently includes evaluating some real JavaScript
  code.
