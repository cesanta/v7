---
title: File.open()
signature: |
  File.open(file_name [, mode]) -> file_object or null
---

Opens a file `path`. For a list of valid `mode` values, see `fopen()`
documentation. If `mode` is not specified, mode `rb` is used, i.e. file is
opened in read-only mode.  Returns an opened file object or null on error.

Example:

```c
var f = File.open('/etc/passwd'); f.close();
```
