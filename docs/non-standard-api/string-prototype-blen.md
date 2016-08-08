---
title: String.prototype.blen
signature: |
  String.prototype.blen -> number
---

Returns string length in bytes.

Example: `"ы".blen` returns 2. Note that `"ы".length` is 1, since that string
consists of a single Unicode character (2-byte).
