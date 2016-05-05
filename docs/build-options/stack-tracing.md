---
title: Stack tracing
---

- `-DV7_STACK_SIZE=8192` - If defined, v7 will try to measure the amount of
  consumed stack space. See `V7_STACK_GUARD_MIN_SIZE`.
- `-DV7_STACK_GUARD_MIN_SIZE` - If `V7_STACK_SIZE` is defined, and amount of
  consumed stack space goes above the specified value, v7 panics.
- `-DV7_ENABLE_STACK_TRACKING` - Allows to track stack usage of certain
  functions. Used to measure footprint. See `cyg_profile.h` for some details.
