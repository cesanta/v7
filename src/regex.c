#include "internal.h"

#define MAX_BRANCHES 100
#define MAX_BRACKETS 100
#define FAIL_IF(condition, error_code) if (condition) return (error_code)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(ar) (sizeof(ar) / sizeof((ar)[0]))
#endif

#ifdef SLRE_DEBUG
#define DBG(x) printf x
#else
#define DBG(x)
#endif

struct bracket_pair {
  const char *ptr;  /* Points to the first char after '(' in regex  */
  int len;          /* Length of the text between '(' and ')'       */
  int branches;     /* Index in the branches array for this pair    */
  int num_branches; /* Number of '|' in this bracket pair           */
};

struct branch {
  int bracket_index;    /* index for 'struct bracket_pair brackets' */
                        /* array defined below                      */
  const char *schlong;  /* points to the '|' character in the regex */
};

struct regex_info {
  /*
   * Describes all bracket pairs in the regular expression.
   * First entry is always present, and grabs the whole regex.
   */
  struct bracket_pair brackets[MAX_BRACKETS];
  int num_brackets;

  /*
   * Describes alternations ('|' operators) in the regular expression.
   * Each branch falls into a specific branch pair.
   */
  struct branch branches[MAX_BRANCHES];
  int num_branches;

  /* Array of captures provided by the user */
  struct slre_cap *caps;
  int num_caps;

  /* E.g. SLRE_IGNORE_CASE. See enum below */
  int flags;
};

V7_PRIVATE int is_metacharacter(const unsigned char *s) {
  static const char *metacharacters = "^$().[]*+?|\\Ssdbfnrtv";
  return strchr(metacharacters, *s) != NULL;
}

V7_PRIVATE int op_len(const char *re) {
  return re[0] == '\\' && re[1] == 'x' ? 4 : re[0] == '\\' ? 2 : 1;
}

V7_PRIVATE int set_len(const char *re, int re_len) {
  int len = 0;

  while (len < re_len && re[len] != ']') {
    len += op_len(re + len);
  }

  return len <= re_len ? len + 1 : -1;
}

V7_PRIVATE int get_op_len(const char *re, int re_len) {
  return re[0] == '[' ? set_len(re + 1, re_len - 1) + 1 : op_len(re);
}

V7_PRIVATE int is_quantifier(const char *re) {
  return re[0] == '*' || re[0] == '+' || re[0] == '?';
}

V7_PRIVATE int toi(int x) {
  return isdigit(x) ? x - '0' : x - 'W';
}

V7_PRIVATE int hextoi(const unsigned char *s) {
  return (toi(tolower(s[0])) << 4) | toi(tolower(s[1]));
}

V7_PRIVATE int match_op(const unsigned char *re, const unsigned char *s,
                    struct regex_info *info) {
  int result = 0;
  switch (*re) {
    case '\\':
      /* Metacharacters */
      switch (re[1]) {
        case 'S': FAIL_IF(isspace(*s), SLRE_NO_MATCH); result++; break;
        case 's': FAIL_IF(!isspace(*s), SLRE_NO_MATCH); result++; break;
        case 'd': FAIL_IF(!isdigit(*s), SLRE_NO_MATCH); result++; break;
        case 'b': FAIL_IF(*s != '\b', SLRE_NO_MATCH); result++; break;
        case 'f': FAIL_IF(*s != '\f', SLRE_NO_MATCH); result++; break;
        case 'n': FAIL_IF(*s != '\n', SLRE_NO_MATCH); result++; break;
        case 'r': FAIL_IF(*s != '\r', SLRE_NO_MATCH); result++; break;
        case 't': FAIL_IF(*s != '\t', SLRE_NO_MATCH); result++; break;
        case 'v': FAIL_IF(*s != '\v', SLRE_NO_MATCH); result++; break;

        case 'x':
          /* Match byte, \xHH where HH is hexadecimal byte representaion */
          FAIL_IF(hextoi(re + 2) != *s, SLRE_NO_MATCH);
          result++;
          break;

        default:
          /* Valid metacharacter check is done in bar() */
          FAIL_IF(re[1] != s[0], SLRE_NO_MATCH);
          result++;
          break;
      }
      break;

    case '|': FAIL_IF(1, SLRE_INTERNAL_ERROR); break;
    case '$': FAIL_IF(1, SLRE_NO_MATCH); break;
    case '.': result++; break;

    default:
      if (info->flags & SLRE_IGNORE_CASE) {
        FAIL_IF(tolower(*re) != tolower(*s), SLRE_NO_MATCH);
      } else {
        FAIL_IF(*re != *s, SLRE_NO_MATCH);
      }
      result++;
      break;
  }

  return result;
}

