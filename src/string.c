/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

V7_PRIVATE val_t to_string(struct v7 *, val_t);

static val_t String_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  val_t res = v7_is_string(arg0) ? arg0 : (v7_is_undefined(arg0)
                                               ? v7_create_string(v7, "", 0, 1)
                                               : to_string(v7, arg0));

  if (v7_is_object(this_obj) && this_obj != v7->global_object) {
    v7_to_object(this_obj)->prototype = v7_to_object(v7->string_prototype);
    v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, res);
    return this_obj;
  }

  return res;
}

static val_t Str_fromCharCode(struct v7 *v7, val_t this_obj, val_t args) {
  int i, num_args = v7_array_length(v7, args);
  val_t res = v7_create_string(v7, "", 0, 1); /* Empty string */

  (void)this_obj;
  for (i = 0; i < num_args; i++) {
    char buf[10];
    val_t arg = v7_array_at(v7, args, i);
    Rune r = (Rune)v7_to_double(arg);
    int n = runetochar(buf, &r);
    val_t s = v7_create_string(v7, buf, n, 1);
    res = s_concat(v7, res, s);
  }

  return res;
}

static double s_charCodeAt(struct v7 *v7, val_t this_obj, val_t args) {
  size_t n;
  val_t s = to_string(v7, this_obj);
  const char *p = v7_to_string(v7, &s, &n);
  val_t arg = v7_array_at(v7, args, 0);
  double at = v7_to_double(arg);

  n = utfnlen((char *)p, n);
  if (v7_is_double(arg) && at >= 0 && at < n) {
    Rune r = 0;
    p = utfnshift((char *)p, at);
    chartorune(&r, (char *)p);
    return r;
  }
  return NAN;
}

static val_t Str_charCodeAt(struct v7 *v7, val_t this_obj, val_t args) {
  return v7_create_number(s_charCodeAt(v7, this_obj, args));
}

static val_t Str_charAt(struct v7 *v7, val_t this_obj, val_t args) {
  double code = s_charCodeAt(v7, this_obj, args);
  char buf[10] = {0};
  int len = 0;

  if (code != NAN) {
    Rune r = (Rune)code;
    len = runetochar(buf, &r);
  }
  return v7_create_string(v7, buf, len, 1);
}

static val_t Str_concat(struct v7 *v7, val_t this_obj, val_t args) {
  val_t res = to_string(v7, this_obj);
  int i, num_args = v7_array_length(v7, args);

  for (i = 0; i < num_args; i++) {
    val_t str = to_string(v7, v7_array_at(v7, args, i));
    res = s_concat(v7, res, str);
  }

  return res;
}

static val_t s_index_of(struct v7 *v7, val_t this_obj, val_t args, int last) {
  val_t s = to_string(v7, this_obj);
  val_t arg0 = v7_array_at(v7, args, 0);
  val_t arg1 = i_value_of(v7, v7_array_at(v7, args, 1));
  val_t sub;
  size_t i, n1, n2, fromIndex = 0;
  double res = -1;
  const char *p1, *p2, *end;

  if (arg0 != V7_UNDEFINED) {
    sub = to_string(v7, arg0);
    p1 = v7_to_string(v7, &s, &n1);
    p2 = v7_to_string(v7, &sub, &n2);

    if (v7_array_length(v7, args) > 1) fromIndex = v7_to_double(arg1);
    end = p1 + n1;
    if (fromIndex > 0) {
      if (last)
        end = utfnshift((char *)p1, fromIndex + 1);
      else
        p1 = utfnshift((char *)p1, fromIndex);
    }
    if (0 == n2 || end - p1 == 0)
      res = 0;
    else {
      for (i = 0; p1 <= (end - n2); i++, p1 = utfnshift((char *)p1, 1))
        if (memcmp(p1, p2, n2) == 0) {
          res = i;
          if (!last) break;
        }
    }
  }
  if (!last && res >= 0) res += fromIndex;
  return v7_create_number(res);
}

