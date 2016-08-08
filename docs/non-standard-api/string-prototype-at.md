---
title: String.prototype.at()
signature: |
  String.prototype.at(position) -> number or NaN
---

Returns bytes at index `position`. The byte value is in the 0,255 range. If `position` is
out of bounds (either negative or larger then the byte array length), `NaN` is
returned.  Example: `"ы".at(0)` returns `0xd1`.
