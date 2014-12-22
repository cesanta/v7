/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#ifdef RE_TEST

int re_replace(struct Resub *loot, const char *src, const char *rstr,
               char **dst) {
  struct Resub newsub;
  struct slre_tok *t = newsub.sub;
  char *d;
  int osz = re_rplc(loot, src, rstr, &newsub);
  int i = newsub.subexpr_num;
  if (osz < 0) {
    printf("re_rplc return: '%s'\n", re_err_msg(osz));
    return 0;
  }
  *dst = NULL;
  if (osz) *dst = reg_malloc(osz + 1);
  if (!*dst) return 0;
  d = *dst;
  do {
    size_t len = t->end - t->start;
    memcpy(d, t->start, len);
    d += len;
    t++;
  } while (--i);
  *d = '\0';
  return osz;
}

#define RE_TEST_STR_SIZE 2000

static unsigned short get_flags(const char *ch) {
  unsigned short flags = 0;

  while (*ch != '\0') {
    switch (*ch) {
      case 'g': flags |= RE_FLAG_G; break;
      case 'i': flags |= RE_FLAG_I; break;
      case 'm': flags |= RE_FLAG_M; break;
      case 'r': flags |= RE_FLAG_RE; break;
      default: return flags;
    }
    ch++;
  }
  return flags;
}

static void usage(void) {
  printf(
      "usage: regex_test.exe \"pattern\" [\"flags: gimr\"[ \"source\"[ "
      "\"replaceStr\"]]]\n   or: regex_test.exe -f file_path [>out.txt]\n");
}

int main(int argc, char **argv) {
  const char *src;
  char *dst;
  const char *rstr;
  const char *error;
  struct Reprog *pr;
  struct Resub sub;
  struct v7_val_flags flags = {0, 0, 0, 0, 0, 0, 0, 0, 1};
  unsigned int i, k = 0;

  if (argc > 1) {
    if (strcmp(argv[1], "-f") == 0) {
      FILE *fp;
      char str[RE_TEST_STR_SIZE];
      long file_size;
      if (argc < 3) {
        usage();
        return 0;
      }
      if ((fp = fopen(argv[2], "r")) == NULL) {
        printf("file: \"%s\" not found", argv[2]);
      } else if (fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0) {
        fclose(fp);
      } else {
        rewind(fp);
        while (fgets(str, RE_TEST_STR_SIZE, fp)) {
          char *patt = NULL, *fl_str = NULL, *curr = str, *beg = NULL;
          src = rstr = NULL;
          k++;
          if ((curr = strchr(curr, '"')) == NULL) continue;
          beg = ++curr;
          while (*curr) {
            if ((curr = strchr(curr, '"')) == NULL) break;
            if (*(curr + 1) == ' ' || *(curr + 1) == '\r' ||
                *(curr + 1) == '\n' || *(curr + 1) == '\0')
              break;
            curr++;
          }
          if (curr == NULL || *curr == '\0') return 1;
          *curr = '\0';
          patt = beg;

          if ((curr = strchr(++curr, '"'))) {
            beg = ++curr;
            while (*curr) {
              if ((curr = strchr(curr, '"')) == NULL) break;
              if (*(curr + 1) == ' ' || *(curr + 1) == '\r' ||
                  *(curr + 1) == '\n' || *(curr + 1) == '\0')
                break;
              curr++;
            }
            if (curr != NULL && *curr != '\0') {
              *curr = '\0';
              fl_str = beg;
              if ((curr = strchr(++curr, '"'))) {
                beg = ++curr;
                while (*curr) {
                  if ((curr = strchr(curr, '"')) == NULL) break;
                  if (*(curr + 1) == ' ' || *(curr + 1) == '\r' ||
                      *(curr + 1) == '\n' || *(curr + 1) == '\0')
                    break;
                  curr++;
                }
                if (curr != NULL && *curr != '\0') {
                  *curr = '\0';
                  src = beg;
                  if ((curr = strchr(++curr, '"'))) {
                    beg = ++curr;
                    while (*curr) {
                      if ((curr = strchr(curr, '"')) == NULL) break;
                      if (*(curr + 1) == ' ' || *(curr + 1) == '\r' ||
                          *(curr + 1) == '\n' || *(curr + 1) == '\0')
                        break;
                      curr++;
                    }
                    if (curr != NULL && *curr != '\0') {
                      *curr = '\0';
                      rstr = beg;
                    }
                  }
                }
              }
            }
          }
          if (patt) {
            if (k > 1) puts("");
            printf("%03d: \"%s\"", k, patt);
            if (fl_str) {
              printf(" \"%s\"", fl_str);
              flags = get_flags(fl_str);
            }
            if (src) printf(" \"%s\"", src);
            if (rstr) printf(" \"%s\"", rstr);
            printf("\n");
            pr = re_compiler(patt, flags, &error);
            if (!pr) {
              printf("re_compiler: %s\n", error);
              return 1;
            }
            printf("number of subexpressions = %d\n", pr->subexpr_num);
            if (src) {
              if (!re_exec(pr, flags, src, &sub)) {
                for (i = 0; i < sub.subexpr_num; ++i) {
                  int n = sub.sub[i].end - sub.sub[i].start;
                  if (n > 0)
                    printf("match: %-3d start:%-3d end:%-3d size:%-3d '%.*s'\n",
                           i, (int)(sub.sub[i].start - src),
                           (int)(sub.sub[i].end - src), n, n, sub.sub[i].start);
                  else
                    printf("match: %-3d ''\n", i);
                }

                if (rstr) {
                  if (re_replace(&sub, src, rstr, &dst)) {
                    printf("output: \"%s\"\n", dst);
                  }
                }
              } else
                printf("no match\n");
            }
            re_free(pr);
          }
        }
        fclose(fp);
      }
      return 0;
    }

    if (argc > 2) flags = get_flags(argv[2]);
    pr = re_compiler(argv[1], flags, &error);
    if (!pr) {
      fprintf(stderr, "re_compiler: %s\n", error);
      return 1;
    }
    printf("number of subexpressions = %d\n", pr->subexpr_num);
    if (argc > 3) {
      src = argv[3];
      if (!re_exec(pr, flags, src, &sub)) {
        for (i = 0; i < sub.subexpr_num; ++i) {
          int n = sub.sub[i].end - sub.sub[i].start;
          if (n > 0)
            printf("match: %-3d start:%-3d end:%-3d size:%-3d '%.*s'\n", i,
                   (int)(sub.sub[i].start - src), (int)(sub.sub[i].end - src),
                   n, n, sub.sub[i].start);
          else
            printf("match: %-3d ''\n", i);
        }

        if (argc > 4) {
          rstr = argv[4];
          if (re_replace(&sub, src, rstr, &dst)) {
            printf("output: \"%s\"\n\n", dst);
          }
        }
      } else
        printf("no match\n");
      re_free(pr);
    }
  } else
    usage();

  return 0;
}
#else

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
        obj->fl.fl.re_flags |= RE_FLAG_G;
        break;
      case 'i':
      obj->fl.fl.re_flags |= RE_FLAG_I;
        break;
      case 'm':
        obj->fl.fl.re_flags |= RE_FLAG_M;
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
  v7_init_bool(result, this_obj->fl.fl.re_flags & RE_FLAG_G);
}

