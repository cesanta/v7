---
title: Generic debug
---

- `-DV7_ENABLE_ENTITY_IDS` - Stores magic numbers in each JS object, function
  and property, so that when V7 encounters any of the JS entity, it can check
  if a magic number is present, and if not, it panics. This feature does _not_
  take any extra space.
