#include "internal.h"

V7_PRIVATE enum v7_err Std_print(struct v7_c_func_arg *cfa) {
  char *p, buf[500];
  int i;
  for (i = 0; i < cfa->num_args; i++) {
    p = v7_stringify(cfa->args[i], buf, sizeof(buf));
    printf("%s", p);
    if (p != buf) free(p);
  }
  putchar('\n');

  return V7_OK;
}

V7_PRIVATE enum v7_err Std_load(struct v7_c_func_arg *cfa) {
  int i;
  struct v7_val *obj = v7_push_new_object(cfa->v7);

  // Push new object as a context for the loading new module
  obj->next = cfa->v7->ctx;
  cfa->v7->ctx = obj;

  for (i = 0; i < cfa->num_args; i++) {
    if (v7_type(cfa->args[i]) != V7_TYPE_STR) return V7_TYPE_ERROR;
    if (!v7_exec_file(cfa->v7, cfa->args[i]->v.str.buf)) return V7_ERROR;
  }

  // Pop context, and return it
  cfa->v7->ctx = obj->next;
  v7_push_val(cfa->v7, obj);

  return V7_OK;
}

V7_PRIVATE enum v7_err Std_exit(struct v7_c_func_arg *cfa) {
  int exit_code = cfa->num_args > 0 ? (int) cfa->args[0]->v.num : EXIT_SUCCESS;
  exit(exit_code);
  return V7_OK;
}

