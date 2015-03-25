/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#define STRINGIFY(x) #x

V7_PRIVATE void init_js_stdlib(struct v7 *v7) {
  val_t res;

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.indexOf = function(a, x) {
      var i; var r = -1; var b = +x;
      if (!b || b < 0) b = 0;
      for (i in this) if (i >= b && (r < 0 || i < r) && this[i] === a) r = +i;
      return r;
    };));

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.lastIndexOf = function(a, x) {
      var i; var r = -1; var b = +x;
      if (isNaN(b) || b < 0 || b >= this.length) b = this.length - 1;
      for (i in this) if (i <= b && (r < 0 || i > r) && this[i] === a) r = +i;
      return r;
    };));

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.reduce = function(a, b) {
      var f = 0;
      if (typeof(a) != "function") {
        throw new TypeError(a + " is not a function");
      }
      for (var k in this) {
        if (f == 0 && b === undefined) {
          b = this[k];
          f = 1;
        } else {
          b = a(b, this[k], k, this);
        }
      }
      return b;
    };));

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.pop = function() {
      var i = this.length - 1;
      return this.splice(i, 1)[0];
    };));

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.shift = function() {
      return this.splice(0, 1)[0];
    };));

  v7_exec(v7, &res, STRINGIFY(
    Function.prototype.call = function() {
      var t = arguments.splice(0, 1)[0];
      return this.apply(t, arguments);
    };));

  /* TODO(lsm): re-enable in a separate PR */
#if 0
  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.unshift = function() {
      var a = new Array(0, 0);
      Array.prototype.push.apply(a, arguments);
      Array.prototype.splice.apply(this, a);
      return this.length;
    };));
#endif
}
