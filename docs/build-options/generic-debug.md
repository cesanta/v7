---
title: Generic debug
---

- `-DV7_ENABLE_ENTITY_IDS` - Store magic numbers in each JS object, function
  and property, so that when v7 encounters any of the JS entity, it can check
  if magic number is present, and if not, it panics. This feature does _not_
  take any extra space.
