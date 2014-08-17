#include "internal.h"

V7_PRIVATE enum v7_err String_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : cfa->result;
  struct v7_val *arg = cfa->args[0];

  // If argument is not a string, do type conversion
  if (cfa->num_args == 1 && !is_string(arg)) {
    TRY(toString(cfa->v7, arg));
    arg = v7_top_val(cfa->v7);
  }

  if (cfa->num_args == 1 && arg->type == V7_TYPE_STR) {
    v7_init_str(obj, arg->v.str.buf, arg->v.str.len, 1);
  } else {
    v7_init_str(obj, NULL, 0, 0);
  }

  if (cfa->called_as_constructor) {
    cfa->this_obj->type = V7_TYPE_OBJ;
    cfa->this_obj->proto = &s_prototypes[V7_CLASS_STRING];
    cfa->this_obj->ctor = &s_constructors[V7_CLASS_STRING];
  }
  return V7_OK;
}

V7_PRIVATE void Str_length(struct v7_val *this_obj, struct v7_val *result) {
  v7_init_num(result, this_obj->v.str.len);
}

V7_PRIVATE enum v7_err Str_charCodeAt(struct v7_c_func_arg *cfa) {
  double idx = cfa->num_args > 0 && cfa->args[0]->type == V7_TYPE_NUM ?
  cfa->args[0]->v.num : NAN;
  const struct v7_string *str = &cfa->this_obj->v.str;

  v7_init_num(cfa->result, NAN);
  if (!isnan(idx) && cfa->this_obj->type == V7_TYPE_STR && fabs(idx) < str->len) {
    cfa->result->v.num = ((unsigned char *) str->buf)[(int) idx];
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_charAt(struct v7_c_func_arg *cfa) {
  double idx = cfa->num_args > 0 && cfa->args[0]->type == V7_TYPE_NUM ?
  cfa->args[0]->v.num : NAN;
  const struct v7_string *str = &cfa->this_obj->v.str;

  if (!isnan(idx) && cfa->this_obj->type == V7_TYPE_STR &&
      fabs(idx) < str->len) {
    v7_init_str(cfa->result, &str->buf[(int) idx], 1, 1);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_match(struct v7_c_func_arg *cfa) {
  struct slre_cap caps[100];
  const struct v7_string *s = &cfa->this_obj->v.str;
  int i, n;

  cfa->result->type = V7_TYPE_NULL;
  memset(caps, 0, sizeof(caps));

  if (cfa->num_args == 1 &&
      v7_is_class(cfa->args[0], V7_CLASS_REGEXP) &&
      (n = slre_match(cfa->args[0]->v.regex, s->buf, s->len,
                      caps, ARRAY_SIZE(caps) - 1, 0)) > 0) {
    v7_set_class(cfa->result, V7_CLASS_ARRAY);
    v7_append(cfa->v7, cfa->result,
              v7_mkv(cfa->v7, V7_TYPE_STR, s->buf, (long) n, 1));
    for (i = 0; i < (int) ARRAY_SIZE(caps); i++) {
      if (caps[i].len == 0) break;
      v7_append(cfa->v7, cfa->result,
                v7_mkv(cfa->v7, V7_TYPE_STR, caps[i].ptr, (long) caps[i].len, 1));
    }
  }
  return V7_OK;
}

// Implementation of memmem()
V7_PRIVATE const char *memstr(const char *a, size_t al, const char *b, size_t bl) {
  const char *end;
  if (al == 0 || bl == 0 || al < bl) return NULL;
  for (end = a + (al - bl); a < end; a++) if (!memcmp(a, b, bl)) return a;
  return NULL;
}

V7_PRIVATE enum v7_err Str_split(struct v7_c_func_arg *cfa) {
  const struct v7_string *s = &cfa->this_obj->v.str;
  const char *p1, *p2, *e = s->buf + s->len;
  int limit = cfa->num_args == 2 && cfa->args[1]->type == V7_TYPE_NUM ?
  cfa->args[1]->v.num : -1;
  int num_elems = 0;

  v7_set_class(cfa->result, V7_CLASS_ARRAY);
  if (cfa->num_args == 0) {
    v7_append(cfa->v7, cfa->result,
              v7_mkv(cfa->v7, V7_TYPE_STR, s->buf, s->len, 1));
  } else if (cfa->args[0]->type == V7_TYPE_STR) {
    const struct v7_string *sep = &cfa->args[0]->v.str;
    if (sep->len == 0) {
      // Separator is empty. Split string by characters.
      for (p1 = s->buf; p1 < e; p1++) {
        if (limit >= 0 && limit <= num_elems) break;
        v7_append(cfa->v7, cfa->result, v7_mkv(cfa->v7, V7_TYPE_STR, p1, 1, 1));
        num_elems++;
      }
    } else {
      p1 = s->buf;
      while ((p2 = memstr(p1, e - p1, sep->buf, sep->len)) != NULL) {
        if (limit >= 0 && limit <= num_elems) break;
        v7_append(cfa->v7, cfa->result,
                  v7_mkv(cfa->v7, V7_TYPE_STR, p1, p2 - p1, 1));
        p1 = p2 + sep->len;
        num_elems++;
      }
      if (limit < 0 || limit > num_elems) {
        v7_append(cfa->v7, cfa->result,
                  v7_mkv(cfa->v7, V7_TYPE_STR, p1, e - p1, 1));
      }
    }
  } else if (instanceof(cfa->args[0], &s_constructors[V7_CLASS_REGEXP])) {
    char regex[200];
    struct slre_cap caps[20];
    int n = 0;

    snprintf(regex, sizeof(regex), "(%s)", cfa->args[0]->v.regex);
    p1 = s->buf;
    while ((n = slre_match(regex, p1, e - p1, caps, ARRAY_SIZE(caps), 0)) > 0) {
      if (limit >= 0 && limit <= num_elems) break;
      v7_append(cfa->v7, cfa->result,
                v7_mkv(cfa->v7, V7_TYPE_STR, p1, caps[0].ptr - p1, 1));
      p1 += n;
      num_elems++;
    }
    if (limit < 0 || limit > num_elems) {
      v7_append(cfa->v7, cfa->result,
                v7_mkv(cfa->v7, V7_TYPE_STR, p1, e - p1, 1));
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_indexOf(struct v7_c_func_arg *cfa) {
  v7_init_num(cfa->result, -1.0);
  if (cfa->this_obj->type == V7_TYPE_STR &&
      cfa->num_args > 0 &&
      cfa->args[0]->type == V7_TYPE_STR) {
    int i = cfa->num_args > 1 && cfa->args[1]->type == V7_TYPE_NUM ?
    (int) cfa->args[1]->v.num : 0;
    const struct v7_string *a = &cfa->this_obj->v.str,
    *b = &cfa->args[0]->v.str;

    // Scan the string, advancing one byte at a time
    for (; i >= 0 && a->len >= b->len && i <= (int) (a->len - b->len); i++) {
      if (memcmp(a->buf + i, b->buf, b->len) == 0) {
        cfa->result->v.num = i;
        break;
      }
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_substr(struct v7_c_func_arg *cfa) {
  long start = 0;

  v7_init_str(cfa->result, NULL, 0, 0);
  if (cfa->num_args > 0 && cfa->args[0]->type == V7_TYPE_NUM) {
    start = (long) cfa->args[0]->v.num;
  }
  if (start < 0) {
    start += (long) cfa->this_obj->v.str.len;
  }
  if (start >= 0 && start < (long) cfa->this_obj->v.str.len) {
    long n = cfa->this_obj->v.str.len - start;
    if (cfa->num_args > 1 && cfa->args[1]->type == V7_TYPE_NUM) {
      n = cfa->args[1]->v.num;
    }
    if (n > 0 && n <= ((long) cfa->this_obj->v.str.len - start)) {
      v7_init_str(cfa->result, cfa->this_obj->v.str.buf + start, n, 1);
    }
  }
  return V7_OK;
}

V7_PRIVATE void init_string(void) {
  init_standard_constructor(V7_CLASS_STRING, String_ctor);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_STRING], "length", Str_length);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charCodeAt", Str_charCodeAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charAt", Str_charAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "indexOf", Str_indexOf);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "substr", Str_substr);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "match", Str_match);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "split", Str_split);

  SET_RO_PROP_V(s_global, "String", s_constructors[V7_CLASS_STRING]);
}