V7_PRIVATE int match_set(const char *re, int re_len, const char *s,
                     struct regex_info *info) {
  int len = 0, result = -1, invert = re[0] == '^';

  if (invert) re++, re_len--;

  while (len <= re_len && re[len] != ']' && result <= 0) {
    /* Support character range */
    if (re[len] != '-' && re[len + 1] == '-' && re[len + 2] != ']' &&
        re[len + 2] != '\0') {
      result = info->flags &&  SLRE_IGNORE_CASE ?
        *s >= re[len] && *s <= re[len + 2] :
        tolower(*s) >= tolower(re[len]) && tolower(*s) <= tolower(re[len + 2]);
      len += 3;
    } else {
      result = match_op((unsigned char *) re + len, (unsigned char *) s, info);
      len += op_len(re + len);
    }
  }
  return (!invert && result > 0) || (invert && result <= 0) ? 1 : -1;
}

V7_PRIVATE int doh(const char *s, int s_len, struct regex_info *info, int bi);

V7_PRIVATE int bar(const char *re, int re_len, const char *s, int s_len,
               struct regex_info *info, int bi) {
  /* i is offset in re, j is offset in s, bi is brackets index */
  int i, j, n, step;

  for (i = j = 0; i < re_len && j <= s_len; i += step) {

    /* Handle quantifiers. Get the length of the chunk. */
    step = re[i] == '(' ? info->brackets[bi + 1].len + 2 :
      get_op_len(re + i, re_len - i);

    DBG(("%s [%.*s] [%.*s] re_len=%d step=%d i=%d j=%d\n", __func__,
         re_len - i, re + i, s_len - j, s + j, re_len, step, i, j));

    FAIL_IF(is_quantifier(&re[i]), SLRE_UNEXPECTED_QUANTIFIER);
    FAIL_IF(step <= 0, SLRE_INVALID_CHARACTER_SET);

    if (i + step < re_len && is_quantifier(re + i + step)) {
      DBG(("QUANTIFIER: [%.*s]%c [%.*s]\n", step, re + i,
           re[i + step], s_len - j, s + j));
      if (re[i + step] == '?') {
        int result = bar(re + i, step, s + j, s_len - j, info, bi);
        j += result > 0 ? result : 0;
        i++;
      } else if (re[i + step] == '+' || re[i + step] == '*') {
        int j2 = j, nj = j, n1, n2 = -1, ni, non_greedy = 0;

        /* Points to the regexp code after the quantifier */
        ni = i + step + 1;
        if (ni < re_len && re[ni] == '?') {
          non_greedy = 1;
          ni++;
        }

        do {
          if ((n1 = bar(re + i, step, s + j2, s_len - j2, info, bi)) > 0) {
            j2 += n1;
          }
          if (re[i + step] == '+' && n1 < 0) break;

          if (ni >= re_len) {
            /* After quantifier, there is nothing */
            nj = j2;
          } else if ((n2 = bar(re + ni, re_len - ni, s + j2,
                               s_len - j2, info, bi)) >= 0) {
            /* Regex after quantifier matched */
            nj = j2 + n2;
          }
          if (nj > j && non_greedy) break;
        } while (n1 > 0);

        if (n1 < 0 && re[i + step] == '*' &&
            (n2 = bar(re + ni, re_len - ni, s + j, s_len - j, info, bi)) > 0) {
          nj = j + n2;
        }

        DBG(("STAR/PLUS END: %d %d %d %d %d\n", j, nj, re_len - ni, n1, n2));
        FAIL_IF(re[i + step] == '+' && nj == j, SLRE_NO_MATCH);

        /* If while loop body above was not executed for the * quantifier,  */
        /* make sure the rest of the regex matches                          */
        FAIL_IF(nj == j && ni < re_len && n2 < 0, SLRE_NO_MATCH);

        /* Returning here cause we've matched the rest of RE already */
        return nj;
      }
      continue;
    }

    if (re[i] == '[') {
      n = match_set(re + i + 1, re_len - (i + 2), s + j, info);
      DBG(("SET %.*s [%.*s] -> %d\n", step, re + i, s_len - j, s + j, n));
      FAIL_IF(n <= 0, SLRE_NO_MATCH);
      j += n;
    } else if (re[i] == '(') {
      n = SLRE_NO_MATCH;
      bi++;
      FAIL_IF(bi >= info->num_brackets, SLRE_INTERNAL_ERROR);
      DBG(("CAPTURING [%.*s] [%.*s] [%s]\n",
           step, re + i, s_len - j, s + j, re + i + step));

      if (re_len - (i + step) <= 0) {
        /* Nothing follows brackets */
        n = doh(s + j, s_len - j, info, bi);
      } else {
        int j2;
        for (j2 = 0; j2 <= s_len - j; j2++) {
          if ((n = doh(s + j, s_len - (j + j2), info, bi)) >= 0 &&
              bar(re + i + step, re_len - (i + step),
                  s + j + n, s_len - (j + n), info, bi) >= 0) break;
        }
      }

      DBG(("CAPTURED [%.*s] [%.*s]:%d\n", step, re + i, s_len - j, s + j, n));
      FAIL_IF(n < 0, n);
      if (info->caps != NULL) {
        info->caps[bi - 1].ptr = s + j;
        info->caps[bi - 1].len = n;
      }
      j += n;
    } else if (re[i] == '^') {
      FAIL_IF(j != 0, SLRE_NO_MATCH);
    } else if (re[i] == '$') {
      FAIL_IF(j != s_len, SLRE_NO_MATCH);
    } else {
      FAIL_IF(j >= s_len, SLRE_NO_MATCH);
      n = match_op((unsigned char *) (re + i), (unsigned char *) (s + j), info);
      FAIL_IF(n <= 0, n);
      j += n;
    }
  }

  return j;
}

