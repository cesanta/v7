---
title: "v7_prop_attr_desc_t"
decl_name: "v7_prop_attr_desc_t"
symbol_kind: "typedef"
signature: |
  typedef unsigned long v7_prop_attr_desc_t;
  #define V7_DESC_WRITABLE(v) _V7_MK_DESC_INV(v, V7_PROPERTY_NON_WRITABLE)
  #define V7_DESC_ENUMERABLE(v) _V7_MK_DESC_INV(v, V7_PROPERTY_NON_ENUMERABLE)
  #define V7_DESC_CONFIGURABLE(v) _V7_MK_DESC_INV(v, V7_PROPERTY_NON_CONFIGURABLE)
  #define V7_DESC_GETTER(v) _V7_MK_DESC(v, V7_PROPERTY_GETTER)
  #define V7_DESC_SETTER(v) _V7_MK_DESC(v, V7_PROPERTY_SETTER)
  #define V7_DESC_PRESERVE_VALUE _V7_DESC_PRESERVE_VALUE
---

Property attribute descriptors that may be given to `v7_def()`: for each
attribute (`v7_prop_attr_t`), there is a corresponding macro, which takes
param: either 1 (set attribute) or 0 (clear attribute). If some particular
attribute isn't mentioned at all, it's left unchanged (or default, if the
property is being created)

There is additional flag: `V7_DESC_PRESERVE_VALUE`. If it is set, the
property value isn't changed (or set to `undefined` if the property is being
created) 

