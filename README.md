V7: Embedded JavaScript engine
==============================

V7 is a C/C++ library that implements Javascript engine. It is intended
to be embedded into other programs to allow scripting and configuration,
but also it could be used as stand-alone JS interpreter in scripts.
Small size, simplicity and easy C/C++ integration are V7's main design goals.

## Features
- Small footprint, under 100 kilobytes of compiled x86 code
- Simple embedding API, easy C/C++ interoperability, single .c file
- Standard library with file IO, Crypto, BSD sockets and more

V7 is targeted to conform to the ECMAScript specification version 3.

## Example: implementing HTTP/Websocket server

Please take a look at [Smart.js](https://github.com/cesanta/Smart.js)

## Example: exporting existing C/C++ function `foo()` into Javascript

    // C code. exported_foo() glues C function "foo" to Javascript
    static enum v7_err exported_foo(struct v7_c_func_arg *cfa) {
      if (cfa->num_args != 2) {
        return V7_ERROR;                // Return error: expecting 2 arguments
      } else {
        double res = foo(cfa->args[0]->v.num, cfa->args[1]->v.num);
        v7_init_num(cfa->result, res);  // Set return value
        return V7_OK;                   // Return success
      }
    }
    ...
    // Export variable "foo" as C function to the root namespace
    v7_set_func(v7, v7_rootns(v7), "foo", &exported_foo);

<!-- -->

    // Javascript code
    var y = foo(1, 2);

## Example: loading complex JSON configuration

    struct v7 *v7 = v7_create();
    v7_exec(v7, "var config = load('config.json');");   // Load JSON config
    v7_exec(v7, "config.devices[2].name");              // Lookup value
    printf("Device2 name: [%s]\n", v7_top_val(v7)->v.str.buf);
