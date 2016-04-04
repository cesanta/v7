---
title: File.read()
signature: |
  File.read(file_name) -> string or undefined
---

Open a file `path`, read until EOF and return. In case of errors
`undefined` is returned.

```c
var passwd = File.read('/etc/passwd');
```
