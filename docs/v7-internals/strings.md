---
title: Strings
---

Strings in V7 are stored as UTF8 chunks of memory. They are always
0-terminated, in order to be suitable for standard C string functions.
V7 has four distinct string types:

- `#define V7_TAG_STRING_I ((uint64_t) 0xFFFA << 48)` - this is an inlined
  string. Inlined string are fully contained in the NaN payload. NaN payload
  is 6 bytes in total. First byte of the payload specifies string length.
  Another byte is 0 terminator. Therefore, inlined strings could only be
  4 bytes or less. For example, inlined string `hi!` looks like this:

```
11111111|1111tttt|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv
 NaN marker |0xa |   3    |    h   |    i   |    !   |  \0    |
```

- `#define V7_TAG_STRING_5 ((uint64_t) 0xFFF9 << 48)` - this is an inlined
   string exactly 5 bytes in size: 6 bytes of string payload + 1 zero byte.
   For example, string `hello` looks like this:

```
11111111|1111tttt|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv
 NaN marker |0x9 |   h    |    e   |    l   |    l   |   o    |  \0
```

- `#define V7_TAG_STRING_O ((uint64_t) 0xFFF8 << 48)` - this is an owned
   string, i.e. string data is owned by V7. This type is for larger strings.
   NaN payload is a pointer to a memory chunk containing varint-encoded
   string length followed by string data, 0-terminated. All such memory
   chunks are stored in `struct v7::owned_strings` mbuf. Example of a string
   `Mukacheve` looks like this:

```
11111111|1111tttt|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv
 NaN marker |0x8 |    0    |    0   |  0xa  |  0xb   |   0xc   |  0xd
```

```
        pointer 0xabcd points inside struct v7::owned_strings mbuf
                   v
|------------------|0x9|Mukacheve\0|-----...other strings...
```


- `#define V7_TAG_STRING_F ((uint64_t) 0xFFF7 << 48)` - this string type is
   used for long (>5 bytes) strings, whose data is not owned by V7. NaN payload
   is a pointer to a memory chunk containing varint-encoded string length
   followed by a pointer to string data outside V7. V7 cannot guarantee that
   the foreign string is 0-terminated. Example of a foreign string
   `Marko The Great`:

```
11111111|1111tttt|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv
 NaN marker |0x7 |    0    |    0   |  0xa  |  0xb   |   0xc   |  0xd
```

```
        pointer 0xabcd points inside struct v7::foreign_strings mbuf
                   v
|------------------|0xf|0x123456|-----...other strings...
```

```
 pointer 0x12345 points to a foreign string
           v
-----------|Marko The Great|------
```

