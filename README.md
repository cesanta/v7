V7: Embedded JavaScript engine
==============================

V7 is a C/C++ library that implements Javascript engine. It is intended
to be embedded into other programs to allow scripting and configuration,
but also it could be used as stand-alone JS interpreter in automation scripts.
Main V7 design factors are:

- Small size and simplicity. V7 can fit into resource-constrained environments,
  like firmware, embedded devices, etc. V7 is under 100kb of compiled
  x86_64 code.
- Easy C/C++ integration. Exporting existing C/C++ functions into V7 and
  vice versa is easier then for any other embedded language.
- Simple embedding process. V7 could be embedded into existing project simply
  by copying two files, `v7.c` and `v7.h` and adding few lines of C/C++ code.
- Optional standard library with file IO, Crypto, BSD sockets and more

V7 is targeted to conform to the ECMAScript specification version 5.

## Restrictions

- V7 enforces [strict mode](http://goo.gl/Rhqzs2), even if `use strict`
is not specified in the source code. Benefits of strict mode are 
outlined [here](http://goo.gl/MlBicD)
- All strings passed to `v7_exec()` functions must be immutable. This is to
simplify memory management and reduce memory usage. V7 stores references to
variables and functions without making an internal copy.

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

## API documentation

## Licensing

V7 is released under commercial and
[GNU GPL v.2](http://www.gnu.org/licenses/old-licenses/gpl-2.0.html) open
source licenses. The GPLv2 open source License does not generally permit
incorporating this software into non-open source programs.
For those customers who do not wish to comply with the GPLv2 open
source license requirements,
[Cesanta Software](http://cesanta.com) offers a full,
royalty-free commercial license and professional support
without any of the GPL restrictions.