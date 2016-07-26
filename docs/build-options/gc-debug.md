---
title: GC debug
---

- `-DV7_DISABLE_GC` - Disables the garbage collector. This will lead to memory
  leaks. Practical to debug garbage collector issues.
- `-DV7_DISABLE_STR_ALLOC_SEQ` - Unless defined, V7 helps to spot bugs by
  "owning" string values: it uses 16 bits of the `v7_val_t` to store the number
  which increments every time GC compacts the owned string. We call this number
  ASN. So there is always a range of the ASNs which are valid and if V7
  encounters any usage of the string with ASN outside of this range, it panics.
- `-DV7_GC_VERBOSE` - Makes GC verbose about every ASN assignment. It is useful
  to turn this option on when invalid ASN is detected. Reproduces the problem
  and stares at logs.
- `-DV7_GC_AFTER_STRING_ALLOC` - Invokes GC after every string allocation.
- `-DV7_GC_PANIC_ON_ASN` - When GC detects the GC rooting problem (usage of the
  string which was reallocated), calls `abort()`.
- `-DV7_MALLOC_GC` - Instead of using a slab allocator for GC, uses malloc.
- `-DV7_ENABLE_GC_CHECK` - Uses instrumentation (cyg_profile) in order to check
  GC owning. TODO: explain better.
