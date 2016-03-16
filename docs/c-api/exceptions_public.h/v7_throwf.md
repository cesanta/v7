---
title: v7 throwf()
signature: |
  enum v7_err v7_throwf(struct v7 *v7, const char *typ, const char *err_fmt, ...);
---

Throw an exception with given formatted message.

Pass "Error" as typ for a generic error. 