/* Process branch points */
V7_PRIVATE int doh(const char *s, int s_len, struct regex_info *info, int bi) {
  const struct bracket_pair *b = &info->brackets[bi];
  int i = 0, len, result;
  const char *p;

  do {
    p = i == 0 ? b->ptr : info->branches[b->branches + i - 1].schlong + 1;
    len = b->num_branches == 0 ? b->len :
      i == b->num_branches ? (int) (b->ptr + b->len - p) :
      (int) (info->branches[b->branches + i].schlong - p);
    DBG(("%s %d %d [%.*s] [%.*s]\n", __func__, bi, i, len, p, s_len, s));
    result = bar(p, len, s, s_len, info, bi);
    DBG(("%s <- %d\n", __func__, result));
  } while (result <= 0 && i++ < b->num_branches);  /* At least 1 iteration */

  return result;
}

V7_PRIVATE int baz(const char *s, int s_len, struct regex_info *info) {
  int i, result = -1, is_anchored = info->brackets[0].ptr[0] == '^';

  for (i = 0; i <= s_len; i++) {
    result = doh(s + i, s_len - i, info, 0);
    if (result >= 0) {
      result += i;
      break;
    }
    if (is_anchored) break;
  }

  return result;
}

V7_PRIVATE void setup_branch_points(struct regex_info *info) {
  int i, j;
  struct branch tmp;

  /* First, sort branches. Must be stable, no qsort. Use bubble algo. */
  for (i = 0; i < info->num_branches; i++) {
    for (j = i + 1; j < info->num_branches; j++) {
      if (info->branches[i].bracket_index > info->branches[j].bracket_index) {
        tmp = info->branches[i];
        info->branches[i] = info->branches[j];
        info->branches[j] = tmp;
      }
    }
  }

  /*
   * For each bracket, set their branch points. This way, for every bracket
   * (i.e. every chunk of regex) we know all branch points before matching.
   */
  for (i = j = 0; i < info->num_brackets; i++) {
    info->brackets[i].num_branches = 0;
    info->brackets[i].branches = j;
    while (j < info->num_branches && info->branches[j].bracket_index == i) {
      info->brackets[i].num_branches++;
      j++;
    }
  }
}

