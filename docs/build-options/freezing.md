---
title: Freezing
---

The "freezing" feature allows you to save about 10 KB of RAM by moving the JavaScript
object hierarchy of the standard library from RAM to ROM.

- `-DV7_FREEZE` - This macro is defined by build: it causes V7 to generate a
  "snapshot" of the JS object hierarchy and save it as a C source file which
  will contain a pointer to `struct v7_vals` named `fr_vals`. The resulting
  V7 binary should _not_ be used by clients. See also `V7_THAW`.
- `-DV7_FREEZE_NOT_READONLY` - If defined, "frozen" objects will not be stored
  as readonly objects. TODO: explain when would it be appropriate to use it.
- `-DV7_THAW` - If defined, the file generated at the "freeze" stage should be
  included in the binary, and then, instead of initialising all the standard
  library JS objects (and putting it in RAM), V7 will take the object hierarchy
  from the `fr_vals` pointer.
