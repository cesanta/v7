#include "internal.h"

V7_PRIVATE enum v7_err check_str_re_conv(struct v7 *v7, struct v7_val **arg, int re_fl){
  /* If argument is not (RegExp + re_fl) or string, do type conversion */
  if(!is_string(*arg) && !(re_fl && instanceof(*arg, &s_constructors[V7_CLASS_REGEXP]))){
    TRY(toString(v7, *arg));
    *arg = v7_top_val(v7);
    inc_ref_count(*arg);
    TRY(inc_stack(v7, -2));
    TRY(v7_push(v7, *arg));
 }
  return V7_OK;
}

V7_PRIVATE enum v7_err String_ctor(struct v7_c_func_arg *cfa) {
  #define v7 (cfa->v7) /* Needed for TRY() macro below */
  const char *str = NULL;
  size_t len = 0;
  int own = 0;
  struct v7_val *arg = cfa->args[0],
                *obj = cfa->this_obj;
  if(!cfa->called_as_constructor) obj = v7_push_new_object(v7);

  if(cfa->num_args > 0){
    TRY(check_str_re_conv(v7, &arg, 0));
    str = arg->v.str.buf;
    len = arg->v.str.len;
    own = 1;
  }
  v7_init_str(obj, str, len, own);
  v7_set_class(obj, V7_CLASS_STRING);
  return V7_OK;
  #undef v7
}

V7_PRIVATE void Str_length(struct v7_val *this_obj, struct v7_val *arg, struct v7_val *result) {
  if(NULL == result || arg) return;
  v7_init_num(result, this_obj->v.str.len);
}

static const char *StrAt(struct v7_c_func_arg *cfa) {
  if (cfa->num_args > 0 && cfa->args[0]->type == V7_TYPE_NUM &&
    cfa->this_obj->type == V7_TYPE_STR &&
    fabs(cfa->args[0]->v.num) < cfa->this_obj->v.str.len) {
    int idx = (int) cfa->args[0]->v.num;
    const char *p = cfa->this_obj->v.str.buf;
    return idx > 0 ? p + idx : p + cfa->this_obj->v.str.len - idx;
  }
  return NULL;
}

V7_PRIVATE enum v7_err Str_charCodeAt(struct v7_c_func_arg *cfa) {
  const char *p = StrAt(cfa);
  v7_push_number(cfa->v7, p == NULL ? NAN : p[0]);
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_charAt(struct v7_c_func_arg *cfa) {
  const char *p = StrAt(cfa);
  v7_push_string(cfa->v7, p, p == NULL ? 0 : 1, 1);
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_match(struct v7_c_func_arg *cfa) {
  #define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0];
  struct Resub sub;
  struct v7_val *arr = NULL;
  unsigned long shift = 0;

  if(cfa->num_args > 0){
    TRY(check_str_re_conv(v7, &arg, 1));
    TRY(regex_check_prog(arg));
    do{
      if(!re_exec(arg->v.str.prog, arg->fl.fl, cfa->this_obj->v.str.buf + shift, &sub)){
        if(NULL == arr){
          arr = v7_push_new_object(v7);
          v7_set_class(arr, V7_CLASS_ARRAY);
        }
        shift = sub.sub[0].end - cfa->this_obj->v.str.buf;
        v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, sub.sub[0].start, sub.sub[0].end - sub.sub[0].start, 1));
      }
    }while(arg->fl.fl.re_g && shift < cfa->this_obj->v.str.len);
  }
  if(0 == shift) TRY(v7_make_and_push(v7, V7_TYPE_NULL));
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
  if(cfa->num_args > 0){
    if(cfa->num_args > 1 && cfa->args[1]->type == V7_TYPE_NUM) limit = cfa->args[1]->v.num;
    TRY(check_str_re_conv(v7, &arg, 1));
    TRY(regex_check_prog(arg));
    for(; elem < limit && shift < cfa->this_obj->v.str.len; elem++){
      if(re_exec(arg->v.str.prog, arg->fl.fl, cfa->this_obj->v.str.buf + shift, &sub)) break;
      v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, cfa->this_obj->v.str.buf + shift, sub.sub[0].start - cfa->this_obj->v.str.buf - shift, 1));
      for(i = 1; i < sub.subexpr_num; i++)
        v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, sub.sub[i].start, sub.sub[i].end - sub.sub[i].start, 1));
      shift = sub.sub[0].end - cfa->this_obj->v.str.buf;
      sub1=sub;
    }
  }
  len = cfa->this_obj->v.str.len - shift;
  if(elem < limit && len > 0)
    v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, cfa->this_obj->v.str.buf + shift, len, 1));
  return V7_OK;
  #undef v7
}

