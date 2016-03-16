---
title: v7 array get()
signature: |
  v7_val_t v7_array_get(struct v7 *, v7_val_t arr, unsigned long index);
---

Return array member at index `index`. If `index` is out of bounds, undefined
is returned. 

