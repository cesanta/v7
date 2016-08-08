---
title: Socket.listen()
signature: |
  Socket.listen(port [, ip_address [,is_udp]]) -> socket_obj
---

Creates a listening socket on a given port. Optional `ip_address` argument
specifies an IP address to bind to. Optional `is_udp` parameter, if true,
indicates that the socket should be UDP. Returns a socket object on success, null on
error.