V7_PRIVATE void Regex_ignoreCase(struct v7_val *this_obj, struct v7_val *arg,
                                 struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_bool(result, this_obj->fl.fl.re_flags & RE_FLAG_I);
}

V7_PRIVATE void Regex_multiline(struct v7_val *this_obj, struct v7_val *arg,
                                struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_bool(result, this_obj->fl.fl.re_flags & RE_FLAG_M);
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
  if (NULL == re_obj->v.str.prog) {
    re_obj->v.str.prog = re_compiler(re_obj->v.str.buf, re_obj->fl.fl.re_flags,
                                     NULL);
    if (-1 == (int)re_obj->v.str.prog) return V7_REGEXP_ERROR;
    if (NULL == re_obj->v.str.prog) return V7_OUT_OF_MEMORY;
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Regex_exec(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0], *arr = NULL, *t = cfa->this_obj;
  struct Resub sub;
  struct slre_tok *ptok = sub.sub;

  if (cfa->num_args > 0) {
    char *begin;
    TRY(check_str_re_conv(v7, &arg, 0));
    begin = arg->v.str.buf;
    if (t->v.str.lastIndex < 0) t->v.str.lastIndex = 0;
    if (t->fl.fl.re_flags & RE_FLAG_G) {
      begin = utfnshift(begin, t->v.str.lastIndex);
    }
    TRY(regex_check_prog(t));
    if (!re_exec(t->v.str.prog, t->fl.fl.re_flags, begin, &sub)) {
      int i;
      arr = v7_push_new_object(v7);
      v7_set_class(arr, V7_CLASS_ARRAY);
      for (i = 0; i < sub.subexpr_num; i++, ptok++)
        v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, ptok->start,
                                  ptok->end - ptok->start, 1));
      if (t->fl.fl.re_flags & RE_FLAG_G)
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
#endif
