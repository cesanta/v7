---
title: "v7_prop_attr_t"
decl_name: "v7_prop_attr_t"
symbol_kind: "typedef"
signature: |
  typedef unsigned short v7_prop_attr_t;
  #define V7_PROPERTY_NON_WRITABLE (1 << 0)
  #define V7_PROPERTY_NON_ENUMERABLE (1 << 1)
  #define V7_PROPERTY_NON_CONFIGURABLE (1 << 2)
  #define V7_PROPERTY_GETTER (1 << 3)
  #define V7_PROPERTY_SETTER (1 << 4)
  #define _V7_PROPERTY_HIDDEN (1 << 5)
  /* property not managed by V7 HEAP */
  #define _V7_PROPERTY_OFF_HEAP (1 << 6)
  /* special property holding user data and destructor cb */
  #define _V7_PROPERTY_USER_DATA_AND_DESTRUCTOR (1 << 7)
  /*
   * not a property attribute, but a flag for `v7_def()`. It's here in order to
   * keep all offsets in one place
   */
  #define _V7_DESC_PRESERVE_VALUE (1 << 8)
---

Property attributes bitmask 

