---
title: "v7_stringify()"
decl_name: "v7_stringify"
symbol_kind: "func"
signature: |
  char *v7_stringify(struct v7 *v7, v7_val_t v, char *buf, size_t len,
                     enum v7_stringify_mode mode);
---

Generate string representation of the JavaScript value `val` into a buffer
`buf`, `len`. If `len` is too small to hold a generated string,
`v7_stringify()` allocates required memory. In that case, it is caller's
responsibility to free the allocated buffer. Generated string is guaranteed
to be 0-terminated.

Available stringification modes are:

- `V7_STRINGIFY_DEFAULT`:
  Convert JS value to string, using common JavaScript semantics:
  - If value is an object:
    - call `toString()`;
    - If `toString()` returned non-primitive value, call `valueOf()`;
    - If `valueOf()` returned non-primitive value, throw `TypeError`.
  - Now we have a primitive, and if it's not a string, then stringify it.

- `V7_STRINGIFY_JSON`:
  Generate JSON output

- `V7_STRINGIFY_DEBUG`:
  Mostly like JSON, but will not omit non-JSON objects like functions.

Example code:

    char buf[100], *p;
    p = v7_stringify(v7, obj, buf, sizeof(buf), V7_STRINGIFY_DEFAULT);
    printf("JSON string: [%s]\n", p);
    if (p != buf) {
      free(p);
    } 