static val_t Str_valueOf(struct v7 *v7, val_t this_obj, val_t args) {
  if (!v7_is_string(this_obj) &&
      (v7_is_object(this_obj) &&
       v7_object_to_value(v7_to_object(this_obj)->prototype) !=
           v7->string_prototype)) {
    throw_exception(v7, "TypeError",
                    "String.valueOf called on non-string object");
  }
  return Obj_valueOf(v7, this_obj, args);
}

static val_t Str_indexOf(struct v7 *v7, val_t this_obj, val_t args) {
  return s_index_of(v7, this_obj, args, 0);
}

static val_t Str_lastIndexOf(struct v7 *v7, val_t this_obj, val_t args) {
  return s_index_of(v7, this_obj, args, 1);
}

static val_t Str_localeCompare(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = to_string(v7, v7_array_at(v7, args, 0));
  val_t s = to_string(v7, this_obj);

  return v7_create_number(s_cmp(v7, s, arg0));
}

static val_t Str_toString(struct v7 *v7, val_t this_obj, val_t args) {
  (void)args;

  if (this_obj == v7->string_prototype) {
    return v7_create_string(v7, "false", 5, 1);
  }

  if (!v7_is_string(this_obj) &&
      !(v7_is_object(this_obj) &&
        is_prototype_of(v7, this_obj, v7->string_prototype))) {
    throw_exception(v7, "TypeError",
                    "String.toString called on non-string object");
  }

  return to_string(v7, i_value_of(v7, this_obj));
}

static val_t Str_match(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arr = V7_NULL;

  if (v7_array_length(v7, args) > 0) {
    size_t s_len;
    struct slre_prog *prog = NULL;
    val_t so, ro = i_value_of(v7, v7_array_at(v7, args, 0));
    const char *s, *end;
    int flag_g;
    if (!v7_is_regexp(ro)) {
      so = to_string(v7, ro);
      s = v7_to_string(v7, &so, &s_len);
      if (slre_compile(s, s_len, NULL, 0, &prog, 0) != SLRE_OK ||
          prog == NULL) {
        throw_exception(v7, "Error", "Invalid String");
        return V7_UNDEFINED;
      }
    } else {
      struct v7_regexp *rp = v7_to_pointer(ro);
      prog = rp->compiled_regexp;
    }
    flag_g = slre_get_flags(prog) & SLRE_FLAG_G;
    so = to_string(v7, this_obj);
    s = v7_to_string(v7, &so, &s_len);
    end = s + s_len;

    do {
      struct slre_loot sub;
      struct slre_cap *ptok = sub.caps;
      int i;
      if (slre_exec(prog, 0, s, end, &sub)) break;
      if (arr == V7_NULL) arr = v7_create_array(v7);
      s = ptok->end;
      i = 0;
      do {
        v7_array_append(v7, arr, v7_create_string(v7, ptok->start,
                                                  ptok->end - ptok->start, 1));
        ptok++;
      } while (!flag_g && ++i < sub.num_captures);
    } while (flag_g && s < end);
  }
  return arr;
}

