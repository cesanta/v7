V7: Embedded JavaScript engine
==============================

[![Circle CI](https://circleci.com/gh/cesanta/v7.svg?style=shield)](https://circleci.com/gh/cesanta/v7)
[![Appveyor CI (Windows)](https://img.shields.io/appveyor/ci/mmikulicic/v7/branch/master.svg)](https://ci.appveyor.com/project/mmikulicic/v7/branch/master)
[![License](https://img.shields.io/badge/license-GPL_2-green.svg)](https://github.com/cesanta/v7/blob/master/LICENSE)

V7 is a C/C++ library that implements Javascript engine. It is intended
to be embedded into other programs to allow scripting and configuration,
but also it could be used as stand-alone JS interpreter in automation scripts.
Main V7 design factors are:

- Small size: V7 is targeted for embedded environments with constrained
  resources. Both static and runtime footprint is kept to a minimum.
  V7 is the smallest JavaScript engine on the market.
- Embedding simplicity: V7 provides scripting capabilities to existing
  software, and thus an ability to easily export existing functionality into
  JavaScript environment is paramount. V7's C/C++ interface is the
  best on the market.
- Standard: V7 aims to support JavaScript 5.1 and pass standard ECMA tests.
- Performance: V7 aims to be the fastest non-JIT engine available.
- Usable out-of-the-box: V7 provides an optional standard library with
  file IO, Crypto, BSD sockets API and more

In order to embed V7, simply copy two files: [v7.h](v7.h)
and [v7.c](v7.c) into your project, and use V7 embedding API as shown
in multiple examples.

Project Status:

- Tokenizer: completed
- Parser: completed
- Abstract Syntax Tree: completed
- VM: completed
- Standard library: partially implemented
- ECMA tests: coverage 50.27% (as of Mar 15)

Estimated beta release date: May 2015.

## Example: call C/C++ function `foo()` in JavaScript

    // Javascript code. Invokes C function which calculates sum of two numbers
    var result = foo(1, 2);

<!-- -->

    // C code. foo_glue() is invoked when JavaScript makes foo() call
    static v7_val_t foo_glue(struct v7 *v7, v7_val_t this_obj, v7_val_t args) {
      double arg0 = v7_to_double(v7_array_get(v7, args, 0));
      double arg1 = v7_to_double(v7_array_get(v7, args, 1));
      double result = foo(arg0, arg1);        // Call foo()
      return v7_double_to_value(v7, result);  // Return result to JavaScript
    }

    ...
    // Create function "foo" in global JavaScript namespace
    struct v7 *v7 = v7_create();
    v7_set_attribute(v7, v7_get_global_object(v7), "foo", 3, 0,
                     v7_create_cfunction(v7, &exported_foo));
    v7_exec_file(v7, "my_js_code.js");

## Example: call JavaScript function `foo(a, b)` in C/C++

Assume we have a JavaScript function `foo()` that calculates sum of two numbers:

    var foo = function(a, b) {
      return a + b;
    };

Here is how we call that function from C/C++:

    v7_val_t func, result, args;
    
    /* Lookup function */
    func = v7_get(v7, v7_get_global_object(v7), "foo", 3);
    
    /* Prepare arguments: array with two numbers */
    args = v7_create_array(v7);
    v7_array_push(v7, args, v7_to_double(123.0));
    v7_array_push(v7, args, v7_to_double(456.0));
    
    /* Call function. Pass undefined as `this` */
    result = v7_apply(v7, func, v7_create_undefined(), args);
    printf("Result: %lf\n", v7_to_double(result));

## Example: loading complex JSON configuration

    struct v7 *v7 = v7_create();
    v7_exec(v7, "var config = load('config.json');");   // Load JSON config
    v7_val_t = v7_exec(v7, "config.devices[2].name");   // Lookup value
    printf("Device2 name: [%s]\n", v7_to_json(v7, v));

Note: JavaScript configuration may contain not only static data, but also
some dynamic logic in form of JavaScript function calls. That makes it possible
to write adaptive configuration and offload configuration logic to JavaScript.

## V7 Architecture

V7 has four loosely coupled layers:

1. Tokenizer. Reads input source and returns next token.
2. Parser. Calls tokenizer to parse the source code, and builds an AST
3. Abstract Syntax Tree (AST) - a representation of a program's syntax tree
   in portable binary form
4. Interpreter - an actual VM that executes AST

V7 is deployed as two files: [v7.h](v7.h) that defines V7 embedding API,
and [v7.c](v7.c) which is an engine itself. Note that [v7.c](v7.c) is a
generated file, amalgamated from the source files in [src](src) directory.
Whereas end-user will use [v7.c](v7.c) file, all references in this
document are made to actual source files in [src](src) directory.

### Utilities

#### Mutable/growing memory buffer (mbuf)

Source is in [src/mbuf.h](src/mbuf.h) and [src/mbuf.c](src/mbuf.c).
Implements a memory buffer that can append data to the end of a buffer, or
insert data into arbitrary position in the middle of a buffer. The buffer
grows automatically when needed. Mbuf is used as a backing store for
the AST and strings.

### Tokenizer
### Parser
### Abstract Syntax Tree (AST)
### Interpreter (VM)

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
