/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

V7_PRIVATE enum v7_err regex_xctor(struct v7 *v7, struct v7_val *obj,
                                   const char *re, size_t re_len,
                                   const char *fl, size_t fl_len) {
  if (NULL == obj) obj = v7_push_new_object(v7);
  v7_init_str(obj, re, re_len, 1);
  v7_set_class(obj, V7_CLASS_REGEXP);
  obj->v.str.prog = NULL;
  obj->fl.fl.re = 1;
  while (fl_len) {
    switch (fl[--fl_len]) {
      case 'g':
        obj->fl.fl.re_flags |= SLRE_FLAG_G;
        break;
      case 'i':
      obj->fl.fl.re_flags |= SLRE_FLAG_I;
        break;
      case 'm':
        obj->fl.fl.re_flags |= SLRE_FLAG_M;
        break;
    }
  }
  obj->v.str.lastIndex = 0;
  return V7_OK;
}

V7_PRIVATE enum v7_err Regex_ctor(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  size_t fl_len = 0;
  const char *fl_start = NULL;
  struct v7_val *re = cfa->args[0], *fl = NULL, *obj = NULL;
  if (cfa->called_as_constructor) obj = cfa->this_obj;

  if (cfa->num_args > 0) {
    TRY(check_str_re_conv(v7, &re, 0));
    if (cfa->num_args > 1) {
      fl = cfa->args[1];
      TRY(check_str_re_conv(v7, &fl, 0));
      fl_len = fl->v.str.len;
      fl_start = fl->v.str.buf;
    }
    regex_xctor(v7, obj, re->v.str.buf, re->v.str.len, fl_start, fl_len);
  }
  return V7_OK;
#undef v7
}

V7_PRIVATE void Regex_global(struct v7_val *this_obj, struct v7_val *arg,
                             struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_bool(result, this_obj->fl.fl.re_flags & SLRE_FLAG_G);
}

V7_PRIVATE void Regex_ignoreCase(struct v7_val *this_obj, struct v7_val *arg,
                                 struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_bool(result, this_obj->fl.fl.re_flags & SLRE_FLAG_I);
}

V7_PRIVATE void Regex_multiline(struct v7_val *this_obj, struct v7_val *arg,
                                struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_bool(result, this_obj->fl.fl.re_flags & SLRE_FLAG_M);
}

V7_PRIVATE void Regex_source(struct v7_val *this_obj, struct v7_val *arg,
                             struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_str(result, this_obj->v.str.buf, this_obj->v.str.len, 1);
}

V7_PRIVATE void Regex_lastIndex(struct v7_val *this_obj, struct v7_val *arg,
                                struct v7_val *result) {
  if (arg)
    this_obj->v.str.lastIndex = arg->v.num;
  else
    v7_init_num(result, this_obj->v.str.lastIndex);
}

V7_PRIVATE enum v7_err regex_check_prog(struct v7_val *re_obj) {
  if (re_obj->v.str.prog == NULL) {
    int res = slre_compile(re_obj->v.str.buf, re_obj->fl.fl.re_flags,
                           &re_obj->v.str.prog);
    if (res != SLRE_OK) {
      return V7_REGEXP_ERROR;
    } else if (re_obj->v.str.prog == NULL) {
      return V7_OUT_OF_MEMORY;
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Regex_exec(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0], *arr = NULL, *t = cfa->this_obj;
  struct slre_loot sub;
  struct slre_cap *ptok = sub.sub;

  if (cfa->num_args > 0) {
    char *begin;
    TRY(check_str_re_conv(v7, &arg, 0));
    begin = arg->v.str.buf;
    if (t->v.str.lastIndex < 0) t->v.str.lastIndex = 0;
    if (t->fl.fl.re_flags & SLRE_FLAG_G) {
      begin = utfnshift(begin, t->v.str.lastIndex);
    }
    TRY(regex_check_prog(t));
    if (!slre_exec(t->v.str.prog, t->fl.fl.re_flags, begin, &sub)) {
      int i;
      arr = v7_push_new_object(v7);
      v7_set_class(arr, V7_CLASS_ARRAY);
      for (i = 0; i < sub.num_captures; i++, ptok++)
        v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, ptok->start,
                                  ptok->end - ptok->start, 1));
      if (t->fl.fl.re_flags & SLRE_FLAG_G)
        t->v.str.lastIndex = utfnlen(begin, sub.sub->end - begin);
      return V7_OK;
    } else {
      t->v.str.lastIndex = 0;
    }
  }
  TRY(v7_make_and_push(v7, V7_TYPE_NULL));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Regex_test(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  TRY(Regex_exec(cfa));

  v7_push_bool(v7, v7_top_val(v7)->type != V7_TYPE_NULL);
  return V7_OK;
#undef v7
}

V7_PRIVATE void init_regex(void) {
  init_standard_constructor(V7_CLASS_REGEXP, Regex_ctor);

  SET_METHOD(s_prototypes[V7_CLASS_REGEXP], "exec", Regex_exec);
  SET_METHOD(s_prototypes[V7_CLASS_REGEXP], "test", Regex_test);

  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "global", Regex_global);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "ignoreCase", Regex_ignoreCase);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "multiline", Regex_multiline);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "source", Regex_source);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "lastIndex", Regex_lastIndex);

  SET_RO_PROP_V(s_global, "RegExp", s_constructors[V7_CLASS_REGEXP]);
}
