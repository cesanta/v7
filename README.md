V7: Embedded JavaScript engine
==============================

V7 is a C/C++ library that implements Javascript engine. It is intended
to be embedded into other programs to allow scripting and configuration,
but also it could be used as stand-alone JS interpreter in scripts.
Small size, simplicity and easy C/C++ integration are V7's main design goals.

## Features
- Small footprint, under 100 kilobytes of compiled x86 code
- Simple API, easy C/C++ interoperability
- Standard library with file IO, BSD sockets interface, and more

V7 is targeted to conform to the ECMAScript specification version 3.

## Example: implementing HTTP/Websocket server

Please take a look at [Smart.js](https://github.com/cesanta/Smart.js)

## Example: loading complex JSON configuration

    struct v7 *v7 = v7_create();
    v7_exec(v7, "var config = load('config.json');");   // Load JSON config
    v7_exec(v7, "config.devices[2].name");              // Lookup value
    printf("Device2 name: [%s]\n", v7_top(v7)[-1].v.str.buf);

## Example: using V7 as a game scripting engine