static val_t Str_replace(struct v7 *v7, val_t this_obj, val_t args) {
  const char *s;
  size_t s_len;
  val_t out_str_o;
  this_obj = to_string(v7, this_obj);
  s = v7_to_string(v7, &this_obj, &s_len);

  if (v7_array_length(v7, args) > 1) {
    const char *const str_end = s + s_len;
    char *p = (char *)s;
    uint32_t out_sub_num = 0;
    val_t ro = i_value_of(v7, v7_array_at(v7, args, 0)), str_func = i_value_of(v7, v7_array_at(v7, args, 1));
    struct slre_prog *prog = NULL;
    struct slre_cap out_sub[V7_RE_MAX_REPL_SUB], *ptok = out_sub;
    struct slre_loot loot;
    int flag_g = 0;

    if (!v7_is_regexp(ro)) {
      const char *str;
      size_t str_len;
      ro = to_string(v7, ro);
      str = v7_to_string(v7, &ro, &str_len);
      if (slre_compile(str, str_len, NULL, 0, &prog, 0) != SLRE_OK ||
          prog == NULL) {
        throw_exception(v7, "Error", "Invalid String");
        return V7_UNDEFINED;
      }
    } else {
      struct v7_regexp *rp = v7_to_pointer(ro);
      prog = rp->compiled_regexp;
      flag_g = slre_get_flags(prog) & SLRE_FLAG_G;
    }

    if (!v7_is_function(str_func))
      str_func = to_string(v7, str_func);

    do {
      int i;
      if (slre_exec(prog, 0, p, str_end, &loot)) break;
      if (p != loot.caps->start) {
        ptok->start = p;
        ptok->end = loot.caps->start;
        ptok++;
        out_sub_num++;
      }

      if (v7_is_function(str_func)) { /* replace function */
        const char *rez_str;
        size_t rez_len;
        val_t arr = v7_create_array(v7);

        for (i = 0; i < loot.num_captures; i++)
          v7_array_append(
              v7, arr,
              v7_create_string(v7, loot.caps[i].start,
                               loot.caps[i].end - loot.caps[i].start, 1));
        v7_array_append(v7, arr, v7_create_number(utfnlen(
                                     (char *)s, loot.caps[0].start - s)));
        v7_array_append(v7, arr, this_obj);
        out_str_o = to_string(v7, v7_apply(v7, str_func, this_obj, arr));
        rez_str = v7_to_string(v7, &out_str_o, &rez_len);
        if (rez_len) {
          ptok->start = rez_str;
          ptok->end = rez_str + rez_len;
          ptok++;
          out_sub_num++;
        }
      } else { /* replace string */
        struct slre_loot newsub;
        size_t f_len;
        const char *f_str = v7_to_string(v7, &str_func, &f_len);
        slre_replace(&loot, s, s_len, f_str, f_len, &newsub);
        for (i = 0; i < newsub.num_captures; i++) {
          ptok->start = newsub.caps[i].start;
          ptok->end = newsub.caps[i].end;
          ptok++;
          out_sub_num++;
        }
      }
      p = (char *)loot.caps[0].end;
    } while (flag_g && p < str_end);
    if (p < str_end) {
      ptok->start = p;
      ptok->end = str_end;
      ptok++;
      out_sub_num++;
    }
    out_str_o = v7_create_string(v7, NULL, 0, 1);
    ptok = out_sub;
    do {
      size_t ln = ptok->end - ptok->start;
      out_str_o =
          s_concat(v7, out_str_o, v7_create_string(v7, ptok->start, ln, 1));
      p += ln;
      ptok++;
    } while (--out_sub_num);
    *p = '\0';

    return out_str_o;
  }
  return v7_create_string(v7, s, s_len, 1);
}

static val_t Str_search(struct v7 *v7, val_t this_obj, val_t args) {
  long utf_shift = -1;

  if (v7_array_length(v7, args) > 0) {
    size_t s_len;
    struct slre_prog *prog = NULL;
    struct slre_loot sub;
    val_t so, ro = i_value_of(v7, v7_array_at(v7, args, 0));
    const char *s;
    if (!v7_is_regexp(ro)) {
      so = to_string(v7, ro);
      s = v7_to_string(v7, &so, &s_len);
      if (slre_compile(s, s_len, NULL, 0, &prog, 0) != SLRE_OK ||
          prog == NULL) {
        throw_exception(v7, "Error", "Invalid String");
        return V7_UNDEFINED;
      }
    } else {
      struct v7_regexp *rp = v7_to_pointer(ro);
      prog = rp->compiled_regexp;
    }
    so = to_string(v7, this_obj);
    s = v7_to_string(v7, &so, &s_len);

    if (!slre_exec(prog, 0, s, s + s_len, &sub))
      utf_shift =
          utfnlen((char *)s, sub.caps[0].start - s); /* calc shift for UTF-8 */
  } else
    utf_shift = 0;
  return v7_create_number(utf_shift);
}

static val_t Str_slice(struct v7 *v7, val_t this_obj, val_t args) {
  long from = 0, to = 0;
  size_t len;
  val_t so = to_string(v7, this_obj);
  const char *begin = v7_to_string(v7, &so, &len), *end;
  int num_args = v7_array_length(v7, args);

  to = len = utfnlen((char *)begin, len);
  if (num_args > 0) {
    from = arg_long(v7, args, 0, 0);
    if (from < 0) {
      from += len;
      if (from < 0) from = 0;
    } else if ((size_t)from > len)
      from = len;
    if (num_args > 1) {
      to = arg_long(v7, args, 1, 0);
      if (to < 0) {
        to += len;
        if (to < 0) to = 0;
      } else if ((size_t)to > len)
        to = len;
    }
  }
  if (from > to) to = from;
  end = utfnshift((char *)begin, to);
  begin = utfnshift((char *)begin, from);
  return v7_create_string(v7, begin, end - begin, 1);
}

