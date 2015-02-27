/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

V7_PRIVATE val_t to_string(struct v7 *, val_t);

static val_t Regex_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  long argnum = v7_array_length(v7, args);
  if (argnum > 0) {
    val_t ro = to_string(v7, v7_array_at(v7, args, 0));
    size_t re_len, flags_len = 0;
    const char *re = v7_to_string(v7, &ro, &re_len), *flags = NULL;
    struct slre_prog *p = NULL;
    struct v7_regexp *rp;

    (void)this_obj;
    if (argnum > 1) {
      val_t fl = to_string(v7, v7_array_at(v7, args, 1));
      flags = v7_to_string(v7, &fl, &flags_len);
    }
    if (slre_compile(re, re_len, flags, flags_len, &p, 1) != SLRE_OK ||
        p == NULL) {
      throw_exception(v7, "Error", "Invalid regex");
      return V7_UNDEFINED;
    } else {
      rp = (struct v7_regexp *)malloc(sizeof(*rp));
      rp->regexp_string = v7_create_string(v7, re, re_len, 1);
      rp->compiled_regexp = p;
      rp->lastIndex = 0;

      return v7_pointer_to_value(rp) | V7_TAG_REGEXP;
    }
  }
  return v7_create_regexp(v7, "(?:)", 4, NULL, 0);
}

static val_t Regex_global(struct v7 *v7, val_t this_obj, val_t args) {
  int flags = 0;
  val_t r = i_value_of(v7, this_obj);

  (void)args;
  if (v7_is_regexp(r)) {
    struct v7_regexp *rp = v7_to_pointer(r);
    flags = slre_get_flags(rp->compiled_regexp);
  }
  return v7_create_boolean(flags & SLRE_FLAG_G);
}

static val_t Regex_ignoreCase(struct v7 *v7, val_t this_obj, val_t args) {
  int flags = 0;
  val_t r = i_value_of(v7, this_obj);

  (void)args;
  if (v7_is_regexp(r)) {
    struct v7_regexp *rp = v7_to_pointer(r);
    flags = slre_get_flags(rp->compiled_regexp);
  }
  return v7_create_boolean(flags & SLRE_FLAG_I);
}

static val_t Regex_multiline(struct v7 *v7, val_t this_obj, val_t args) {
  int flags = 0;
  val_t r = i_value_of(v7, this_obj);

  (void)args;
  if (v7_is_regexp(r)) {
    struct v7_regexp *rp = v7_to_pointer(r);
    flags = slre_get_flags(rp->compiled_regexp);
  }
  return v7_create_boolean(flags & SLRE_FLAG_M);
}

static val_t Regex_source(struct v7 *v7, val_t this_obj, val_t args) {
  val_t r = i_value_of(v7, this_obj);
  const char *buf = 0;
  size_t len = 0;

  (void)args;
  if (v7_is_regexp(r)) {
    struct v7_regexp *rp = v7_to_pointer(r);
    buf = v7_to_string(v7, &rp->regexp_string, &len);
  }
  return v7_create_string(v7, buf, len, 1);
}

static val_t Regex_get_lastIndex(struct v7 *v7, val_t this_obj, val_t args) {
  long lastIndex = 0;

  (void)v7;
  (void)args;
  if (v7_is_regexp(this_obj)) {
    struct v7_regexp *rp = v7_to_pointer(this_obj);
    lastIndex = rp->lastIndex;
  }
  return v7_create_number(lastIndex);
}

static val_t Regex_set_lastIndex(struct v7 *v7, val_t this_obj, val_t args) {
  long lastIndex = 0;

  if (v7_is_regexp(this_obj)) {
    struct v7_regexp *rp = v7_to_pointer(this_obj);
    rp->lastIndex = lastIndex = arg_long(v7, args, 0, 0);
  }
  return v7_create_number(lastIndex);
}

static val_t Regex_exec(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arr = V7_NULL;
  if (v7_is_regexp(this_obj) && v7_array_length(v7, args) > 0) {
    val_t s = to_string(v7, v7_array_at(v7, args, 0));
    size_t len;
    struct slre_loot sub;
    struct slre_cap *ptok = sub.caps;
    const char *begin = v7_to_string(v7, &s, &len);
    struct v7_regexp *rp = v7_to_pointer(this_obj);
    int flag_g = slre_get_flags(rp->compiled_regexp) & SLRE_FLAG_G;
    if (rp->lastIndex < 0) rp->lastIndex = 0;
    if (flag_g) begin = utfnshift((char *)begin, rp->lastIndex);

    if (!slre_exec(rp->compiled_regexp, 0, begin, len, &sub)) {
      int i;
      arr = v7_create_array(v7);

      for (i = 0; i < sub.num_captures; i++, ptok++)
        v7_array_append(v7, arr, v7_create_string(v7, ptok->start,
                                                  ptok->end - ptok->start, 1));
      if (flag_g) rp->lastIndex = utfnlen((char *)begin, sub.caps->end - begin);
    } else
      rp->lastIndex = 0;
  }
  return arr;
}

static val_t Regex_test(struct v7 *v7, val_t this_obj, val_t args) {
  return v7_create_boolean(Regex_exec(v7, this_obj, args) != V7_NULL);
}

V7_PRIVATE void init_regex(struct v7 *v7) {
  val_t ctor = v7_create_cfunction(Regex_ctor);
  val_t lastIndex = v7_create_array(v7);

  v7_set_property(v7, v7->global_object, "RegExp", 6, 0, ctor);
  v7_set(v7, v7->regexp_prototype, "constructor", 11, ctor);

  set_cfunc_prop(v7, v7->regexp_prototype, "exec", Regex_exec);
  set_cfunc_prop(v7, v7->regexp_prototype, "test", Regex_test);

  v7_set_property(v7, v7->regexp_prototype, "global", 6, V7_PROPERTY_GETTER,
                  v7_create_cfunction(Regex_global));
  v7_set_property(v7, v7->regexp_prototype, "ignoreCase", 10,
                  V7_PROPERTY_GETTER, v7_create_cfunction(Regex_ignoreCase));
  v7_set_property(v7, v7->regexp_prototype, "multiline", 9, V7_PROPERTY_GETTER,
                  v7_create_cfunction(Regex_multiline));
  v7_set_property(v7, v7->regexp_prototype, "source", 6, V7_PROPERTY_GETTER,
                  v7_create_cfunction(Regex_source));

  v7_set(v7, lastIndex, "0", 1, v7_create_cfunction(Regex_get_lastIndex));
  v7_set(v7, lastIndex, "1", 1, v7_create_cfunction(Regex_set_lastIndex));
  v7_set_property(v7, v7->regexp_prototype, "lastIndex", 9,
                  V7_PROPERTY_GETTER | V7_PROPERTY_SETTER, lastIndex);
}
