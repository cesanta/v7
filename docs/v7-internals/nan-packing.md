---
title: NaN packing
---

V7 uses a well-known NaN packing technique for values. This is a method of
storing values in the IEEE 754 double-precision floating-point number.
`double` type is 64-bit, comprised of 1 sign bit, 11 exponent bits and
52 mantissa bits:

```
    7         6        5        4        3        2        1        0
seeeeeee|eeeemmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm
```

If an exponent is all-1 and mantissa is non-0, then `double` value represents
NaN (not-a-number). V7 uses first 4 bits of mantissa to store value type,
and the rest 48 bits of mantissa goes to storing values:

```
11111111|1111tttt|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv
 NaN marker |type|  48-bit placeholder for values: pointers, strings
```

On 64-bit platforms, pointers occupy only 48 bits, so they can fit into the
48-bit placeholder - provided they are sign extended.
