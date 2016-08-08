---
title: File.read()
signature: |
  File.read(file_name) -> string or undefined
---

Opens a file `path`, reads until EOF and returns. In case of errors
`undefined` is returned.

```c
var passwd = File.read('/etc/passwd');
```