V7_PRIVATE enum v7_err Str_indexOf(struct v7_c_func_arg *cfa) {
  double index = -1.0;
  if (cfa->this_obj->type == V7_TYPE_STR &&
      cfa->num_args > 0 &&
      cfa->args[0]->type == V7_TYPE_STR) {
    int i = cfa->num_args > 1 && cfa->args[1]->type == V7_TYPE_NUM ?
    (int) cfa->args[1]->v.num : 0;
    const struct v7_string *a = &cfa->this_obj->v.str,
    *b = &cfa->args[0]->v.str;

    /* Scan the string, advancing one byte at a time */
    for (; i >= 0 && a->len >= b->len && i <= (int) (a->len - b->len); i++) {
      if (memcmp(a->buf + i, b->buf, b->len) == 0) {
        index = i;
        break;
      }
    }
  }
  v7_push_number(cfa->v7, index);
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_substr(struct v7_c_func_arg *cfa) {
  struct v7_val *result = v7_push_string(cfa->v7, NULL, 0, 0);
  long start = 0;

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
      v7_init_str(result, cfa->this_obj->v.str.buf + start, n, 1);
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_search(struct v7_c_func_arg *cfa) {
  #define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0];
  struct Resub sub;
  int shift = -1, utf_shift = -1;

  if(cfa->num_args > 0){
    TRY(check_str_re_conv(v7, &arg, 1));
    TRY(regex_check_prog(arg));
    if(!re_exec(arg->v.str.prog, arg->fl.fl, cfa->this_obj->v.str.buf, &sub)) shift = sub.sub[0].start - cfa->this_obj->v.str.buf;
  }
  if(shift > 0){ /* calc shift for UTF-8 */
    Rune rune;
    const char *str = cfa->this_obj->v.str.buf;
    utf_shift = 0;
    do{
      str += chartorune(&rune, str);
      utf_shift++;
    }while(str - cfa->this_obj->v.str.buf < shift);
  }
  v7_push_number(v7, utf_shift);
  return V7_OK;
  #undef v7
}

V7_PRIVATE enum v7_err Str_replace(struct v7_c_func_arg *cfa){
  #define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *result = v7_push_new_object(v7);
  const char *out_str = cfa->this_obj->v.str.buf;
  uint8_t own = 1;
  size_t out_len = cfa->this_obj->v.str.len;
  int old_sp = v7->sp;

  if(cfa->num_args > 1){
    const char * const str_end = cfa->this_obj->v.str.buf + cfa->this_obj->v.str.len;
    char *p = cfa->this_obj->v.str.buf;
    uint32_t out_sub_num = 0;
    struct v7_val *re = cfa->args[0], *str_func = cfa->args[1], *arr = NULL;
    struct re_tok out_sub[V7_RE_MAX_REPL_SUB], *ptok = out_sub;
    struct Resub loot;
    TRY(check_str_re_conv(v7, &re, 1));
    TRY(regex_check_prog(re));
    if(v7_is_class(str_func, V7_CLASS_FUNCTION)){
      arr = v7_push_new_object(v7);
      v7_set_class(arr, V7_CLASS_ARRAY);
      TRY(v7_push(v7, str_func));
    }else TRY(check_str_re_conv(v7, &str_func, 0));

    out_len = 0;
    do{
      int i;
      if(re_exec(re->v.str.prog, re->fl.fl, p, &loot)) break;
      if(p != loot.sub->start){
        ptok->start = p;
        ptok->end = loot.sub->start;
        ptok++;
        out_len += loot.sub->start - p;
        out_sub_num++;
      }

      if(NULL != arr){ /* replace function */
        Rune rune;
        int old_sp = v7->sp, utf_shift = 0;
        struct v7_val *rez_str;
        for(i = 0; i < loot.subexpr_num; i++)
          v7_push_string(v7, loot.sub[i].start, loot.sub[i].end - loot.sub[i].start, 1);
        for(i = 0; p + i < loot.sub[0].start; i += chartorune(&rune, p + i), utf_shift++);
        TRY(push_number(v7, utf_shift));
        TRY(v7_push(v7, cfa->this_obj));
        rez_str = v7_call(v7, cfa->this_obj, loot.subexpr_num + 2);
        TRY(check_str_re_conv(v7, &rez_str, 0));
        if(rez_str->v.str.len){
          ptok->start = rez_str->v.str.buf;
          ptok->end = rez_str->v.str.buf + rez_str->v.str.len;
          ptok++;
          out_len += rez_str->v.str.len;
          out_sub_num++;
          v7_append(v7, arr, rez_str);
        }
        TRY(inc_stack(v7, old_sp - v7->sp));
      }else{ /* replace string */
        struct Resub newsub;
        re_rplc(&loot, p, str_func->v.str.buf, &newsub);
        for(i = 0; i < newsub.subexpr_num; i++){
          ptok->start = newsub.sub[i].start;
          ptok->end = newsub.sub[i].end;
          ptok++;
          out_len += newsub.sub[i].end - newsub.sub[i].start;
          out_sub_num++;
        }
      }
      p = (char *)loot.sub->end;
    }while(re->fl.fl.re_g && p < str_end);
    if(p < str_end){
      ptok->start = p;
      ptok->end = str_end;
      ptok++;
      out_len += str_end - p;
      out_sub_num++;
    }
    out_str = malloc(out_len+1);
    CHECK(out_str, V7_OUT_OF_MEMORY);
    ptok = out_sub; p = (char *)out_str;
    do{
      size_t ln = ptok->end - ptok->start;
      memcpy(p, ptok->start, ln);
      p += ln;
      ptok++;
    }while(--out_sub_num);
    *p = '\0';
    own = 0;
  }
  TRY(inc_stack(v7, old_sp - v7->sp));
  v7_init_str(result, out_str, out_len, own);
  result->fl.fl.str_alloc = 1;
  return V7_OK;
  #undef v7
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
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "search", Str_search);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "replace", Str_replace);

  SET_RO_PROP_V(s_global, "String", s_constructors[V7_CLASS_STRING]);
}
