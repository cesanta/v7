---
title: v7 rethrow()
signature: |
  enum v7_err v7_rethrow(struct v7 *v7);
---

Rethrow the currently thrown object. In fact, it just returns
V7_EXEC_EXCEPTION. 