V7_PRIVATE void base64_encode(const unsigned char *src, int src_len, char *dst) {
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

// Convert one byte of encoded base64 input stream to 6-bit chunk
V7_PRIVATE unsigned char from_b64(unsigned char ch) {
  // Inverse lookup map
  V7_PRIVATE const unsigned char tab[128] = {
    255, 255, 255, 255, 255, 255, 255, 255, //  0
    255, 255, 255, 255, 255, 255, 255, 255, //  8
    255, 255, 255, 255, 255, 255, 255, 255, //  16
    255, 255, 255, 255, 255, 255, 255, 255, //  24
    255, 255, 255, 255, 255, 255, 255, 255, //  32
    255, 255, 255,  62, 255, 255, 255,  63, //  40
     52,  53,  54,  55,  56,  57,  58,  59, //  48
     60,  61, 255, 255, 255, 200, 255, 255, //  56   '=' is 200, on index 61
    255,   0,   1,   2,   3,   4,   5,   6, //  64
      7,   8,   9,  10,  11,  12,  13,  14, //  72
     15,  16,  17,  18,  19,  20,  21,  22, //  80
     23,  24,  25, 255, 255, 255, 255, 255, //  88
    255,  26,  27,  28,  29,  30,  31,  32, //  96
     33,  34,  35,  36,  37,  38,  39,  40, //  104
     41,  42,  43,  44,  45,  46,  47,  48, //  112
     49,  50,  51, 255, 255, 255, 255, 255, //  120
  };
  return tab[ch & 127];
}

V7_PRIVATE void base64_decode(const unsigned char *s, int len, char *dst) {
  unsigned char a, b, c, d;
  while (len >= 4 &&
         (a = from_b64(s[0])) != 255 &&
         (b = from_b64(s[1])) != 255 &&
         (c = from_b64(s[2])) != 255 &&
         (d = from_b64(s[3])) != 255) {
    if (a == 200 || b == 200) break;  // '=' can't be there
    *dst++ = a << 2 | b >> 4;
    if (c == 200) break;
    *dst++ = b << 4 | c >> 2;
    if (d == 200) break;
    *dst++ = c << 6 | d;
    s += 4;
    len -=4;
  }
  *dst = 0;
}

V7_PRIVATE enum v7_err Std_base64_decode(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0], *result;

  result = v7_push_string(cfa->v7, NULL, 0, 0);
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    result->v.str.len = v->v.str.len * 3 / 4 + 1;
    result->v.str.buf = (char *) malloc(result->v.str.len + 1);
    base64_decode((const unsigned char *) v->v.str.buf, (int) v->v.str.len,
                  result->v.str.buf);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_base64_encode(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0], *result;

  result = v7_push_string(cfa->v7, NULL, 0, 0);
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    result->v.str.len = v->v.str.len * 3 / 2 + 1;
    result->v.str.buf = (char *) malloc(result->v.str.len + 1);
    base64_encode((const unsigned char *) v->v.str.buf, (int) v->v.str.len,
                  result->v.str.buf);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_eval(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0];
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    return do_exec(cfa->v7, "<eval>", v->v.str.buf, cfa->v7->sp);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_read(struct v7_c_func_arg *cfa) {
  struct v7_val *v;
  char buf[2048];
  size_t n;

  if ((v = v7_get(cfa->this_obj, "fp")) != NULL &&
      (n = fread(buf, 1, sizeof(buf), (FILE *) (unsigned long) v->v.num)) > 0) {
    v7_push_string(cfa->v7, buf, n, 1);
  } else {
    v7_make_and_push(cfa->v7, V7_TYPE_NULL);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_write(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0], *result;
  size_t n, i;

  result = v7_push_number(cfa->v7, 0);
  if ((v = v7_get(cfa->this_obj, "fp")) != NULL) {
    for (i = 0; (int) i < cfa->num_args; i++) {
      if (is_string(cfa->args[i]) &&
          (n = fwrite(cfa->args[i]->v.str.buf, 1, cfa->args[i]->v.str.len,
                      (FILE *) (unsigned long) v->v.num)) > 0) {
        result->v.num += n;
      }
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_close(struct v7_c_func_arg *cfa) {
  struct v7_val *v;
  int ok = 0;
  if ((v = v7_get(cfa->this_obj, "fp")) != NULL &&
      fclose((FILE *) (unsigned long) v->v.num) == 0) {
    ok = 1;
  }
  v7_push_bool(cfa->v7, ok);
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_open(struct v7_c_func_arg *cfa) {
  struct v7_val *v1 = cfa->args[0], *v2 = cfa->args[1], *result = NULL;
  FILE *fp;

  if (cfa->num_args == 2 && is_string(v1) && is_string(v2) &&
      (fp = fopen(v1->v.str.buf, v2->v.str.buf)) != NULL) {
    result = v7_push_new_object(cfa->v7);
    result->proto = &s_file;
    v7_setv(cfa->v7, result, V7_TYPE_STR, V7_TYPE_NUM,
            "fp", 2, 0, (double) (unsigned long) fp);  // after v7_set_class !
  } else {
    v7_make_and_push(cfa->v7, V7_TYPE_NULL);
  }
  return V7_OK;
}

V7_PRIVATE void init_stdlib(void) {
  init_object();
  init_number();
  init_array();
  init_string();
  init_regex();
  init_function();
  init_date();
  init_error();
  init_boolean();
  init_math();
  init_json();
#ifndef V7_DISABLE_CRYPTO
  init_crypto();
#endif

  SET_METHOD(s_global, "print", Std_print);
  SET_METHOD(s_global, "exit", Std_exit);
  SET_METHOD(s_global, "load", Std_load);
  SET_METHOD(s_global, "base64_encode", Std_base64_encode);
  SET_METHOD(s_global, "base64_decode", Std_base64_decode);
  SET_METHOD(s_global, "eval", Std_eval);
  SET_METHOD(s_global, "open", Std_open);

  SET_METHOD(s_file, "read", Std_read);
  SET_METHOD(s_file, "write", Std_write);
  SET_METHOD(s_file, "close", Std_close);

  v7_set_class(&s_file, V7_CLASS_OBJECT);
  s_file.ref_count = 1;

  v7_set_class(&s_global, V7_CLASS_OBJECT);
  s_global.ref_count = 1;
}
