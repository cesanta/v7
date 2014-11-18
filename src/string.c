#include "internal.h"

V7_PRIVATE enum v7_err String_ctor(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  const char *str = NULL;
  size_t len = 0;
  int own = 0;
  struct v7_val *obj = cfa->this_obj;

  if (cfa->num_args > 0) {
    struct v7_val *arg = cfa->args[0];
    TRY(check_str_re_conv(v7, &arg, 0));
    str = arg->v.str.buf;
    len = arg->v.str.len;
    own = 1;
  }
  if (cfa->called_as_constructor) {
    v7_init_str(obj, str, len, own);
    v7_set_class(obj, V7_CLASS_STRING);
  }
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_fromCharCode(struct v7_c_func_arg *cfa) {
  long n, blen = 0;
  struct v7_val *str;
  char *p;
  Rune runes[500];
  for (n = 0; n < cfa->num_args; n++) {
    if (!is_num(cfa->args[n])) {
      // TODO(vrz) type conversion
    }
    blen += runelen((Rune)cfa->args[n]->v.num);
    runes[n] = (Rune)cfa->args[n]->v.num;
  }
  str = v7_push_string(cfa->v7, NULL, blen, 1);
  p = str->v.str.buf;
  for (n = 0; n < cfa->num_args; n++) p += runetochar(p, &runes[n]);
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_valueOf(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  if (!is_string(cfa->this_obj)) THROW(V7_TYPE_ERROR);
  TRY(push_string(v7, cfa->this_obj->v.str.buf, cfa->this_obj->v.str.len, 1));
  return V7_OK;
#undef v7
}

static enum v7_err _charAt(struct v7_c_func_arg *cfa, const char **p) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  if (cfa->num_args > 0) {
    long len = utfnlen(cfa->this_obj->v.str.buf, cfa->this_obj->v.str.len),
         idx = cfa->args[0]->v.num;
    if (!is_num(cfa->args[0])) {
      // TODO(vrz) type conversion
    }
    if (idx < 0) idx = len - idx;
    if (idx >= 0 && idx < len)
      return *p = utfnshift(cfa->this_obj->v.str.buf, idx), V7_OK;
  }
  return *p = NULL, V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_charAt(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  const char *p;
  TRY(_charAt(cfa, &p));
  TRY(push_string(v7, p, p == NULL ? 0 : 1, 1));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_charCodeAt(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  const char *p;
  Rune rune;
  TRY(_charAt(cfa, &p));
  TRY(push_number(v7, p == NULL ? NAN : (chartorune(&rune, p), rune)));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_concat(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long n, blen;
  struct v7_val *str;
  char *p;
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  blen = cfa->this_obj->v.str.len;
  for (n = 0; n < cfa->num_args; n++) {
    TRY(check_str_re_conv(v7, &cfa->args[n], 0));
    blen += cfa->args[n]->v.str.len;
  }
  str = v7_push_string(v7, cfa->this_obj->v.str.buf, blen, 1);
  p = str->v.str.buf + cfa->this_obj->v.str.len;
  for (n = 0; n < cfa->num_args; n++) {
    memcpy(p, cfa->args[n]->v.str.buf, cfa->args[n]->v.str.len);
    p += cfa->args[n]->v.str.len;
  }
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
#undef v7
}

static long _indexOf(char *pp, char *const end, char *p, long blen,
                     uint8_t last) {
  long i, idx = -1;
  for (i = 0; pp <= (end - blen); i++, pp = utfnshift(pp, 1))
    if (0 == memcmp(pp, p, blen)) {
      idx = i;
      if (!last) break;
    }
  return idx;
}

V7_PRIVATE enum v7_err Str_indexOf(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long idx = -1, pos = 0;
  char *p, *end;
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  if (cfa->num_args > 0) {
    TRY(check_str_re_conv(v7, &cfa->args[0], 0));
    if (cfa->num_args > 1) {
      if (!is_num(cfa->args[1])) {
        // TODO(vrz) type conversion
      }
      // TODO(vrz)
      p = utfnshift(p, pos = cfa->args[1]->v.num);
    }
    idx = _indexOf(p, end, cfa->args[0]->v.str.buf, cfa->args[0]->v.str.len, 0);
  }
  if (idx >= 0) idx += pos;
  TRY(push_number(v7, idx));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_lastIndexOf(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long idx = -1;
  char *p, *end;
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  if (cfa->num_args > 0) {
    TRY(check_str_re_conv(v7, &cfa->args[0], 0));
    if (cfa->num_args > 1) {
      if (!is_num(cfa->args[1])) {
        // TODO(vrz) type conversion
      }
      // TODO(vrz)
      end = utfnshift(p, cfa->args[1]->v.num + 1);
    }
    idx = _indexOf(p, end, cfa->args[0]->v.str.buf, cfa->args[0]->v.str.len, 1);
  }
  TRY(push_number(v7, idx));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_localeCompare(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0];
  long i, ln = 0, ret = 0;
  Rune s, t;
  char *ps, *pt, *end;
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  TRY(check_str_re_conv(v7, &arg, 0));
  ps = cfa->this_obj->v.str.buf;
  pt = arg->v.str.buf;
  end = ps + cfa->this_obj->v.str.len;
  if (arg->v.str.len < cfa->this_obj->v.str.len) {
    end = ps + arg->v.str.len;
    ln = 1;
  } else if (arg->v.str.len > cfa->this_obj->v.str.len) {
    ln = -1;
  }
  for (i = 0; ps < end; i++) {
    ps += chartorune(&s, ps);
    pt += chartorune(&t, pt);
    if (s < t) {
      ret = -1;
      break;
    }
    if (s > t) {
      ret = 1;
      break;
    }
  }
  if (0 == ret) ret = ln;
  TRY(push_number(v7, ret));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_match(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0];
  struct Resub sub;
  struct v7_val *arr = NULL;
  unsigned long shift = 0;

  if (cfa->num_args > 0) {
    TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
    TRY(check_str_re_conv(v7, &arg, 1));
    TRY(regex_check_prog(arg));
    do {
      if (!re_exec(arg->v.str.prog, arg->fl.fl,
                   cfa->this_obj->v.str.buf + shift, &sub)) {
        if (NULL == arr) {
          arr = v7_push_new_object(v7);
          v7_set_class(arr, V7_CLASS_ARRAY);
        }
        shift = sub.sub[0].end - cfa->this_obj->v.str.buf;
        v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, sub.sub[0].start,
                                  sub.sub[0].end - sub.sub[0].start, 1));
      }
    } while (arg->fl.fl.re_g && shift < cfa->this_obj->v.str.len);
  }
  if (0 == shift) TRY(v7_make_and_push(v7, V7_TYPE_NULL));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_replace(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *result = v7_push_new_object(v7);
  const char *out_str;
  uint8_t own = 1;
  size_t out_len;
  int old_sp = v7->sp;

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  out_str = cfa->this_obj->v.str.buf;
  out_len = cfa->this_obj->v.str.len;
  if (cfa->num_args > 1) {
    const char *const str_end =
        cfa->this_obj->v.str.buf + cfa->this_obj->v.str.len;
    char *p = cfa->this_obj->v.str.buf;
    uint32_t out_sub_num = 0;
    struct v7_val *re = cfa->args[0], *str_func = cfa->args[1], *arr = NULL;
    struct re_tok out_sub[V7_RE_MAX_REPL_SUB], *ptok = out_sub;
    struct Resub loot;
    TRY(check_str_re_conv(v7, &re, 1));
    TRY(regex_check_prog(re));
    if (v7_is_class(str_func, V7_CLASS_FUNCTION)) {
      arr = v7_push_new_object(v7);
      v7_set_class(arr, V7_CLASS_ARRAY);
      TRY(v7_push(v7, str_func));
    } else
      TRY(check_str_re_conv(v7, &str_func, 0));

    out_len = 0;
    do {
      int i;
      if (re_exec(re->v.str.prog, re->fl.fl, p, &loot)) break;
      if (p != loot.sub->start) {
        ptok->start = p;
        ptok->end = loot.sub->start;
        ptok++;
        out_len += loot.sub->start - p;
        out_sub_num++;
      }

      if (NULL != arr) { /* replace function */
        int old_sp = v7->sp;
        struct v7_val *rez_str;
        for (i = 0; i < loot.subexpr_num; i++)
          TRY(push_string(v7, loot.sub[i].start,
                          loot.sub[i].end - loot.sub[i].start, 1));
        TRY(push_number(v7, utfnlen(p, loot.sub[0].start - p)));
        TRY(v7_push(v7, cfa->this_obj));
        rez_str = v7_call(v7, cfa->this_obj, loot.subexpr_num + 2);
        TRY(check_str_re_conv(v7, &rez_str, 0));
        if (rez_str->v.str.len) {
          ptok->start = rez_str->v.str.buf;
          ptok->end = rez_str->v.str.buf + rez_str->v.str.len;
          ptok++;
          out_len += rez_str->v.str.len;
          out_sub_num++;
          v7_append(v7, arr, rez_str);
        }
        TRY(inc_stack(v7, old_sp - v7->sp));
      } else { /* replace string */
        struct Resub newsub;
        re_rplc(&loot, p, str_func->v.str.buf, &newsub);
        for (i = 0; i < newsub.subexpr_num; i++) {
          ptok->start = newsub.sub[i].start;
          ptok->end = newsub.sub[i].end;
          ptok++;
          out_len += newsub.sub[i].end - newsub.sub[i].start;
          out_sub_num++;
        }
      }
      p = (char *)loot.sub->end;
    } while (re->fl.fl.re_g && p < str_end);
    if (p < str_end) {
      ptok->start = p;
      ptok->end = str_end;
      ptok++;
      out_len += str_end - p;
      out_sub_num++;
    }
    out_str = malloc(out_len + 1);
    CHECK(out_str, V7_OUT_OF_MEMORY);
    ptok = out_sub;
    p = (char *)out_str;
    do {
      size_t ln = ptok->end - ptok->start;
      memcpy(p, ptok->start, ln);
      p += ln;
      ptok++;
    } while (--out_sub_num);
    *p = '\0';
    own = 0;
  }
  TRY(inc_stack(v7, old_sp - v7->sp));
  v7_init_str(result, out_str, out_len, own);
  result->fl.fl.str_alloc = 1;
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_search(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0];
  struct Resub sub;
  int shift = -1, utf_shift = -1;

  if (cfa->num_args > 0) {
    TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
    TRY(check_str_re_conv(v7, &arg, 1));
    TRY(regex_check_prog(arg));
    if (!re_exec(arg->v.str.prog, arg->fl.fl, cfa->this_obj->v.str.buf, &sub))
      shift = sub.sub[0].start - cfa->this_obj->v.str.buf;
  }
  if (shift > 0) /* calc shift for UTF-8 */
    utf_shift = utfnlen(cfa->this_obj->v.str.buf, shift);
  TRY(push_number(v7, utf_shift));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_slice(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  char *begin, *end;
  long from = 0, to = 0, len;

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  to = len = utfnlen(cfa->this_obj->v.str.buf, cfa->this_obj->v.str.len);
  begin = cfa->this_obj->v.str.buf;
  end = begin + cfa->this_obj->v.str.len;
  if (cfa->num_args > 0) {
    if (!is_num(cfa->args[0])) {
      // TODO(vrz) type conversion
    }
    from = cfa->args[0]->v.num;
    if (from < 0) {
      from += len;
      if (from < 0) from = 0;
    } else if (from > len)
      from = len;
    if (cfa->num_args > 1) {
      if (!is_num(cfa->args[1])) {
        // TODO(vrz) type conversion
      }
      to = cfa->args[1]->v.num;
      if (to < 0) {
        to += len;
        if (to < 0) to = 0;
      } else if (to > len)
        to = len;
    }
  }
  if (from > to) to = from;
  end = utfnshift(begin, to);
  begin = utfnshift(begin, from);
  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v7_init_str(v7_top_val(v7), begin, end - begin, 1);
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_split(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0], *arr = v7_push_new_object(v7);
  struct Resub sub, sub1;
  int limit = 1000000, elem = 0, i, len;
  unsigned long shift = 0;

  v7_set_class(arr, V7_CLASS_ARRAY);
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  if (cfa->num_args > 0) {
    if (cfa->num_args > 1 && cfa->args[1]->type == V7_TYPE_NUM)
      limit = cfa->args[1]->v.num;
    TRY(check_str_re_conv(v7, &arg, 1));
    TRY(regex_check_prog(arg));
    for (; elem < limit && shift < cfa->this_obj->v.str.len; elem++) {
      if (re_exec(arg->v.str.prog, arg->fl.fl, cfa->this_obj->v.str.buf + shift,
                  &sub))
        break;
      v7_append(v7, arr,
                v7_mkv(v7, V7_TYPE_STR, cfa->this_obj->v.str.buf + shift,
                       sub.sub[0].start - cfa->this_obj->v.str.buf - shift, 1));
      for (i = 1; i < sub.subexpr_num; i++)
        v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, sub.sub[i].start,
                                  sub.sub[i].end - sub.sub[i].start, 1));
      shift = sub.sub[0].end - cfa->this_obj->v.str.buf;
      sub1 = sub;
    }
  }
  len = cfa->this_obj->v.str.len - shift;
  if (elem < limit && len > 0)
    v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, cfa->this_obj->v.str.buf + shift,
                              len, 1));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_substr(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  char *begin, *end;
  long from = 0, to = 0, len;

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  to = len = utfnlen(cfa->this_obj->v.str.buf, cfa->this_obj->v.str.len);
  begin = cfa->this_obj->v.str.buf;
  end = begin + cfa->this_obj->v.str.len;
  if (cfa->num_args > 0) {
    if (!is_num(cfa->args[0])) {
      // TODO(vrz) type conversion
    }
    from = cfa->args[0]->v.num;
    if (NAN == cfa->args[0]->v.num && from < 0) from = 0;
    if (from > len) from = len;

    if (cfa->num_args > 1) {
      if (!is_num(cfa->args[1])) {
        // TODO(vrz) type conversion
      }
      to = cfa->args[1]->v.num;
      if (NAN == cfa->args[1]->v.num && to < 0) to = 0;
      if (to > len) to = len;
    }
  }
  if (from > to) {
    long tmp = to;
    to = from;
    from = tmp;
  }
  end = utfnshift(begin, to);
  begin = utfnshift(begin, from);
  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v7_init_str(v7_top_val(v7), begin, end - begin, 1);
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_toLowerCase(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long n, blen = 0;
  struct v7_val *str;
  char *p, *end;
  Rune runes[500];

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  for (n = 0; p < end; n++) {
    p += chartorune(&runes[n], p);
    runes[n] = tolowerrune(runes[n]);
    blen += runelen(runes[n]);
  }
  str = v7_push_string(v7, NULL, blen, 1);
  p = str->v.str.buf;
  end = p + blen;
  for (n = 0; p < end; n++) p += runetochar(p, &runes[n]);
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_toLocaleLowerCase(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long n, blen = 0;
  struct v7_val *str;
  char *p, *end;
  Rune runes[500];

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  for (n = 0; p < end; n++) {
    p += chartorune(&runes[n], p);
    runes[n] = tolowerrune(runes[n]);
    blen += runelen(runes[n]);
  }
  str = v7_push_string(v7, NULL, blen, 1);
  p = str->v.str.buf;
  end = p + blen;
  for (n = 0; p < end; n++) p += runetochar(p, &runes[n]);
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_toUpperCase(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long n, blen = 0;
  struct v7_val *str;
  char *p, *end;
  Rune runes[500];

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  for (n = 0; p < end; n++) {
    p += chartorune(&runes[n], p);
    runes[n] = toupperrune(runes[n]);
    blen += runelen(runes[n]);
  }
  str = v7_push_string(v7, NULL, blen, 1);
  p = str->v.str.buf;
  end = p + blen;
  for (n = 0; p < end; n++) p += runetochar(p, &runes[n]);
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_toLocaleUpperCase(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long n, blen = 0;
  struct v7_val *str;
  char *p, *end;
  Rune runes[500];

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  for (n = 0; p < end; n++) {
    p += chartorune(&runes[n], p);
    runes[n] = toupperrune(runes[n]);
    blen += runelen(runes[n]);
  }
  str = v7_push_string(v7, NULL, blen, 1);
  p = str->v.str.buf;
  end = p + blen;
  for (n = 0; p < end; n++) p += runetochar(p, &runes[n]);
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
#undef v7
}

static int _isspase(Rune c) { return isspacerune(c) || isnewline(c); }

V7_PRIVATE enum v7_err Str_trim(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  char *p, *begin = NULL, *end = NULL, *pend;
  Rune rune = ' ';

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  pend = p + cfa->this_obj->v.str.len;
  while (p < pend) {
    char *prevp = p;
    Rune prevrune = rune;
    p += chartorune(&rune, p);
    if (!_isspase(rune)) {
      end = NULL;
      if (_isspase(prevrune))
        if (NULL == begin) begin = prevp;
    } else if (!_isspase(prevrune))
      end = prevp;
  }
  if (NULL == end) end = cfa->this_obj->v.str.buf + cfa->this_obj->v.str.len;
  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v7_init_str(v7_top_val(v7), begin, end - begin, 1);
  return V7_OK;
#undef v7
}

V7_PRIVATE void Str_length(struct v7_val *this_obj, struct v7_val *arg,
                           struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_num(result, utfnlen(this_obj->v.str.buf, this_obj->v.str.len));
}

V7_PRIVATE void init_string(void) {
  init_standard_constructor(V7_CLASS_STRING, String_ctor);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "fromCharCode", Str_fromCharCode);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "valueOf", Str_valueOf);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charAt", Str_charAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charCodeAt", Str_charCodeAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "concat", Str_concat);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "indexOf", Str_indexOf);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "lastIndexOf", Str_lastIndexOf);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "localeCompare", Str_localeCompare);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "match", Str_match);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "replace", Str_replace);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "search", Str_search);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "slice", Str_slice);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "split", Str_split);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "substring", Str_substr);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "toLowerCase", Str_toLowerCase);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "toLocaleLowerCase",
             Str_toLocaleLowerCase);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "toUpperCase", Str_toUpperCase);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "toLocaleUpperCase",
             Str_toLocaleUpperCase);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "trim", Str_trim);

  SET_PROP_FUNC(s_prototypes[V7_CLASS_STRING], "length", Str_length);

  SET_RO_PROP_V(s_global, "String", s_constructors[V7_CLASS_STRING]);
}
