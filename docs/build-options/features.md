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
