---
title: v7 to cstring()
signature: |
  const char *v7_to_cstring(struct v7 *v7, v7_val_t *v);
---

Returns a pointer to the string stored in `v7_val_t`.

Returns NULL if the value is not a string or if the string is not compatible
with a C string.

C compatible strings contain exactly one NUL char, in terminal position.

All strings owned by the V7 engine (see v7_mk_string) are guaranteed to
be NUL terminated.
Out of these, those that don't include embedded NUL chars are guaranteed to
be C compatible. 

