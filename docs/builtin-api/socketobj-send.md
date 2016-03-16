---
title: sock.send(string)
signature: |
  sock.send(string) -> num_bytes_sent
---

Send string to the socket. Return number of bytes sent, or 0 on error.
Simple HTTP client example:

```c
var s = Socket.connect("google.com", 80);
s.send("GET / HTTP/1.0\n\n");
var reply = s.recv();
```