static val_t s_transform(struct v7 *v7, val_t this_obj, val_t args,
                         Rune (*func)(Rune)) {
  val_t s = to_string(v7, this_obj);
  size_t i, n, len;
  const char *p = v7_to_string(v7, &s, &len);
  val_t res = v7_create_string(v7, p, len, 1);
  Rune r;

  (void)args;

  p = v7_to_string(v7, &res, &len);
  for (i = 0; i < len; i += n) {
    n = chartorune(&r, p + i);
    r = func(r);
    runetochar((char *)p + i, &r);
  }

  return res;
}

static val_t Str_toLowerCase(struct v7 *v7, val_t this_obj, val_t args) {
  return s_transform(v7, this_obj, args, tolowerrune);
}

static val_t Str_toUpperCase(struct v7 *v7, val_t this_obj, val_t args) {
  return s_transform(v7, this_obj, args, toupperrune);
}

static int s_isspace(Rune c) { return isspacerune(c) || isnewline(c); }

static val_t Str_trim(struct v7 *v7, val_t this_obj, val_t args) {
  val_t s = to_string(v7, this_obj);
  size_t i, n, len, start = 0, end, state = 0;
  const char *p = v7_to_string(v7, &s, &len);
  Rune r;

  (void)args;
  end = len;
  for (i = 0; i < len; i += n) {
    n = chartorune(&r, p + i);
    if (!s_isspace(r)) {
      if (state++ == 0) start = i;
      end = i + n;
    }
  }

  return v7_create_string(v7, p + start, end - start, 1);
}

static val_t Str_length(struct v7 *v7, val_t this_obj, val_t args) {
  size_t len = 0;
  val_t s = i_value_of(v7, this_obj);

  (void)args;
  if (v7_is_string(s)) {
    const char *p = v7_to_string(v7, &s, &len);
    len = utfnlen((char *)p, len);
  }

  return v7_create_number(len);
}

V7_PRIVATE long arg_long(struct v7 *v7, val_t args, int n, long default_value) {
  char buf[40];
  size_t l;
  val_t arg_n = i_value_of(v7, v7_array_at(v7, args, n));
  double d;
  if (v7_is_double(arg_n)) {
    d = v7_to_double(arg_n);
    if (isnan(d) || (isinf(d) && d < 0)) {
      return 0;
    } else if (d > LONG_MAX) {
      return LONG_MAX;
    }
    return (long)d;
  }
  if (arg_n == V7_NULL) return 0;
  l = to_str(v7, arg_n, buf, sizeof(buf), 0);
  if (l > 0 && isdigit(buf[0])) return strtol(buf, NULL, 10);
  return default_value;
}

static val_t Str_substr(struct v7 *v7, val_t this_obj, val_t args) {
  long start = arg_long(v7, args, 0, 0);
  long len = arg_long(v7, args, 1, LONG_MAX);
  return s_substr(v7, this_obj, start, len);
}

static val_t Str_substring(struct v7 *v7, val_t this_obj, val_t args) {
  long start = arg_long(v7, args, 0, 0);
  long end = arg_long(v7, args, 1, LONG_MAX);
  return s_substr(v7, this_obj, start, end - start);
}