V7_PRIVATE int foo(const char *re, int re_len, const char *s, int s_len,
               struct regex_info *info) {
  int i, step, depth = 0;

  /* First bracket captures everything */
  info->brackets[0].ptr = re;
  info->brackets[0].len = re_len;
  info->num_brackets = 1;

  /* Make a single pass over regex string, memorize brackets and branches */
  for (i = 0; i < re_len; i += step) {
    step = get_op_len(re + i, re_len - i);

    if (re[i] == '|') {
      FAIL_IF(info->num_branches >= (int) ARRAY_SIZE(info->branches),
              SLRE_TOO_MANY_BRANCHES);
      info->branches[info->num_branches].bracket_index =
        info->brackets[info->num_brackets - 1].len == -1 ?
        info->num_brackets - 1 : depth;
      info->branches[info->num_branches].schlong = &re[i];
      info->num_branches++;
    } else if (re[i] == '\\') {
      FAIL_IF(i >= re_len - 1, SLRE_INVALID_METACHARACTER);
      if (re[i + 1] == 'x') {
        /* Hex digit specification must follow */
        FAIL_IF(re[i + 1] == 'x' && i >= re_len - 3,
                SLRE_INVALID_METACHARACTER);
        FAIL_IF(re[i + 1] ==  'x' && !(isxdigit(re[i + 2]) &&
                isxdigit(re[i + 3])), SLRE_INVALID_METACHARACTER);
      } else {
        FAIL_IF(!is_metacharacter((unsigned char *) re + i + 1),
                SLRE_INVALID_METACHARACTER);
      }
    } else if (re[i] == '(') {
      FAIL_IF(info->num_brackets >= (int) ARRAY_SIZE(info->brackets),
              SLRE_TOO_MANY_BRACKETS);
      depth++;  /* Order is important here. Depth increments first. */
      info->brackets[info->num_brackets].ptr = re + i + 1;
      info->brackets[info->num_brackets].len = -1;
      info->num_brackets++;
      FAIL_IF(info->num_caps > 0 && info->num_brackets - 1 > info->num_caps,
              SLRE_CAPS_ARRAY_TOO_SMALL);
    } else if (re[i] == ')') {
      int ind = info->brackets[info->num_brackets - 1].len == -1 ?
        info->num_brackets - 1 : depth;
      info->brackets[ind].len = (int) (&re[i] - info->brackets[ind].ptr);
      DBG(("SETTING BRACKET %d [%.*s]\n",
           ind, info->brackets[ind].len, info->brackets[ind].ptr));
      depth--;
      FAIL_IF(depth < 0, SLRE_UNBALANCED_BRACKETS);
      FAIL_IF(i > 0 && re[i - 1] == '(', SLRE_NO_MATCH);
    }
  }

  FAIL_IF(depth != 0, SLRE_UNBALANCED_BRACKETS);
  setup_branch_points(info);

  return baz(s, s_len, info);
}

V7_PRIVATE int slre_match(const char *regexp, const char *s, int s_len,
               struct slre_cap *caps, int num_caps, int flags) {
  struct regex_info info;

  /* Initialize info structure */
  info.flags = flags;
  info.num_brackets = info.num_branches = 0;
  info.num_caps = num_caps;
  info.caps = caps;

  DBG(("========================> [%s] [%.*s]\n", regexp, s_len, s));
  return foo(regexp, (int) strlen(regexp), s, s_len, &info);
}


V7_PRIVATE enum v7_err Regex_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  struct v7_val *str = cfa->args[0];

  if(cfa->num_args > 0){
    if(!is_string(str)){
      TRY(toString(cfa->v7, str));
      str = v7_top_val(cfa->v7);
    }
    // TODO: copy str --> regex
    
    if(cfa->num_args > 1){
      struct v7_val *flags = cfa->args[1];
      if(!is_string(flags)){
        TRY(toString(cfa->v7, flags));
        flags = v7_top_val(cfa->v7);
      }
      uint32_t ind = flags->v.str.len;
      while(ind){
        switch(flags->v.str.buf[--ind]){
          case 'g': obj->regexp_fl_g=1;  break;
          case 'i': obj->regexp_fl_i=1;  break;
          case 'm': obj->regexp_fl_m=1;  break;
        }
      }
    }
  }
  
  v7_set_class(obj, V7_CLASS_REGEXP);
  return V7_OK;
}

V7_PRIVATE void Regex_global(struct v7_val *this_obj, struct v7_val *result) {
  v7_init_bool(result, this_obj->regexp_fl_g);
}

V7_PRIVATE void Regex_ignoreCase(struct v7_val *this_obj, struct v7_val *result) {
  v7_init_bool(result, this_obj->regexp_fl_i);
}

V7_PRIVATE void Regex_multiline(struct v7_val *this_obj, struct v7_val *result) {
  v7_init_bool(result, this_obj->regexp_fl_m);
}

V7_PRIVATE void init_regex(void) {
  init_standard_constructor(V7_CLASS_REGEXP, Regex_ctor);
  SET_RO_PROP_V(s_global, "RegExp", s_constructors[V7_CLASS_REGEXP]);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "global", Regex_global);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "ignoreCase", Regex_ignoreCase);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "multiline", Regex_multiline);
}
