---
title: Builtin API
items:
  - { type: file, name: crypto-base64-decode.md }
  - { type: file, name: crypto-base64-encode.md }
  - { type: file, name: crypto-md5.md }
  - { type: file, name: crypto-sha1.md }
  - { type: file, name: file-eval.md }
  - { type: file, name: file-list.md }
  - { type: file, name: file-open.md }
  - { type: file, name: file-read.md }
  - { type: file, name: file-remove.md }
  - { type: file, name: file-rename.md }
  - { type: file, name: file-write.md }
  - { type: file, name: fileobj-close.md }
  - { type: file, name: fileobj-read.md }
  - { type: file, name: fileobj-write.md }
  - { type: file, name: socket-connect.md }
  - { type: file, name: socket-listen.md }
  - { type: file, name: socketobj-accept.md }
  - { type: file, name: socketobj-close.md }
  - { type: file, name: socketobj-recv.md }
  - { type: file, name: socketobj-recvall.md }
  - { type: file, name: socketobj-send.md }
---

Builtin API provides additional JavaScript interfaces available for V7 scripts.

File API is a wrapper around standard C calls `fopen()`, `fclose()`, `fread()`,
`fwrite()`, `rename()`, `remove()`.

Crypto API provides functions for base64, md5, and sha1 encoding/decoding.

Socket API provides low-level socket API.
