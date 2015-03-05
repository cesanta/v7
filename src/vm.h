/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED

#include "internal.h"

/* TODO(mkm): remove ifdef once v7 has been moved here */
#ifndef V7_VALUE_DEFINED
typedef uint64_t val_t;
#endif

#define V7_TAG_OBJECT    ((uint64_t) 0xFFFF << 48)
#define V7_TAG_FOREIGN   ((uint64_t) 0xFFFE << 48)
#define V7_TAG_UNDEFINED ((uint64_t) 0xFFFD << 48)
#define V7_TAG_BOOLEAN   ((uint64_t) 0xFFFC << 48)
#define V7_TAG_NAN       ((uint64_t) 0xFFFB << 48)
#define V7_TAG_STRING_I  ((uint64_t) 0xFFFA << 48)  /* Inlined string len < 5 */
#define V7_TAG_STRING_5  ((uint64_t) 0xFFF9 << 48)  /* Inlined string len 5 */
#define V7_TAG_STRING_O  ((uint64_t) 0xFFF8 << 48)  /* Owned string */
#define V7_TAG_STRING_F  ((uint64_t) 0xFFF7 << 48)  /* Foreign string */
#define V7_TAG_STRING_C  ((uint64_t) 0xFFF6 << 48)  /* String chunk */
#define V7_TAG_FUNCTION  ((uint64_t) 0xFFF5 << 48)  /* JavaScript function */
#define V7_TAG_CFUNCTION ((uint64_t) 0xFFF4 << 48)  /* C function */
#define V7_TAG_GETSETTER ((uint64_t) 0xFFF3 << 48)  /* getter+setter */
#define V7_TAG_REGEXP    ((uint64_t) 0xFFF2 << 48)  /* Regex */
#define V7_TAG_MASK      ((uint64_t) 0xFFFF << 48)

#define V7_NULL V7_TAG_FOREIGN
#define V7_UNDEFINED V7_TAG_UNDEFINED

struct v7_property {
  struct v7_property *next; /* Linkage in struct v7_object::properties */
  val_t name;               /* Property name (a string) */
  val_t value;              /* Property value */

  unsigned int attributes;
#define V7_PROPERTY_READ_ONLY    1
#define V7_PROPERTY_DONT_ENUM    2
#define V7_PROPERTY_DONT_DELETE  4
#define V7_PROPERTY_HIDDEN       8
#define V7_PROPERTY_GETTER      16
#define V7_PROPERTY_SETTER      32
};

/*
 * An object is an unordered collection of properties.
 * A function stored in a property of an object is called a method.
 * A property has a name, a value, and set of attributes.
 * Attributes are: ReadOnly, DontEnum, DontDelete, Internal.
 *
 * A constructor is a function that creates and initializes objects.
 * Each constructor has an associated prototype object that is used for
 * inheritance and shared properties. When a constructor creates an object,
 * the new object references the constructor’s prototype.
 *
 * Objects could be a "generic objects" which is a collection of properties,
 * or a "typed object" which also hold an internal value like String or Number.
 * Those values are implicit, unnamed properties of the respective types,
 * and can be coerced into primitive types by calling a respective constructor
 * as a function:
 *    var a = new Number(123);
 *    typeof(a) == 'object';
 *    typeof(Number(a)) == 'number';
 */
struct v7_object {
  /* First HIDDEN property in a chain is an internal object value */
  struct v7_property *properties;
  struct v7_object *prototype;
  uintptr_t debug;
};

/*
 * Variables are function-scoped and are hoisted.
 * Lexical scoping & closures: each function has a chain of scopes, defined
 * by the lexicographic order of function definitions.
 * Scope is different from the execution context.
 * Execution context carries "variable object" which is variable/value
 * mapping for all variables defined in a function, and `this` object.
 * If function is not called as a method, then `this` is a global object.
 * Otherwise, `this` is an object that contains called method.
 * New execution context is created each time a function call is performed.
 * Passing arguments through recursion is done using execution context, e.g.
 *
 *    var factorial = function(num) {
 *      return num < 2 ? 1 : num * factorial(num - 1);
 *    };
 *
 * Here, recursion calls the same function `factorial` several times. Execution
 * contexts for each call form a stack. Each context has different variable
 * object, `vars`, with different values of `num`.
 */

struct v7_function {
  /*
   * Functions are objects. This has to be the first field so that function
   * objects can be managed by the GC.
   */
  struct v7_property *properties;
  struct v7_object *scope;    /* lexical scope of the closure */
  uintptr_t debug;
  struct ast *ast;            /* AST, used as a byte code for execution */
  unsigned int ast_off;       /* Position of the function node in the AST */
  unsigned int attributes;    /* Function attributes */
#define V7_FUNCTION_STRICT    1
};

struct v7_regexp {
  val_t regexp_string;
  val_t flags_string;
  struct slre_prog *compiled_regexp;
};

#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

/* TODO(mkm): possibly replace those with macros for inlining */
enum v7_type val_type(struct v7 *v7, val_t);
int v7_is_error(struct v7 *v7, val_t);
V7_PRIVATE val_t v7_pointer_to_value(void *);

val_t v7_object_to_value(struct v7_object *);
val_t v7_function_to_value(struct v7_function *);

struct v7_object *v7_to_object(val_t);
struct v7_function *v7_to_function(val_t);
V7_PRIVATE void *v7_to_pointer(val_t v);

V7_PRIVATE void init_object(struct v7 *v7);
V7_PRIVATE void init_array(struct v7 *v7);
V7_PRIVATE void init_error(struct v7 *v7);
V7_PRIVATE void init_boolean(struct v7 *v7);
V7_PRIVATE void init_math(struct v7 *v7);
V7_PRIVATE void init_string(struct v7 *v7);
V7_PRIVATE void init_number(struct v7 *v7);
V7_PRIVATE void init_json(struct v7 *v7);
V7_PRIVATE void init_date(struct v7 *v7);
V7_PRIVATE void init_function(struct v7 *v7);
V7_PRIVATE void init_stdlib(struct v7 *v7);

V7_PRIVATE int set_cfunc_prop(struct v7 *, val_t, const char *, v7_cfunction_t);
V7_PRIVATE v7_val_t v7_create_cfunction_object(struct v7 *, v7_cfunction_t,
                                               int);
V7_PRIVATE v7_val_t v7_create_cfunction_ctor(struct v7 *, val_t, v7_cfunction_t,
                                             int);

V7_PRIVATE int set_cfunc_obj_prop(struct v7 *, val_t obj, const char *name,
                                  v7_cfunction_t f, int num_args);

V7_PRIVATE val_t v_get_prototype(struct v7 *, val_t);
V7_PRIVATE int is_prototype_of(struct v7 *, val_t, val_t);

/* TODO(lsm): NaN payload location depends on endianness, make crossplatform */
#define GET_VAL_NAN_PAYLOAD(v) ((char *) &(v))

V7_PRIVATE val_t create_object(struct v7 *, val_t);
V7_PRIVATE v7_val_t v7_create_function(struct v7 *v7);
V7_PRIVATE int v7_stringify_value(struct v7 *, val_t, char *, size_t);
V7_PRIVATE struct v7_property *v7_create_property(struct v7 *);

V7_PRIVATE struct v7_property *v7_get_own_property(struct v7 *, val_t,
                                                   const char *, size_t);
V7_PRIVATE struct v7_property *v7_get_own_property2(struct v7 *, val_t obj,
                                                    const char *name,
                                                    size_t, unsigned int attrs);

/* If `len` is -1/MAXUINT/~0, then `name` must be 0-terminated */
V7_PRIVATE struct v7_property *v7_get_property(struct v7 *, val_t obj,
                                               const char *name, size_t);
V7_PRIVATE void v7_invoke_setter(struct v7 *, struct v7_property *, val_t,
                                 val_t);
V7_PRIVATE int v7_set_v(struct v7 *, v7_val_t, v7_val_t, v7_val_t);
V7_PRIVATE int v7_set_property_v(struct v7 *, v7_val_t obj, v7_val_t name,
                                 unsigned int attributes, v7_val_t val);
V7_PRIVATE int v7_set_property(struct v7 *, v7_val_t obj, const char *name,
                               size_t len, unsigned int attributes,
                               v7_val_t val);

/* Return address of property value or NULL if the passed property is NULL */
V7_PRIVATE val_t v7_property_value(struct v7 *, val_t, struct v7_property *);

/*
 * If `len` is -1/MAXUINT/~0, then `name` must be 0-terminated.
 * Return 0 on success, -1 on error.
 */
V7_PRIVATE int v7_del_property(struct v7 *, val_t, const char *, size_t);

/*
 * Returns the array length, or `-1` if the object is not an array
 */
V7_PRIVATE uint32_t v7_array_length(struct v7 *v7, val_t);
V7_PRIVATE long arg_long(struct v7 *v7, val_t args, int n, long default_value);
V7_PRIVATE int to_str(struct v7 *v7, val_t v, char *buf, size_t size,
                      int as_json);
V7_PRIVATE void v7_destroy_property(struct v7_property **p);
V7_PRIVATE val_t i_value_of(struct v7 *v7, val_t v);
V7_PRIVATE val_t Std_eval(struct v7 *v7, val_t t, val_t args);

/* String API */
V7_PRIVATE int s_cmp(struct v7 *, val_t a, val_t b);
V7_PRIVATE val_t s_concat(struct v7 *, val_t, val_t);
V7_PRIVATE val_t s_substr(struct v7 *, val_t, long, long);
V7_PRIVATE void embed_string(struct mbuf *, size_t, const char *, size_t, int);
/* TODO(mkm): rename after regexp merge */
V7_PRIVATE val_t to_string(struct v7 *v7, val_t v);

V7_PRIVATE val_t Obj_valueOf(struct v7 *, val_t, val_t);
V7_PRIVATE double i_as_num(struct v7 *, val_t);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */

#endif  /* VM_H_INCLUDED */
