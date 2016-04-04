---
title: File.write()
signature: |
  File.write(file_name, body) -> boolean
---

Write `body` into `file_name`, creating the file if it doesn't exist, or truncating it otherwise.
Return true in case of success.

Example:

```c
File.write('/tmp/foo, 'bar');
```
