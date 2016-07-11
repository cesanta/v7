---
title: "struct prop_iter_ctx"
decl_name: "struct prop_iter_ctx"
symbol_kind: "struct"
signature: |
  struct prop_iter_ctx {
  #if V7_ENABLE__Proxy
    struct prop_iter_proxy_ctx *proxy_ctx;
  #endif
    struct v7_property *cur_prop;
  
    unsigned init : 1;
  };
---

Context for property iteration, see `v7_next_prop()`.

Clients should not interpret contents of this structure, it's here merely to
allow clients to allocate it not from the heap. 

