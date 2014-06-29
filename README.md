V7: Embedded JavaScript engine 
==============================

Note: This project is in the design phase.

V7 is a C/C++ library that implements Javascript engine. It is intended
to be embedded into other programs to allow scripting and configuration,
but also it could be used as stand-alone JS interpreter in scripts.
Small size and simplicity are V7's main design driving factors.

## Features
- Small footprint, under 100 kilobytes of compiled x86 code
- Simple API, easy C/C++ interoperability
- Standard library with file IO, BSD sockets interface, and more

To retain it's small size, V7 does not implement full ECMAScript specification.

## Example: using V7 to implement HTTP/Websocket server

Please take a look at [websocket.js](https://github.com/cesanta/websocket.js)

## Example: using V7 to load complex configuration in JSON format

TBD

## Example: using V7 to provide scripting ability for the online game

TBD