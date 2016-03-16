---
title: Build options
---

- `-DV7_BUILD_PROFILE=1` - build minimal configuration:
  disable Date, UTF8, RegExp
- `-DV7_BUILD_PROFILE=2` - build medium-sized configuration: disable all Date
  functions but `Date.now()`
- `-DV7_BUILD_PROFILE=3` - build full configuration
- `-DV7_NO_FS` - disable all functions that access filesystem, like
  `v7_exec_file()`, `File`, etc
- `-DV7_DISABLE_GC` - disable garbage collector. That will lead to memory
  leaks. Practical to debug garbage collector issues.
- `-DV7_ENABLE_FILE` - enable builtin File API
- `-DV7_ENABLE_SOCKET` - enable builtin Socket API
- `-DV7_ENABLE_CRYPTO` - enable builtin Crypto API
- `-DV7_EXE` - export `main()` function, i.e. create V7 executable
