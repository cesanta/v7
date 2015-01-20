/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"


V7_PRIVATE void init_error(struct v7 *v7) {
  val_t v;
  v7_exec(v7, &v, "function Error(m) {this.message = m}");
  v7_exec(v7, &v, "function TypeError(m) {this.message = m};"
            "TypeError.prototype = Object.create(Error.prototype)");
  v7_exec(v7, &v, "function SyntaxError(m) {this.message = m};"
            "SyntaxError.prototype = Object.create(Error.prototype)");
  v7_exec(v7, &v, "function ReferenceError(m) {this.message = m};"
            "ReferenceError.prototype = Object.create(Error.prototype)");
  v7_exec(v7, &v, "function ImplementationError(m) {this.message = m};"
            "ReferenceError.prototype = Object.create(Error.prototype)");

  v7->error_prototype = v7_property_value(v7_get_property(
      v7_property_value(v7_get_property(v7->global_object, "Error", 5)),
      "prototype", 9));
}
