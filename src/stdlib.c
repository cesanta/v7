static void Std_print(struct v7_c_func_arg *cfa) {
  char buf[4000];
  int i;
  for (i = 0; i < cfa->num_args; i++) {
    printf("%s", v7_to_string(cfa->args[i], buf, sizeof(buf)));
  }
  putchar('\n');
}

static void Std_load(struct v7_c_func_arg *cfa) {
  int i;

  v7_init_bool(cfa->result, 1);
  for (i = 0; i < cfa->num_args; i++) {
    if (cfa->args[i]->type != V7_TYPE_STR ||
        v7_exec_file(cfa->v7, cfa->args[i]->v.str.buf) != V7_OK) {
      cfa->result->v.num = 0.0;
      break;
    }
  }
}

static void Std_exit(struct v7_c_func_arg *cfa) {
  int exit_code = cfa->num_args > 0 ? (int) cfa->args[0]->v.num : EXIT_SUCCESS;
  exit(exit_code);
}

static void base64_encode(const unsigned char *src, int src_len, char *dst) {
  static const char *b64 =
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
static unsigned char from_b64(unsigned char ch) {
  // Inverse lookup map
  static const unsigned char tab[128] = {
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

static void base64_decode(const unsigned char *s, int len, char *dst) {
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

static void Std_base64_decode(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0];

  v7_init_str(cfa->result, NULL, 0, 0);
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    cfa->result->v.str.len = v->v.str.len * 3 / 4 + 1;
    cfa->result->v.str.buf = (char *) malloc(cfa->result->v.str.len + 1);
    base64_decode((const unsigned char *) v->v.str.buf, v->v.str.len,
                  cfa->result->v.str.buf);
  }
}

static void Std_base64_encode(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0];

  v7_init_str(cfa->result, NULL, 0, 0);
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    cfa->result->v.str.len = v->v.str.len * 3 / 2 + 1;
    cfa->result->v.str.buf = (char *) malloc(cfa->result->v.str.len + 1);
    base64_encode((const unsigned char *) v->v.str.buf, v->v.str.len,
                  cfa->result->v.str.buf);
  }
}

static void Std_eval(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0];
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    int old_line_no = cfa->v7->line_no;
    cfa->v7->line_no = 1;
    do_exec(cfa->v7, v->v.str.buf, cfa->v7->sp);
    cfa->v7->line_no = old_line_no;
#if 0
    if (cfa->v7->sp > old_sp) {
      *cfa->result = *v7_top(cfa->v7)[-1];
      inc_ref_count(cfa->result);
    }
#endif
  }
}