static val_t Str_split(struct v7 *v7, val_t this_obj, val_t args) {
  val_t res = v7_create_array(v7);
  const char *s, *s_end;
  size_t s_len;
  long num_args = v7_array_length(v7, args);
  struct slre_prog *prog = NULL;
  this_obj = to_string(v7, this_obj);
  s = v7_to_string(v7, &this_obj, &s_len);
  s_end = s + s_len;

  if (num_args == 0 || s_len == 0) {
    v7_array_append(v7, res, v7_create_string(v7, s, s_len, 1));
  } else {
    val_t ro = i_value_of(v7, v7_array_at(v7, args, 0));
    long len, elem = 0, limit = arg_long(v7, args, 1, LONG_MAX);
    size_t shift = 0;
    struct slre_loot loot;
    if (!v7_is_regexp(ro)) {
      const char *str;
      size_t str_len;
      ro = to_string(v7, ro);
      str = v7_to_string(v7, &ro, &str_len);
      if (slre_compile(str, str_len, NULL, 0, &prog, 0) != SLRE_OK ||
          prog == NULL) {
        throw_exception(v7, "Error", "Invalid String");
        return V7_UNDEFINED;
      }
    } else {
      struct v7_regexp *rp = v7_to_pointer(ro);
      prog = rp->compiled_regexp;
    }
    for (; elem < limit && shift < s_len; elem++) {
      val_t tmp_s;
      int i;
      if (slre_exec(prog, 0, s + shift, s_end, &loot)) break;
      if (loot.caps[0].end - loot.caps[0].start == 0) {
        tmp_s = v7_create_string(v7, s + shift, 1, 1);
        shift++;
      } else {
        tmp_s =
            v7_create_string(v7, s + shift, loot.caps[0].start - s - shift, 1);
        shift = loot.caps[0].end - s;
      }
      v7_array_append(v7, res, tmp_s);

      for (i = 1; i < loot.num_captures; i++)
        v7_array_append(
            v7, res,
            (loot.caps[i].start != NULL)
                ? v7_create_string(v7, loot.caps[i].start,
                                   loot.caps[i].end - loot.caps[i].start, 1)
                : V7_UNDEFINED);
    }
    len = s_len - shift;
    v7_array_append(v7, res, v7_create_string(v7, s + shift, len, 1));
  }

  return res;
}

V7_PRIVATE void init_string(struct v7 *v7) {
  val_t str = v7_create_cfunction_ctor(v7, v7->string_prototype, String_ctor, 1);
  v7_set_property(v7, v7->global_object, "String", 6, V7_PROPERTY_DONT_ENUM,
                  str);

  set_cfunc_prop(v7, v7->string_prototype, "charCodeAt", Str_charCodeAt);
  set_cfunc_prop(v7, v7->string_prototype, "charAt", Str_charAt);
  set_cfunc_prop(v7, v7->string_prototype, "fromCharCode", Str_fromCharCode);
  set_cfunc_prop(v7, v7->string_prototype, "concat", Str_concat);
  set_cfunc_prop(v7, v7->string_prototype, "indexOf", Str_indexOf);
  set_cfunc_prop(v7, v7->string_prototype, "substr", Str_substr);
  set_cfunc_prop(v7, v7->string_prototype, "substring", Str_substring);
  set_cfunc_prop(v7, v7->string_prototype, "valueOf", Str_valueOf);
  set_cfunc_prop(v7, v7->string_prototype, "lastIndexOf", Str_lastIndexOf);
  set_cfunc_prop(v7, v7->string_prototype, "localeCompare", Str_localeCompare);
  set_cfunc_prop(v7, v7->string_prototype, "match", Str_match);
  set_cfunc_prop(v7, v7->string_prototype, "replace", Str_replace);
  set_cfunc_prop(v7, v7->string_prototype, "search", Str_search);
  set_cfunc_prop(v7, v7->string_prototype, "slice", Str_slice);
  set_cfunc_prop(v7, v7->string_prototype, "trim", Str_trim);
  set_cfunc_prop(v7, v7->string_prototype, "toLowerCase", Str_toLowerCase);
  set_cfunc_prop(v7, v7->string_prototype, "toLocaleLowerCase",
                 Str_toLowerCase);
  set_cfunc_prop(v7, v7->string_prototype, "toUpperCase", Str_toUpperCase);
  set_cfunc_prop(v7, v7->string_prototype, "toLocaleUpperCase",
                 Str_toUpperCase);
  set_cfunc_prop(v7, v7->string_prototype, "split", Str_split);
  set_cfunc_prop(v7, v7->string_prototype, "toString", Str_toString);

  v7_set_property(v7, v7->string_prototype, "length", 6, V7_PROPERTY_GETTER,
                  v7_create_cfunction(Str_length));
}
