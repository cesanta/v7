---
title: File.open()
signature: |
  File.open(file_name [, mode]) -> file_object or null
---

Open a file `path`. For list of valid `mode` values, see `fopen()`
documentation. If `mode` is not specified, mode `rb` is used, i.e. file is
opened in read-only mode.  Return an opened file object, or null on error.

Example:

```c
var f = File.open('/etc/passwd'); f.close();
```
