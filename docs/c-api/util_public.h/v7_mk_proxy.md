---
title: "v7_mk_proxy()"
decl_name: "v7_mk_proxy"
symbol_kind: "func"
signature: |
  v7_val_t v7_mk_proxy(struct v7 *v7, v7_val_t target,
                       const v7_proxy_hnd_t *handler);
---

Create a Proxy object, see:
https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Global_Objects/Proxy

Only two traps are implemented so far: `get()` and `set()`. Note that
`Object.defineProperty()` bypasses the `set()` trap.

If `target` is not an object, the empty object will be used, so it's safe
to pass `V7_UNDEFINED` as `target`. 