static void init_stdlib(void) {
  static v7_c_func_t ctors[V7_NUM_CLASSES] = {
    Array_ctor, Boolean_ctor, Date_ctor, Error_ctor, Function_ctor,
    Number_ctor, Object_ctor, Regex_ctor, String_ctor
  };
  int i;

  for (i = 0; i < V7_NUM_CLASSES; i++) {
    s_prototypes[i].type = s_constructors[i].type = V7_TYPE_OBJ;
    s_prototypes[i].ref_count = s_constructors[i].ref_count = 1;
    s_prototypes[i].proto = &s_prototypes[V7_CLASS_OBJECT];
    s_prototypes[i].ctor = &s_constructors[V7_CLASS_FUNCTION];
    s_constructors[i].proto = &s_prototypes[i];
    s_constructors[i].ctor = &s_constructors[V7_CLASS_FUNCTION];
    s_constructors[i].v.c_func = ctors[i];
  }

  SET_METHOD(s_prototypes[V7_CLASS_OBJECT], "toString", Obj_toString);
  SET_METHOD(s_prototypes[V7_CLASS_OBJECT], "keys", Obj_keys);

  SET_METHOD(s_math, "random", Math_random);
  SET_METHOD(s_math, "pow", Math_pow);
  SET_METHOD(s_math, "sin", Math_sin);
  SET_METHOD(s_math, "tan", Math_tan);
  SET_METHOD(s_math, "sqrt", Math_sqrt);

  SET_RO_PROP(s_prototypes[V7_CLASS_NUMBER], "MAX_VALUE",
              V7_TYPE_NUM, num, LONG_MAX);
  SET_RO_PROP(s_prototypes[V7_CLASS_NUMBER], "MIN_VALUE",
              V7_TYPE_NUM, num, LONG_MIN);
  SET_RO_PROP(s_prototypes[V7_CLASS_NUMBER], "NaN",
              V7_TYPE_NUM, num, NAN);
  SET_METHOD(s_prototypes[V7_CLASS_NUMBER], "toFixed", Num_toFixed);

  SET_METHOD(s_json, "stringify", Json_stringify);

#ifndef V7_DISABLE_CRYPTO
  SET_METHOD(s_crypto, "md5", Crypto_md5);
  SET_METHOD(s_crypto, "md5_hex", Crypto_md5_hex);
  SET_RO_PROP_V(s_global, "Crypto", s_crypto);
  v7_set_class(&s_crypto, V7_CLASS_OBJECT);
  s_crypto.ref_count = 1;
#endif

  SET_PROP_FUNC(s_prototypes[V7_CLASS_ARRAY], "length", Arr_length);
  SET_METHOD(s_prototypes[V7_CLASS_ARRAY], "push", Arr_push);
  SET_METHOD(s_prototypes[V7_CLASS_ARRAY], "sort", Arr_sort);

  SET_PROP_FUNC(s_prototypes[V7_CLASS_STRING], "length", Str_length);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charCodeAt", Str_charCodeAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charAt", Str_charAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "indexOf", Str_indexOf);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "substr", Str_substr);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "match", Str_match);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "split", Str_split);

  SET_RO_PROP(s_math, "E", V7_TYPE_NUM, num, M_E);
  SET_RO_PROP(s_math, "PI", V7_TYPE_NUM, num, M_PI);
  SET_RO_PROP(s_math, "LN2", V7_TYPE_NUM, num, M_LN2);
  SET_RO_PROP(s_math, "LN10", V7_TYPE_NUM, num, M_LN10);
  SET_RO_PROP(s_math, "LOG2E", V7_TYPE_NUM, num, M_LOG2E);
  SET_RO_PROP(s_math, "LOG10E", V7_TYPE_NUM, num, M_LOG10E);
  SET_RO_PROP(s_math, "SQRT1_2", V7_TYPE_NUM, num, M_SQRT1_2);
  SET_RO_PROP(s_math, "SQRT2", V7_TYPE_NUM, num, M_SQRT2);

  SET_METHOD(s_global, "print", Std_print);
  SET_METHOD(s_global, "exit", Std_exit);
  SET_METHOD(s_global, "load", Std_load);
  SET_METHOD(s_global, "base64_encode", Std_base64_encode);
  SET_METHOD(s_global, "base64_decode", Std_base64_decode);
  SET_METHOD(s_global, "eval", Std_eval);

  SET_RO_PROP_V(s_global, "Object", s_constructors[V7_CLASS_OBJECT]);
  SET_RO_PROP_V(s_global, "Number", s_constructors[V7_CLASS_NUMBER]);
  SET_RO_PROP_V(s_global, "String", s_constructors[V7_CLASS_STRING]);
  SET_RO_PROP_V(s_global, "Array", s_constructors[V7_CLASS_ARRAY]);
  SET_RO_PROP_V(s_global, "RegExp", s_constructors[V7_CLASS_REGEXP]);
  SET_RO_PROP_V(s_global, "Function", s_constructors[V7_CLASS_FUNCTION]);
  SET_RO_PROP_V(s_global, "Date", s_constructors[V7_CLASS_DATE]);
  SET_RO_PROP_V(s_global, "Error", s_constructors[V7_CLASS_ERROR]);

  SET_RO_PROP_V(s_global, "Math", s_math);
  SET_RO_PROP_V(s_global, "JSON", s_json);

  v7_set_class(&s_math, V7_CLASS_OBJECT);
  v7_set_class(&s_json, V7_CLASS_OBJECT);
  v7_set_class(&s_global, V7_CLASS_OBJECT);
  s_math.ref_count = s_json.ref_count = s_global.ref_count = 1;
}
