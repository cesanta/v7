/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

V7_PRIVATE v7_val_t Std_print(struct v7 *v7, val_t this_obj, val_t args) {
  char *p, buf[1024];
  int i, num_args = v7_array_length(v7, args);

  (void) this_obj;
  for (i = 0; i < num_args; i++) {
    p = v7_to_json(v7, v7_array_at(v7, args, i), buf, sizeof(buf));
    printf("%s", p);
    if (p != buf) {
      free(p);
    }
  }
  putchar('\n');

  return v7_create_null();
}

V7_PRIVATE val_t Std_eval(struct v7 *v7, val_t t, val_t args) {
  val_t res = v7_create_undefined(), arg = v7_array_at(v7, args, 0);
  (void) t;
  if (arg != V7_UNDEFINED) {
    char buf[100], *p;
    p = v7_to_json(v7, arg, buf, sizeof(buf));
    if (p[0] == '"') {
      p[0] = p[strlen(p) - 1] = ' ';
    }
    if (v7_exec(v7, &res, p) != V7_OK) {
      throw_value(v7, res);
    }
    if (p != buf) {
      free(p);
    }
  }
  return res;
}

static val_t Std_exit(struct v7 *v7, val_t t, val_t args) {
  int exit_code = arg_long(v7, args, 0, 0);
  (void) t;
  exit(exit_code);
  return v7_create_undefined();
}

static void base64_encode(const unsigned char *src, int src_len, char *dst) {
  V7_PRIVATE const char *b64 =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int i, j, a, b, c;

  for (i = j = 0; i < src_len; i += 3) {
    a = src[i];
    b = i + 1 >= src_len ? 0 : src[i + 1];
    c = i + 2 >= src_len ? 0 : src[i + 2];

    dst[j++] = b64[a >> 2];
    dst[j++] = b64[((a & 3) << 4) | (b >> 4)];
    if (i + 1 < src_len) {
      dst[j++] = b64[(b & 15) << 2 | (c >> 6)];
    }
    if (i + 2 < src_len) {
      dst[j++] = b64[c & 63];
    }
  }
  while (j % 4 != 0) {
    dst[j++] = '=';
  }
  dst[j++] = '\0';
}

/* Convert one byte of encoded base64 input stream to 6-bit chunk */
static unsigned char from_b64(unsigned char ch) {
  /* Inverse lookup map */
  V7_PRIVATE const unsigned char tab[128] = {
    255, 255, 255, 255, 255, 255, 255, 255,  /* 0 */
    255, 255, 255, 255, 255, 255, 255, 255,  /* 8 */
    255, 255, 255, 255, 255, 255, 255, 255,  /* 16 */
    255, 255, 255, 255, 255, 255, 255, 255,  /* 24 */
    255, 255, 255, 255, 255, 255, 255, 255,  /* 32 */
    255, 255, 255, 62,  255, 255, 255, 63,   /* 40 */
    52,  53,  54,  55,  56,  57,  58,  59,   /* 48 */
    60,  61,  255, 255, 255, 200, 255, 255,  /* 56 '=' is 200, on index 61 */
    255, 0,   1,   2,   3,   4,   5,   6,    /* 64 */
    7,   8,   9,   10,  11,  12,  13,  14,   /* 72 */
    15,  16,  17,  18,  19,  20,  21,  22,   /* 80 */
    23,  24,  25,  255, 255, 255, 255, 255,  /* 88 */
    255, 26,  27,  28,  29,  30,  31,  32,   /* 96 */
    33,  34,  35,  36,  37,  38,  39,  40,   /* 104 */
    41,  42,  43,  44,  45,  46,  47,  48,   /* 112 */
    49,  50,  51,  255, 255, 255, 255, 255,  /* 120 */
  };
  return tab[ch & 127];
}

static void base64_decode(const unsigned char *s, int len, char *dst) {
  unsigned char a, b, c, d;
  while (len >= 4 && (a = from_b64(s[0])) != 255 &&
         (b = from_b64(s[1])) != 255 && (c = from_b64(s[2])) != 255 &&
         (d = from_b64(s[3])) != 255) {
    if (a == 200 || b == 200) break;  /* '=' can't be there */
    *dst++ = a << 2 | b >> 4;
    if (c == 200) break;
    *dst++ = b << 4 | c >> 2;
    if (d == 200) break;
    *dst++ = c << 6 | d;
    s += 4;
    len -= 4;
  }
  *dst = 0;
}

static val_t b64_transform(struct v7 *v7, val_t this_obj, val_t args,
                           void (func)(const unsigned char *, int, char *),
                           double mult) {
  val_t arg0 = v7_array_at(v7, args, 0);
  val_t res = v7_create_undefined();

  (void) this_obj;
  if (v7_is_string(arg0)) {
    size_t n;
    const char *s = v7_to_string(v7, &arg0, &n);
    char *buf = (char *) malloc(n * mult + 2);
    if (buf != NULL) {
      func((const unsigned char *) s, (int) n, buf);
      res = v7_create_string(v7, buf, strlen(buf), 1);
      free(buf);
    }
  }

  return res;
}

static val_t Std_base64_decode(struct v7 *v7, val_t this_obj, val_t args) {
  return b64_transform(v7, this_obj, args, base64_decode, 0.75);
}

