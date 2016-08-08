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

The File API is a wrapper around standard C calls `fopen()`, `fclose()`, `fread()`,
`fwrite()`, `rename()`, `remove()`.

The Crypto API provides functions for Base64, MD5, and SHA1 encoding/decoding.

The Socket API provides a low-level socket API.
