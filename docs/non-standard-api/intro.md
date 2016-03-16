---
title: Non-Standard API
---

V7 has several non-standard extensions for `String.prototype` in order to give
a compact and fast API to access raw data obtained from File, Socket, and
hardware input/output such as I2C.  V7 IO API functions return string data as a
result of read operations, and that string data is a raw byte array. ECMA6
provides `ArrayBuffer` and `DataView` API for dealing with raw bytes, because
strings in JavaScript are Unicode. That standard API is too bloated for the
embedded use, and does not allow to use handy String API (e.g. `.match()`)
against data.
 *
V7 internally stores strings as byte arrays. All strings created by the String
API are UTF8 encoded. Strings that are the result of input/output API calls
might not be a valid UTF8 strings, but nevertheless they are represented as
strings, and the following API allows to access underlying byte sequence:
