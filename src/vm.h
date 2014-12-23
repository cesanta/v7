/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED

#include "internal.h"

#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

/* Forward declarations */
struct v7_ast;    /* V7 Abstract Syntax Tree. */
struct v7_arg;    /* C/JavaScript function parameters */
struct v7_object;

typedef double v7_num_t;    /* Override to integer on systems with no MMU */
typedef void (*v7_func2_t)(struct v7_arg *arg);

/*
 * Strings might have embedded zero bytes. They might be interned.
 * Also, they might be referencing foreign memory blocks, e.g. from C code,
 * without making a copy.
 * Therefore, define string as a structure that describes generic memory block.
 */
struct v7_str {
  char *buf;
  v7_strlen_t len;
};

union v7_valholder {
  int boolean;
  v7_num_t number;
  struct v7_str string;
  struct v7_object *object;
  v7_func2_t c_function;
};

struct v7_value {
  union v7_valholder value;
  enum v7_type type;          /* This is going to waste a lot for alignment */
};

struct v7_property {
  struct v7_property *next;   /* Linkage in struct v7_object::properties */
  char *name;                 /* Property name is a zero-terminated string */
  struct v7_value value;      /* Property value */

  unsigned int attributes;
#define V7_PROPERTY_READ_ONLY    1
#define V7_PROPERTY_DONT_ENUM    2
#define V7_PROPERTY_DONT_DELETE  4
#define V7_PROPERTY_HIDDEN       8
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
};

/* Argument to all C/JavaScript glue functions */
struct v7_arg {
  struct v7 *v7;
  struct v7_val *this_obj;
  struct v7_val **args;
  int num_args;
  int called_as_constructor;
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
  int starting_line_number;   /* Starting line in the source code */
  struct ast *ast;            /* AST, used as a byte code for execution */
  struct v7_object vars;      /* Declared variables & functions */
};

/*
 * Create a value with the given type.
 * Last arguments to a function depend on `type`:
 * - For `V7_TYPE_UNDEFINED` and `V7_TYPE_NULL` - no argument
 * - For `V7_TYPE_NUMBER`, `v7_num_t` argument
 * - For `V7_TYPE_BOOLEAN`, `int` argument
 * - For `V7_TYPE_STRING`, `char *` followed by `v7_strlen_t` argument
 * - For everything else, `v7_value *` argument. Value is not copied.
 * Return NULL on failure.
 */
struct v7_value *v7_create_value(struct v7 *, enum v7_type, ...);
struct v7_value *v7_va_create_value(struct v7 *, enum v7_type, va_list);

int v7_stringify_value(struct v7 *, struct v7_value *, char *, size_t);
int v7_to_json(struct v7 *, struct v7_value *, char *, size_t);

int v7_set_property_value(struct v7 *, struct v7_value *obj,
                          const char *name, v7_strlen_t len,
                          unsigned int attributes,
                          struct v7_value *val);

/*
 * Set a property for an object.
 * `obj` must be a object value. Last arguments depend on `type` (see above).
 * Return 0 on success, -1 on error.
 */
int v7_set_property(struct v7 *, struct v7_value *obj,
                    const char *name, v7_strlen_t len,
                    unsigned int attributes, enum v7_type, ...);

/* If `len` is -1/MAXUINT/~0, then `name` must be 0-terminated */
V7_PRIVATE struct v7_property *v7_get_property(struct v7_value *obj,
                                               const char *name, v7_strlen_t);

/* Return address of property value or NULL if the passed property is NULL */
V7_PRIVATE struct v7_value *v7_property_value(struct v7_property *);

/*
 * If `len` is -1/MAXUINT/~0, then `name` must be 0-terminated.
 * Return 0 on success, -1 on error.
 */
V7_PRIVATE int v7_del_property(struct v7_value *, const char *, v7_strlen_t);

V7_PRIVATE int v7_is_object(struct v7_value *);

/*
 * Returns the array length as JS number, or `undefined` if the object is not an array
 */
V7_PRIVATE struct v7_value *v7_array_length(struct v7 *v7, struct v7_value *);

#endif  /* VM_H_INCLUDED */
