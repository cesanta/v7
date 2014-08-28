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
        return V7_ERROR;                // Signal error: expecting 2 arguments
      } else {
        double arg0 = v7_number(cfa->args[0]);
        double arg1 = v7_number(cfa->args[1]);
        double result = foo(arg0, arg1);
        v7_push_number(cfa->v7, result);  // Push result
        return V7_OK;                     // Signal success
      }
    }
    ...
    // Export variable "foo" as C function to the root namespace
    v7_set_func(v7, v7_rootns(v7), "foo", v7_push_func(v7, &exported_foo));

<!-- -->

    // Javascript code
    var y = foo(1, 2);

## Example: loading complex JSON configuration

    struct v7 *v7 = v7_create();
    v7_exec(v7, "var config = load('config.json');");   // Load JSON config
    v7_exec(v7, "config.devices[2].name");              // Lookup value
    printf("Device2 name: [%s]\n", v7_top_val(v7)->v.str.buf);

## API documentation

    struct v7 *v7_create(void);       // Creates and initializes V7 engine
    void v7_destroy(struct v7 **);    // Cleanes up and deallocates V7 engine
    enum v7_err v7_exec(struct v7 *, const char *str);        // Executes string
    enum v7_err v7_exec_file(struct v7 *, const char *path);  // Executes file
    struct v7_val *v7_rootns(struct v7 *);  // Returns global obj (root namespace)
    char *v7_stringify(const struct v7_val *v, char *buf, int bsiz);
    const char *v7_get_error_string(const struct v7 *);  // Returns error string
    int v7_is_true(const struct v7_val *);
    void v7_copy(struct v7 *v7, struct v7_val *from, struct v7_val *to);
    enum v7_err v7_set(struct v7 *, struct v7_val *, const char *, struct v7_val *);
    enum v7_err v7_del(struct v7 *, struct v7_val *obj, const char *key);
    struct v7_val *v7_get(struct v7_val *obj, const char *key);
    struct v7_val *v7_call(struct v7 *v7, struct v7_val *this_obj, int num_args);
    struct v7_val *v7_push_number(struct v7 *, double num);
    struct v7_val *v7_push_bool(struct v7 *, int is_true);
    struct v7_val *v7_push_string(struct v7 *, const char *str, int len, int own);
    struct v7_val *v7_push_new_object(struct v7 *);
    struct v7_val *v7_push_val(struct v7 *, struct v7_val *);
    struct v7_val *v7_push_func(struct v7 *, v7_func_t);
    enum v7_type v7_type(const struct v7_val *);
    double v7_number(const struct v7_val *);
    const char *v7_string(const struct v7_val *, unsigned long *len);

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