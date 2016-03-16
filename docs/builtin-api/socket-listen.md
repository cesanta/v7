---
title: Socket.listen()
signature: |
  Socket.listen(port [, ip_address [,is_udp]]) -> socket_obj
---

Create a listening socket on a given port. Optional `ip_address` argument
specifies and IP address to bind to. Optional `is_udp` parameter, if true,
indicates that socket should be UDP. Return socket object on success, null on
error.
