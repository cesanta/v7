---
title: File.write()
signature: |
  File.write(file_name, body) -> boolean
---

Writes the `body` into the `file_name`, creating the file if it doesn't exist or truncating it otherwise.
Returns true in case of success.

Example:

```c
File.write('/tmp/foo, 'bar');
```
