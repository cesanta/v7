---
title: GC debug
---

- `-DV7_DISABLE_GC` - Disable garbage collector. That will lead to memory
  leaks. Practical to debug garbage collector issues.
- `-DV7_DISABLE_STR_ALLOC_SEQ` - Unless defined, v7 helps to spot bugs with
  "owning" of string values: it uses 16 bits of the `v7_val_t` to store number
  which increments every time GC compacts the owned string. We call this number
  ASN. So there always is a range of the ASNs which are valid, and if v7
  encounters usage of the string with ASN outside of this range, it panics.
- `-DV7_GC_VERBOSE` - Make GC verbose about every ASN assignment. It is useful
  to turn on this option when invalid ASN is detected, reproduce the problem
  and stare at logs.
- `-DV7_GC_AFTER_STRING_ALLOC` - Invoke GC after every string allocation
- `-DV7_GC_PANIC_ON_ASN` - When GC detects the GC rooting problem (usage of the
  string which was reallocated), call `abort()`.
- `-DV7_MALLOC_GC` - Instead of using a slab allocator for GC, use malloc.
- `-DV7_ENABLE_GC_CHECK` - Use instrumentation (cyg_profile) in order to check
  GC owning. TODO: explain better.
