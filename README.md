V7: Embedded JavaScript engine 
==============================

V7 is a C/C++ library that implements Javascript engine. It is intended
to be embedded into other programs to allow scripting and configuration,
but also it could be used as stand-alone JS interpreter in scripts.
Small size and simplicity are V7's main design goals.

## Features
- Small footprint, under 100 kilobytes of compiled x86 code
- Simple API, easy C/C++ interoperability
- Standard library with file IO, BSD sockets interface, and more

To retain it's small size, V7 does not implement full ECMAScript specification.

## Example: implementing HTTP/Websocket server

Please take a look at [Smart.js](https://github.com/cesanta/Smart.js)

## Example: loading complex JSON configuration

TBD

## Example: using V7 to provide scripting ability for the online game

TBD