static val_t Std_base64_encode(struct v7 *v7, val_t this_obj, val_t args) {
  return b64_transform(v7, this_obj, args, base64_encode, 1.5);
}

#ifndef V7_NO_FS
static val_t Std_load(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  val_t res = v7_create_undefined();

  (void) this_obj;
  if (v7_is_string(arg0)) {
    size_t n;
    const char *s = v7_to_string(v7, &arg0, &n);
    v7_exec_file(v7, &res, s);
  }

  return res;
}

/*
 * File interface: a wrapper around open(), close(), read(), write().
 * File.open(path, flags) -> fd.
 * File.close(fd) -> undefined
 * File.read(fd) -> string (empty string on EOF)
 * File.write(fd, str) -> num_bytes_written
 */
static val_t Std_read(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  char buf[2048];
  size_t n;

  (void) this_obj;
  if (v7_is_double(arg0)) {
    int fd = v7_to_double(arg0);
    n = read(fd, buf, sizeof(buf));
    if (n > 0) {
      return v7_create_string(v7, buf, n, 1);
    }
  }

  return v7_create_string(v7, "", 0, 1);
}

static val_t Std_write(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  val_t arg1 = v7_array_at(v7, args, 1);
  size_t n = 0, n2;

  (void) this_obj;
  if (v7_is_double(arg0) && v7_is_string(arg1)) {
    const char *s = v7_to_string(v7, &arg1, &n2);
    int fd = v7_to_double(arg0);
    n = write(fd, s, n2);
  }

  return v7_create_number(n);
}

static val_t Std_close(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  (void) this_obj;
  if (v7_is_double(arg0)) {
    close((int) v7_to_double(arg0));
  }
  return v7_create_undefined();
}

static val_t Std_open(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  val_t arg1 = v7_array_at(v7, args, 1);
  val_t res = v7_create_undefined();

  (void) this_obj;
  if (v7_is_string(arg0)) {
    size_t n1;
    const char *s = v7_to_string(v7, &arg0, &n1);
    int flags = v7_is_double(arg1) ? (int) v7_to_double(arg1) : 0;
    int fd = open(s, flags);
    res = v7_create_number(fd);
  }

  return res;
}
#endif

#define STRINGIFY(x) #x

static void init_js_stdlib(struct v7 *v7) {
  val_t res;

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.indexOf = function(a, b) {
      if (!b || b < 0) b = 0;
      for (var i = b; i < this.length; i++) {
        if (this[i] === a) {
          return i;
        }
      }
      return -1;
    };));

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.lastIndexOf = function(a, b) {
      if (!b || b < 0 || b >= this.length) b = this.length - 1;
      for (var i = b; i >= 0; i--) {
        if (this[i] === a) {
          return i;
        }
      }
      return -1;
    };));

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.reduce = function(a, b) {
      var f = 0;
      if (typeof(a) != 'function') {
        throw new TypeError(a + ' is not a function');
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

V7_PRIVATE void init_stdlib(struct v7 *v7) {
  /*
   * Ensure the first call to v7_create_value will use a null proto:
   * {}.__proto__.__proto__ == null
   */
  v7->object_prototype = create_object(v7, V7_NULL);
  v7->array_prototype = v7_create_object(v7);
  v7->boolean_prototype = v7_create_object(v7);
  v7->string_prototype = v7_create_object(v7);
  v7->regexp_prototype = v7_create_object(v7);
  v7->number_prototype = v7_create_object(v7);
  v7->global_object = v7_create_object(v7);
  v7->this_object = v7->global_object;
  v7->date_prototype = v7_create_object(v7);
  v7->function_prototype = v7_create_object(v7);

  set_cfunc_prop(v7, v7->global_object, "print", Std_print);
  set_cfunc_prop(v7, v7->global_object, "eval", Std_eval);
  set_cfunc_prop(v7, v7->global_object, "exit", Std_exit);
  set_cfunc_prop(v7, v7->global_object, "base64_encode", Std_base64_encode);
  set_cfunc_prop(v7, v7->global_object, "base64_decode", Std_base64_decode);

#ifndef V7_NO_FS
  /* TODO(lsm): move to a File object */
  set_cfunc_prop(v7, v7->global_object, "load", Std_load);
  {
    val_t file_obj = v7_create_object(v7);
    v7_set_property(v7, v7->global_object, "File", 4, 0, file_obj);
    set_cfunc_prop(v7, file_obj, "open", Std_open);
    set_cfunc_prop(v7, file_obj, "close", Std_close);
    set_cfunc_prop(v7, file_obj, "read", Std_read);
    set_cfunc_prop(v7, file_obj, "write", Std_write);
  }
#endif

  v7_set_property(v7, v7->global_object, "Infinity", 8, 0,
                  v7_create_number(INFINITY));
  v7_set_property(v7, v7->global_object, "global", 6, 0, v7->global_object);

  init_object(v7);
  init_array(v7);
  init_error(v7);
  init_boolean(v7);
  init_math(v7);
  init_string(v7);
  init_regex(v7);
  init_number(v7);
  init_json(v7);
  init_date(v7);
  init_function(v7);

  init_js_stdlib(v7);
}
