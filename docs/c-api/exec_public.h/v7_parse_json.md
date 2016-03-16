---
title: v7 parse json()
signature: |
  enum v7_err v7_parse_json(struct v7 *v7, const char *str, v7_val_t *res);
---

Parse `str` and store corresponding JavaScript object in `res` variable.
String `str` should be '\0'-terminated.
Return value and semantic is the same as for `v7_exec()`. 

