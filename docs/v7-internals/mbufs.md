---
title: Mbufs
---

Mbufs are mutable/growing memory buffers, similar to C++ strings:

```
struct mbuf {
  char *buf;
  size_t len;
  size_t size;
};
```

Mbuf can append data to the end of a buffer or insert data into an arbitrary
position in the middle of a buffer. The buffer grows automatically when needed.
Mbuf is used as a backing store for the AST and strings.
