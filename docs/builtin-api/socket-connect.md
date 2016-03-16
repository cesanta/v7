---
title: Socket.connect()
signature: |
  Socket.connect(host, port [, is_udp]) -> socket_obj
---

Connect to a given host. `host` can be a string IP address, or a host name.
Optional `is_udp` parameter, if true, indicates that socket should be UDP.
Return socket object on success, null on error.
