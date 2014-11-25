#include "internal.h"

struct re_env {
  struct v7_val_flags flags;
  const char *src;
  Rune curr_rune;

  struct Reprog *prog;
  struct Renode *pstart, *pend;

  struct Renode *sub[RE_MAX_SUB];
  unsigned int subexpr_num;
  unsigned int sets_num;

  int lookahead;
  struct Reclass *curr_set;
  int min_rep, max_rep;

  jmp_buf catch_point;
  const char *err_msg;
};

enum RE_CODE {
  I_END = 10, /* Terminate: match found */
  I_ANY,
  P_ANY = I_ANY, /* Any character except newline, . */
  I_ANYNL,       /* Any character including newline, . */
  I_BOL,
  P_BOL = I_BOL, /* Beginning of line, ^ */
  I_CH,
  P_CH = I_CH,
  I_EOL,
  P_EOL = I_EOL, /* End of line, $ */
  I_EOS,
  P_EOS = I_EOS, /* End of string, \0 */
  I_JUMP,
  I_LA,
  P_LA = I_LA,
  I_LA_N,
  P_LA_N = I_LA_N,
  I_LBRA,
  P_BRA = I_LBRA, /* Left bracket, ( */
  I_REF,
  P_REF = I_REF,
  I_REP,
  P_REP = I_REP,
  I_REP_INI,
  I_RBRA, /* Right bracket, ) */
  I_SET,
  P_SET = I_SET, /* Character set, [] */
  I_SET_N,
  P_SET_N = I_SET_N, /* Negated character set, [] */
  I_SPLIT,
  I_WORD,
  P_WORD = I_WORD,
  I_WORD_N,
  P_WORD_N = I_WORD_N,
  P_ALT, /* Alternation, | */
  P_CAT, /* Concatentation, implicit operator */
  L_CH = 256,
  L_COUNT,  /* {M,N} */
  L_EOS,    /* End of string, \0 */
  L_LA,     /* "(?=" lookahead */
  L_LA_CAP, /* "(?:" lookahead, capture */
  L_LA_N,   /* "(?!" negative lookahead */
  L_REF,    /* "\1" back-reference */
  L_SET,    /* character set */
  L_SET_N,  /* negative character set */
  L_WORD,   /* "\b" word boundary */
  L_WORD_N  /* "\B" non-word boundary */
};

enum RE_MESSAGE {
  INVALID_DEC_DIGIT = -1,
  INVALID_HEX_DIGIT = -2,
  INVALID_ESC_CHAR = -3,
  UNTERM_ESC_SEQ = -4,
  SYNTAX_ERROR = -5,
  UNMATCH_LBR = -6,
  UNMATCH_RBR = -7,
  NUM_OVERFLOW = -8,
  INF_LOOP_M_EMP_STR = -9,
  TOO_MANY_CH_SETS = -10,
  INV_CH_SET_RANGE = -11,
  CH_SET_TOO_LARGE = -12,
  MALFORMED_CH_SET = -13,
  INVALID_BACK_REF = -14,
  TOO_MANY_CAPTURES = -15,
  INVALID_QUANTIFIER = -16,

  BAD_CHAR_AFTER_USD = -64
};

static const char *re_err_msg(enum RE_MESSAGE err) {
  switch (err) {
    case INVALID_DEC_DIGIT:
      return "invalid DEC digit";
    case INVALID_HEX_DIGIT:
      return "invalid HEX digit";
    case INVALID_ESC_CHAR:
      return "invalid escape character";
    case UNTERM_ESC_SEQ:
      return "unterminated escape sequence";
    case SYNTAX_ERROR:
      return "syntax error";
    case UNMATCH_LBR:
      return "'(' unmatched";
    case UNMATCH_RBR:
      return "')' unmatched";
    case NUM_OVERFLOW:
      return "numeric overflow";
    case INF_LOOP_M_EMP_STR:
      return "infinite loop matching the empty string";
    case TOO_MANY_CH_SETS:
      return "too many character sets";
    case INV_CH_SET_RANGE:
      return "invalid character set range";
    case CH_SET_TOO_LARGE:
      return "char set too large; increase struct Reclass.spans size";
    case MALFORMED_CH_SET:
      return "malformed '[]'";
    case INVALID_BACK_REF:
      return "invalid back-reference";
    case TOO_MANY_CAPTURES:
      return "too many captures";
    case INVALID_QUANTIFIER:
      return "invalid quantifier";

    case BAD_CHAR_AFTER_USD:
      return "bad character after '$' in replace pattern";
  }
  return "";
}

static sint8_t dec(int c) {
  if (isdigitrune(c)) return c - '0';
  return INVALID_DEC_DIGIT;
}

static uint8_t re_dec_digit(struct re_env *e, int c) {
  sint8_t ret = dec(c);
  if (ret < 0)
    V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(INVALID_DEC_DIGIT));
  return ret;
}

static sint8_t hex(int c) {
  if (isdigitrune(c)) return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return INVALID_HEX_DIGIT;
}

V7_PRIVATE sint8_t nextesc(Rune *r, const char **src) {
  sint8_t hd;
  *src += chartorune(r, *src);
  switch (*r) {
    case 0:
      return UNTERM_ESC_SEQ;
    case 'c':
      *r = **src & 31;
      ++*src;
      return 0;
    case 'f':
      *r = '\f';
      return 0;
    case 'n':
      *r = '\n';
      return 0;
    case 'r':
      *r = '\r';
      return 0;
    case 't':
      *r = '\t';
      return 0;
    case 'u':
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r = hd << 12;
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r += hd << 8;
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r += hd << 4;
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r += hd;
      if (!*r) {
        *r = '0';
        return 1;
      }
      return 0;
    case 'v':
      *r = '\v';
      return 0;
    case 'x':
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r = hd << 4;
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r += hd;
      if (!*r) {
        *r = '0';
        return 1;
      }
      return 0;
  }
  return 2;
}

static sint8_t re_nextc(Rune *r, const char **src, uint8_t re_flag) {
  *src += chartorune(r, *src);
  if (re_flag && *r == '\\') {
    /* sint8_t ret = */ nextesc(r, src);
    /* if(2 != ret) return ret;
    if (!strchr("$()*+-./0123456789?BDSW[\\]^bdsw{|}", *r))
      return INVALID_ESC_CHAR; */
    return 1;
  }
  return 0;
}

static uint8_t re_nextc_env(struct re_env *e) {
  sint8_t ret = re_nextc(&e->curr_rune, &e->src, e->flags.re);
  if (ret < 0) V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(ret));
  return ret;
}

static void re_nchset(struct re_env *e) {
  if (e->sets_num >= nelem(e->prog->charset))
    V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(TOO_MANY_CH_SETS));
  e->curr_set = e->prog->charset + e->sets_num++;
  e->curr_set->end = e->curr_set->spans;
}

static void re_rng2set(struct re_env *e, Rune start, Rune end) {
  if (start > end)
    V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(INV_CH_SET_RANGE));
  if (e->curr_set->end + 2 == e->curr_set->spans + nelem(e->curr_set->spans))
    V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(CH_SET_TOO_LARGE));
  e->curr_set->end->s = start;
  e->curr_set->end->e = end;
  e->curr_set->end++;
}

#define re_char2set(e, c) re_rng2set(e, c, c)

#define re_d_2set(e) re_rng2set(e, '0', '9')

static void re_D_2set(struct re_env *e) {
  re_rng2set(e, 0, '0' - 1);
  re_rng2set(e, '9' + 1, 0xFFFF);
}

static void re_s_2set(struct re_env *e) {
  re_char2set(e, 0x9);
  re_rng2set(e, 0xA, 0xD);
  re_char2set(e, 0x20);
  re_char2set(e, 0xA0);
  re_rng2set(e, 0x2028, 0x2029);
  re_char2set(e, 0xFEFF);
}

static void re_S_2set(struct re_env *e) {
  re_rng2set(e, 0, 0x9 - 1);
  re_rng2set(e, 0xD + 1, 0x20 - 1);
  re_rng2set(e, 0x20 + 1, 0xA0 - 1);
  re_rng2set(e, 0xA0 + 1, 0x2028 - 1);
  re_rng2set(e, 0x2029 + 1, 0xFEFF - 1);
  re_rng2set(e, 0xFEFF + 1, 0xFFFF);
}

static void re_w_2set(struct re_env *e) {
  re_d_2set(e);
  re_rng2set(e, 'A', 'Z');
  re_char2set(e, '_');
  re_rng2set(e, 'a', 'z');
}

static void re_W_2set(struct re_env *e) {
  re_rng2set(e, 0, '0' - 1);
  re_rng2set(e, '9' + 1, 'A' - 1);
  re_rng2set(e, 'Z' + 1, '_' - 1);
  re_rng2set(e, '_' + 1, 'a' - 1);
  re_rng2set(e, 'z' + 1, 0xFFFF);
}

static uint8_t re_endofcount(Rune c) {
  switch (c) {
    case ',':
    case '}':
      return 1;
  }
  return 0;
}

static void re_ex_num_overfl(struct re_env *e) {
  V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(NUM_OVERFLOW));
}

static enum RE_CODE re_countrep(struct re_env *e) {
  e->min_rep = 0;
  while (!re_endofcount(e->curr_rune = *e->src++))
    e->min_rep = e->min_rep * 10 + re_dec_digit(e, e->curr_rune);
  if (e->min_rep >= RE_MAX_REP) re_ex_num_overfl(e);

  if (e->curr_rune != ',') {
    e->max_rep = e->min_rep;
    return L_COUNT;
  }
  e->max_rep = 0;
  while ((e->curr_rune = *e->src++) != '}')
    e->max_rep = e->max_rep * 10 + re_dec_digit(e, e->curr_rune);
  if (!e->max_rep) {
    e->max_rep = RE_MAX_REP;
    return L_COUNT;
  }
  if (e->max_rep >= RE_MAX_REP) re_ex_num_overfl(e);

  return L_COUNT;
}

static enum RE_CODE re_lexset(struct re_env *e) {
  Rune ch;
  uint8_t esc, ch_fl = 0, dash_fl = 0;
  enum RE_CODE type = L_SET;

  re_nchset(e);

  esc = re_nextc_env(e);
  if (!esc && e->curr_rune == '^') {
    type = L_SET_N;
    esc = re_nextc_env(e);
  }

  for (; esc || e->curr_rune != ']'; esc = re_nextc_env(e)) {
    if (!e->curr_rune)
      V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(MALFORMED_CH_SET));
    if (esc) {
      if (strchr("DdSsWw", e->curr_rune)) {
        if (ch_fl) {
          re_char2set(e, ch);
          if (dash_fl) re_char2set(e, '-');
        }
        switch (e->curr_rune) {
          case 'D':
            re_D_2set(e);
            break;
          case 'd':
            re_d_2set(e);
            break;
          case 'S':
            re_S_2set(e);
            break;
          case 's':
            re_s_2set(e);
            break;
          case 'W':
            re_W_2set(e);
            break;
          case 'w':
            re_w_2set(e);
            break;
        }
        ch_fl = dash_fl = 0;
        continue;
      }
      switch (e->curr_rune) {
        default:
        /* case '-':
        case '\\':
        case '.':
        case '/':
        case ']':
        case '|': */
          break;
        case '0':
          e->curr_rune = 0;
          break;
        case 'b':
          e->curr_rune = '\b';
          break;
          /* default:
            V7_EX_THROW(e->catch_point, e->err_msg,
            re_err_msg(INVALID_ESC_CHAR)); */
      }
    } else {
      if (e->curr_rune == '-') {
        if (ch_fl) {
          if (dash_fl) {
            re_rng2set(e, ch, '-');
            ch_fl = dash_fl = 0;
          } else
            dash_fl = 1;
        } else {
          ch = '-';
          ch_fl = 1;
        }
        continue;
      }
    }
    if (ch_fl) {
      if (dash_fl) {
        re_rng2set(e, ch, e->curr_rune);
        ch_fl = dash_fl = 0;
      } else {
        re_char2set(e, ch);
        ch = e->curr_rune;
      }
    } else {
      ch = e->curr_rune;
      ch_fl = 1;
    }
  }
  if (ch_fl) {
    re_char2set(e, ch);
    if (dash_fl) re_char2set(e, '-');
  }
  return type;
}

static int re_lexer(struct re_env *e) {
  if (re_nextc_env(e)) {
    switch (e->curr_rune) {
      case '0':
        e->curr_rune = 0;
        return L_EOS;
      case 'b':
        return L_WORD;
      case 'B':
        return L_WORD_N;
      case 'd':
        re_nchset(e);
        re_d_2set(e);
        return L_SET;
      case 'D':
        re_nchset(e);
        re_d_2set(e);
        return L_SET_N;
      case 's':
        re_nchset(e);
        re_s_2set(e);
        return L_SET;
      case 'S':
        re_nchset(e);
        re_s_2set(e);
        return L_SET_N;
      case 'w':
        re_nchset(e);
        re_w_2set(e);
        return L_SET;
      case 'W':
        re_nchset(e);
        re_w_2set(e);
        return L_SET_N;
    }
    if (isdigitrune(e->curr_rune)) {
      e->curr_rune -= '0';
      if (isdigitrune(*e->src))
        e->curr_rune = e->curr_rune * 10 + *e->src++ - '0';
      return L_REF;
    }
    return L_CH;
  }

  if (e->flags.re) switch (e->curr_rune) {
      case 0:
      case '$':
      case ')':
      case '*':
      case '+':
      case '.':
      case '?':
      case '^':
      case '|':
        return e->curr_rune;
      case '{':
        return re_countrep(e);
      case '[':
        return re_lexset(e);
      case '(':
        if (e->src[0] == '?') switch (e->src[1]) {
            case '=':
              e->src += 2;
              return L_LA;
            case ':':
              e->src += 2;
              return L_LA_CAP;
            case '!':
              e->src += 2;
              return L_LA_N;
          }
        return '(';
    }
  else if (e->curr_rune == 0)
    return 0;

  return L_CH;
}

#define RE_NEXT(env) (env)->lookahead = re_lexer(env)
#define RE_ACCEPT(env, t) ((env)->lookahead == (t) ? RE_NEXT(env), 1 : 0)

static struct Renode *re_nnode(struct re_env *e, int type) {
  memset(e->pend, 0, sizeof(struct Renode));
  e->pend->type = type;
  return e->pend++;
}

static uint8_t re_isemptynd(struct Renode *nd) {
  if (!nd) return 1;
  switch (nd->type) {
    default:
      return 1;
    case P_ANY:
    case P_CH:
    case P_SET:
    case P_SET_N:
      return 0;
    case P_BRA:
    case P_REF:
      return re_isemptynd(nd->par.xy.x);
    case P_CAT:
      return re_isemptynd(nd->par.xy.x) && re_isemptynd(nd->par.xy.y.y);
    case P_ALT:
      return re_isemptynd(nd->par.xy.x) || re_isemptynd(nd->par.xy.y.y);
    case P_REP:
      return re_isemptynd(nd->par.xy.x) || !nd->par.xy.y.rp.min;
  }
}

static struct Renode *re_nrep(struct re_env *e, struct Renode *nd, int ng,
                              uint16_t min, uint16_t max) {
  struct Renode *rep = re_nnode(e, P_REP);
  if (max == RE_MAX_REP && re_isemptynd(nd))
    V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(INF_LOOP_M_EMP_STR));
  rep->par.xy.y.rp.ng = ng;
  rep->par.xy.y.rp.min = min;
  rep->par.xy.y.rp.max = max;
  rep->par.xy.x = nd;
  return rep;
}

static struct Renode *re_parser(struct re_env *e);

static struct Renode *re_parse_la(struct re_env *e) {
  struct Renode *nd;
  int min, max;
  switch (e->lookahead) {
    case '^':
      RE_NEXT(e);
      return re_nnode(e, P_BOL);
    case '$':
      RE_NEXT(e);
      return re_nnode(e, P_EOL);
    case L_EOS:
      RE_NEXT(e);
      return re_nnode(e, P_EOS);
    case L_WORD:
      RE_NEXT(e);
      return re_nnode(e, P_WORD);
    case L_WORD_N:
      RE_NEXT(e);
      return re_nnode(e, P_WORD_N);
  }

  switch (e->lookahead) {
    case L_CH:
      nd = re_nnode(e, P_CH);
      nd->par.c = e->curr_rune;
      RE_NEXT(e);
      break;
    case L_SET:
      nd = re_nnode(e, P_SET);
      nd->par.cp = e->curr_set;
      RE_NEXT(e);
      break;
    case L_SET_N:
      nd = re_nnode(e, P_SET_N);
      nd->par.cp = e->curr_set;
      RE_NEXT(e);
      break;
    case L_REF:
      nd = re_nnode(e, P_REF);
      if (!e->curr_rune || e->curr_rune > e->subexpr_num ||
          !e->sub[e->curr_rune])
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(INVALID_BACK_REF));
      nd->par.xy.y.n = e->curr_rune;
      nd->par.xy.x = e->sub[e->curr_rune];
      RE_NEXT(e);
      break;
    case '.':
      RE_NEXT(e);
      nd = re_nnode(e, P_ANY);
      break;
    case '(':
      RE_NEXT(e);
      nd = re_nnode(e, P_BRA);
      if (e->subexpr_num == RE_MAX_SUB)
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(TOO_MANY_CAPTURES));
      nd->par.xy.y.n = e->subexpr_num++;
      nd->par.xy.x = re_parser(e);
      e->sub[nd->par.xy.y.n] = nd;
      if (!RE_ACCEPT(e, ')'))
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(UNMATCH_LBR));
      break;
    case L_LA:
      RE_NEXT(e);
      nd = re_nnode(e, P_LA);
      nd->par.xy.x = re_parser(e);
      if (!RE_ACCEPT(e, ')'))
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(UNMATCH_LBR));
      break;
    case L_LA_CAP:
      RE_NEXT(e);
      nd = re_parser(e);
      if (!RE_ACCEPT(e, ')'))
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(UNMATCH_LBR));
      break;
    case L_LA_N:
      RE_NEXT(e);
      nd = re_nnode(e, P_LA_N);
      nd->par.xy.x = re_parser(e);
      if (!RE_ACCEPT(e, ')'))
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(UNMATCH_LBR));
      break;
    default:
      V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(SYNTAX_ERROR));
  }

  switch (e->lookahead) {
    case '*':
      RE_NEXT(e);
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), 0, RE_MAX_REP);
    case '+':
      RE_NEXT(e);
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), 1, RE_MAX_REP);
    case '?':
      RE_NEXT(e);
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), 0, 1);
    case L_COUNT:
      min = e->min_rep, max = e->max_rep;
      RE_NEXT(e);
      if (max < min)
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(INVALID_QUANTIFIER));
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), min, max);
  }
  return nd;
}

static uint8_t re_endofcat(Rune c, uint8_t re_flag) {
  switch (c) {
    case 0:
      return 1;
    case '|':
    case ')':
      if (re_flag) return 1;
  }
  return 0;
}

static struct Renode *re_parser(struct re_env *e) {
  struct Renode *alt = NULL, *cat, *nd;
  if (!re_endofcat(e->lookahead, e->flags.re)) {
    cat = re_parse_la(e);
    while (!re_endofcat(e->lookahead, e->flags.re)) {
      nd = cat;
      cat = re_nnode(e, P_CAT);
      cat->par.xy.x = nd;
      cat->par.xy.y.y = re_parse_la(e);
    }
    alt = cat;
  }
  if (e->lookahead == '|') {
    RE_NEXT(e);
    nd = alt;
    alt = re_nnode(e, P_ALT);
    alt->par.xy.x = nd;
    alt->par.xy.y.y = re_parser(e);
  }
  return alt;
}

static unsigned int re_nodelen(struct Renode *nd) {
  unsigned int n = 0;
  if (!nd) return 0;
  switch (nd->type) {
    case P_ALT:
      n = 2;
    case P_CAT:
      return re_nodelen(nd->par.xy.x) + re_nodelen(nd->par.xy.y.y) + n;
    case P_BRA:
    case P_LA:
    case P_LA_N:
      return re_nodelen(nd->par.xy.x) + 2;
    case P_REP:
      n = nd->par.xy.y.rp.max - nd->par.xy.y.rp.min;
      switch (nd->par.xy.y.rp.min) {
        case 0:
          if (!n) return 0;
          if (nd->par.xy.y.rp.max >= RE_MAX_REP)
            return re_nodelen(nd->par.xy.x) + 2;
        case 1:
          if (!n) return re_nodelen(nd->par.xy.x);
          if (nd->par.xy.y.rp.max >= RE_MAX_REP)
            return re_nodelen(nd->par.xy.x) + 1;
        default:
          n = 4;
          if (nd->par.xy.y.rp.max >= RE_MAX_REP) n++;
          return re_nodelen(nd->par.xy.x) + n;
      }
    default:
      return 1;
  }
}

static struct Reinst *re_newinst(struct Reprog *prog, int opcode) {
  memset(prog->end, 0, sizeof(struct Reinst));
  prog->end->opcode = opcode;
  return prog->end++;
}

static void re_compile(struct re_env *e, struct Renode *nd) {
  struct Reinst *inst, *split, *jump, *rep;
  unsigned int n;

  if (!nd) return;

  switch (nd->type) {
    case P_ALT:
      split = re_newinst(e->prog, I_SPLIT);
      re_compile(e, nd->par.xy.x);
      jump = re_newinst(e->prog, I_JUMP);
      re_compile(e, nd->par.xy.y.y);
      split->par.xy.x = split + 1;
      split->par.xy.y.y = jump + 1;
      jump->par.xy.x = e->prog->end;
      break;

    case P_ANY:
      re_newinst(e->prog, I_ANY);
      break;

    case P_BOL:
      re_newinst(e->prog, I_BOL);
      break;

    case P_BRA:
      inst = re_newinst(e->prog, I_LBRA);
      inst->par.n = nd->par.xy.y.n;
      re_compile(e, nd->par.xy.x);
      inst = re_newinst(e->prog, I_RBRA);
      inst->par.n = nd->par.xy.y.n;
      break;

    case P_CAT:
      re_compile(e, nd->par.xy.x);
      re_compile(e, nd->par.xy.y.y);
      break;

    case P_CH:
      inst = re_newinst(e->prog, I_CH);
      inst->par.c = nd->par.c;
      if (e->flags.re_i) inst->par.c = tolowerrune(nd->par.c);
      break;

    case P_EOL:
      re_newinst(e->prog, I_EOL);
      break;

    case P_EOS:
      re_newinst(e->prog, I_EOS);
      break;

    case P_LA:
      split = re_newinst(e->prog, I_LA);
      re_compile(e, nd->par.xy.x);
      re_newinst(e->prog, I_END);
      split->par.xy.x = split + 1;
      split->par.xy.y.y = e->prog->end;
      break;
    case P_LA_N:
      split = re_newinst(e->prog, I_LA_N);
      re_compile(e, nd->par.xy.x);
      re_newinst(e->prog, I_END);
      split->par.xy.x = split + 1;
      split->par.xy.y.y = e->prog->end;
      break;

    case P_REF:
      inst = re_newinst(e->prog, I_REF);
      inst->par.n = nd->par.xy.y.n;
      break;

    case P_REP:
      n = nd->par.xy.y.rp.max - nd->par.xy.y.rp.min;
      switch (nd->par.xy.y.rp.min) {
        case 0:
          if (!n) break;
          if (nd->par.xy.y.rp.max >= RE_MAX_REP) {
            split = re_newinst(e->prog, I_SPLIT);
            re_compile(e, nd->par.xy.x);
            jump = re_newinst(e->prog, I_JUMP);
            jump->par.xy.x = split;
            split->par.xy.x = split + 1;
            split->par.xy.y.y = e->prog->end;
            if (nd->par.xy.y.rp.ng) {
              split->par.xy.y.y = split + 1;
              split->par.xy.x = e->prog->end;
            }
            break;
          }
        case 1:
          if (!n) {
            re_compile(e, nd->par.xy.x);
            break;
          }
          if (nd->par.xy.y.rp.max >= RE_MAX_REP) {
            inst = e->prog->end;
            re_compile(e, nd->par.xy.x);
            split = re_newinst(e->prog, I_SPLIT);
            split->par.xy.x = inst;
            split->par.xy.y.y = e->prog->end;
            if (nd->par.xy.y.rp.ng) {
              split->par.xy.y.y = inst;
              split->par.xy.x = e->prog->end;
            }
            break;
          }
        default:
          inst = re_newinst(e->prog, I_REP_INI);
          inst->par.xy.y.rp.min = nd->par.xy.y.rp.min;
          inst->par.xy.y.rp.max = n;
          rep = re_newinst(e->prog, I_REP);
          split = re_newinst(e->prog, I_SPLIT);
          re_compile(e, nd->par.xy.x);
          jump = re_newinst(e->prog, I_JUMP);
          jump->par.xy.x = rep;
          rep->par.xy.x = e->prog->end;
          split->par.xy.x = split + 1;
          split->par.xy.y.y = e->prog->end;
          if (nd->par.xy.y.rp.ng) {
            split->par.xy.y.y = split + 1;
            split->par.xy.x = e->prog->end;
          }
          if (nd->par.xy.y.rp.max >= RE_MAX_REP) {
            inst = split + 1;
            split = re_newinst(e->prog, I_SPLIT);
            split->par.xy.x = inst;
            split->par.xy.y.y = e->prog->end;
            if (nd->par.xy.y.rp.ng) {
              split->par.xy.y.y = inst;
              split->par.xy.x = e->prog->end;
            }
            break;
          }
          break;
      }
      break;

    case P_SET:
      inst = re_newinst(e->prog, I_SET);
      inst->par.cp = nd->par.cp;
      break;
    case P_SET_N:
      inst = re_newinst(e->prog, I_SET_N);
      inst->par.cp = nd->par.cp;
      break;

    case P_WORD:
      re_newinst(e->prog, I_WORD);
      break;
    case P_WORD_N:
      re_newinst(e->prog, I_WORD_N);
      break;
  }
}

#ifdef RE_TEST
static void print_set(struct Reclass *cp) {
  struct Rerange *p;
  for (p = cp->spans; p < cp->end; p++) {
    printf("%s", p == cp->spans ? "'" : ",'");
    printf(
        p->s >= 32 && p->s < 127 ? "%c" : (p->s < 256 ? "\\x%02X" : "\\u%04X"),
        p->s);
    if (p->s != p->e) {
      printf(p->e >= 32 && p->e < 127 ? "-%c"
                                      : (p->e < 256 ? "-\\x%02X" : "-\\u%04X"),
             p->e);
    }
    printf("'");
  }
  printf("]");
}

static void node_print(struct Renode *nd) {
  if (!nd) {
    printf("Empty");
    return;
  }
  switch (nd->type) {
    case P_ALT:
      printf("{");
      node_print(nd->par.xy.x);
      printf(" | ");
      node_print(nd->par.xy.y.y);
      printf("}");
      break;
    case P_ANY:
      printf(".");
      break;
    case P_BOL:
      printf("^");
      break;
    case P_BRA:
      printf("(%d,", nd->par.xy.y.n);
      node_print(nd->par.xy.x);
      printf(")");
      break;
    case P_CAT:
      printf("{");
      node_print(nd->par.xy.x);
      printf(" & ");
      node_print(nd->par.xy.y.y);
      printf("}");
      break;
    case P_CH:
      printf(nd->par.c >= 32 && nd->par.c < 127 ? "'%c'" : "'\\u%04X'", nd->par.c);
      break;
    case P_EOL:
      printf("$");
      break;
    case P_EOS:
      printf("\\0");
      break;
    case P_LA:
      printf("LA(");
      node_print(nd->par.xy.x);
      printf(")");
      break;
    case P_LA_N:
      printf("LA_N(");
      node_print(nd->par.xy.x);
      printf(")");
      break;
    case P_REF:
      printf("\\%d", nd->par.xy.y.n);
      break;
    case P_REP:
      node_print(nd->par.xy.x);
      printf(nd->par.xy.y.rp.ng ? "{%d,%d}?" : "{%d,%d}", nd->par.xy.y.rp.min, nd->par.xy.y.rp.max);
      break;
    case P_SET:
      printf("[");
      print_set(nd->par.cp);
      break;
    case P_SET_N:
      printf("[^");
      print_set(nd->par.cp);
      break;
    case P_WORD:
      printf("\\b");
      break;
    case P_WORD_N:
      printf("\\B");
      break;
  }
}

static void program_print(struct Reprog *prog) {
  struct Reinst *inst;
  for (inst = prog->start; inst < prog->end; ++inst) {
    printf("%3d: ", inst - prog->start);
    switch (inst->opcode) {
      case I_END:
        puts("end");
        break;
      case I_ANY:
        puts(".");
        break;
      case I_ANYNL:
        puts(". | '\\r' | '\\n'");
        break;
      case I_BOL:
        puts("^");
        break;
      case I_CH:
        printf(inst->par.c >= 32 && inst->par.c < 127 ? "'%c'\n" : "'\\u%04X'\n", inst->par.c);
        break;
      case I_EOL:
        puts("$");
        break;
      case I_EOS:
        puts("\\0");
        break;
      case I_JUMP:
        printf("-->%d\n", inst->par.xy.x - prog->start);
        break;
      case I_LA:
        printf("la %d %d\n", inst->par.xy.x - prog->start, inst->par.xy.y.y - prog->start);
        break;
      case I_LA_N:
        printf("la_n %d %d\n", inst->par.xy.x - prog->start, inst->par.xy.y.y - prog->start);
        break;
      case I_LBRA:
        printf("( %d\n", inst->par.n);
        break;
      case I_RBRA:
        printf(") %d\n", inst->par.n);
        break;
      case I_SPLIT:
        printf("-->%d | -->%d\n", inst->par.xy.x - prog->start, inst->par.xy.y.y - prog->start);
        break;
      case I_REF:
        printf("\\%d\n", inst->par.n);
        break;
      case I_REP:
        printf("repeat -->%d\n", inst->par.xy.x - prog->start);
        break;
      case I_REP_INI:
        printf("init_rep %d %d\n", inst->par.xy.y.rp.min, inst->par.xy.y.rp.min + inst->par.xy.y.rp.max);
        break;
      case I_SET:
        printf("[");
        print_set(inst->par.cp);
        puts("");
        break;
      case I_SET_N:
        printf("[^");
        print_set(inst->par.cp);
        puts("");
        break;
      case I_WORD:
        puts("\\w");
        break;
      case I_WORD_N:
        puts("\\W");
        break;
    }
  }
}
#endif

struct Reprog *re_compiler(const char *pattern, struct v7_val_flags flags,
                           const char **p_err_msg) {
  struct re_env e;
  struct Renode *nd;
  struct Reinst *split, *jump;

  e.prog = reg_malloc(sizeof(struct Reprog));
  e.pstart = e.pend = reg_malloc(sizeof(struct Renode) * strlen(pattern) * 2);

  if (V7_EX_TRY_CATCH(e.catch_point)) {
    if (p_err_msg) *p_err_msg = e.err_msg;
    reg_free(e.pstart);
    reg_free(e.prog);
    return (struct Reprog *)-1;
  }

  e.src = pattern;
  e.sets_num = 0;
  e.subexpr_num = 1;
  e.flags = flags;
  memset(e.sub, 0, sizeof(e.sub));

  RE_NEXT(&e);
  nd = re_parser(&e);
  if (e.lookahead == ')')
    V7_EX_THROW(e.catch_point, e.err_msg, re_err_msg(UNMATCH_RBR));
  if (e.lookahead != 0)
    V7_EX_THROW(e.catch_point, e.err_msg, re_err_msg(SYNTAX_ERROR));

  e.prog->subexpr_num = e.subexpr_num;
  e.prog->start = e.prog->end =
      reg_malloc((re_nodelen(nd) + 6) * sizeof(struct Reinst));

  split = re_newinst(e.prog, I_SPLIT);
  split->par.xy.x = split + 3;
  split->par.xy.y.y = split + 1;
  re_newinst(e.prog, I_ANYNL);
  jump = re_newinst(e.prog, I_JUMP);
  jump->par.xy.x = split;
  re_newinst(e.prog, I_LBRA);
  re_compile(&e, nd);
  re_newinst(e.prog, I_RBRA);
  re_newinst(e.prog, I_END);

#ifdef RE_TEST
  node_print(nd);
  putchar('\n');
  program_print(e.prog);
#endif

  reg_free(e.pstart);

  if (p_err_msg) *p_err_msg = NULL;
  return e.prog;
}

void re_free(struct Reprog *prog) {
  if (prog) {
    reg_free(prog->start);
    reg_free(prog);
  }
}

static void re_newthread(struct Rethread *t, struct Reinst *pc,
                         const char *start, struct Resub *sub) {
  t->pc = pc;
  t->start = start;
  t->sub = *sub;
}

#define RE_NO_MATCH() \
  if (!(thr = 0)) continue

static uint8_t re_match(struct Reinst *pc, const char *start, const char *bol,
                        struct v7_val_flags flags, struct Resub *loot) {
  struct Rethread threads[RE_MAX_THREADS];
  struct Resub sub, tmpsub;
  Rune c, r;
  struct Rerange *p;
  uint16_t thr_num = 1;
  uint8_t thr;
  int i;

  /* queue initial thread */
  re_newthread(threads, pc, start, loot);

  /* run threads in stack order */
  do {
    pc = threads[--thr_num].pc;
    start = threads[thr_num].start;
    sub = threads[thr_num].sub;
    for (thr = 1; thr;) {
      switch (pc->opcode) {
        case I_END:
          memcpy(loot->sub, sub.sub, sizeof loot->sub);
          return 1;
        case I_ANY:
        case I_ANYNL:
          start += chartorune(&c, start);
          if (!c || (pc->opcode == I_ANY && isnewline(c))) RE_NO_MATCH();
          break;

        case I_BOL:
          if (start == bol) break;
          if (flags.re_m && isnewline(start[-1])) break;
          RE_NO_MATCH();
        case I_CH:
          start += chartorune(&c, start);
          if (c && (flags.re_i ? tolowerrune(c) : c) == pc->par.c) break;
          RE_NO_MATCH();
        case I_EOL:
          if (!*start) break;
          if (flags.re_m && isnewline(*start)) break;
          RE_NO_MATCH();
        case I_EOS:
          if (!*start) break;
          RE_NO_MATCH();

        case I_JUMP:
          pc = pc->par.xy.x;
          continue;

        case I_LA:
          if (re_match(pc->par.xy.x, start, bol, flags, &sub)) {
            pc = pc->par.xy.y.y;
            continue;
          }
          RE_NO_MATCH();
        case I_LA_N:
          tmpsub = sub;
          if (!re_match(pc->par.xy.x, start, bol, flags, &tmpsub)) {
            pc = pc->par.xy.y.y;
            continue;
          }
          RE_NO_MATCH();

        case I_LBRA:
          sub.sub[pc->par.n].start = start;
          break;

        case I_REF:
          i = sub.sub[pc->par.n].end - sub.sub[pc->par.n].start;
          if (flags.re_i) {
            int num = i;
            const char *s = start, *p = sub.sub[pc->par.n].start;
            Rune rr;
            for (; num && *s && *p; num--) {
              s += chartorune(&r, s);
              p += chartorune(&rr, p);
              if (tolowerrune(r) != tolowerrune(rr)) break;
            }
            if (num) RE_NO_MATCH();
          } else if (strncmp(start, sub.sub[pc->par.n].start, i))
            RE_NO_MATCH();
          if (i > 0) start += i;
          break;

        case I_REP:
          if (pc->par.xy.y.rp.min) {
            pc->par.xy.y.rp.min--;
            pc++;
          } else if (!pc->par.xy.y.rp.max--) {
            pc = pc->par.xy.x;
            continue;
          }
          break;

        case I_REP_INI:
          (pc + 1)->par.xy.y.rp.min = pc->par.xy.y.rp.min;
          (pc + 1)->par.xy.y.rp.max = pc->par.xy.y.rp.max;
          break;

        case I_RBRA:
          sub.sub[pc->par.n].end = start;
          break;

        case I_SET:
        case I_SET_N:
          start += chartorune(&c, start);
          if (!c) RE_NO_MATCH();

          i = 1;
          for (p = pc->par.cp->spans; i && p < pc->par.cp->end; p++)
            if (flags.re_i) {
              for (r = p->s; r <= p->e; ++r)
                if (tolowerrune(c) == tolowerrune(r)) {
                  i = 0;
                  break;
                }
            } else if (p->s <= c && c <= p->e)
              i = 0;

          if (pc->opcode == I_SET) i = !i;
          if (i) break;
          RE_NO_MATCH();

        case I_SPLIT:
          if (thr_num >= RE_MAX_THREADS) {
            fprintf(stderr, "re_match: backtrack overflow!\n");
            return 0;
          }
          re_newthread(&threads[thr_num++], pc->par.xy.y.y, start, &sub);
          pc = pc->par.xy.x;
          continue;

        case I_WORD:
        case I_WORD_N:
          i = (start > bol && iswordchar(start[-1]));
          if (iswordchar(start[0])) i = !i;
          if (pc->opcode == I_WORD_N) i = !i;
          if (i) break;
        /* RE_NO_MATCH(); */

        default:
          RE_NO_MATCH();
      }
      pc++;
    }
  } while (thr_num);
  return 0;
}

uint8_t re_exec(struct Reprog *prog, struct v7_val_flags flags,
                const char *start, struct Resub *loot) {
  struct Resub tmpsub;
  const char *st = start;

  if (loot) memset(loot, 0, sizeof(*loot));
  if (!flags.re_g || !loot) {
    if (!loot) loot = &tmpsub;
    loot->subexpr_num = prog->subexpr_num;
    return !re_match(prog->start, start, start, flags, loot);
  }
  while (re_match(prog->start, st, start, flags, &tmpsub)) {
    unsigned int i;
    st = tmpsub.sub[0].end;
    for (i = 0; i < prog->subexpr_num; i++) {
      struct re_tok *l = &loot->sub[loot->subexpr_num + i], *s = &tmpsub.sub[i];
      l->start = s->start;
      l->end = s->end;
    }
    loot->subexpr_num += prog->subexpr_num;
  }
  return !loot->subexpr_num;
}

V7_PRIVATE int re_rplc(struct Resub *loot, const char *src, const char *rstr,
                       struct Resub *dstsub) {
  int size = 0, n;
  Rune curr_rune;

  memset(dstsub, 0, sizeof(*dstsub));
  while (!(n = re_nextc(&curr_rune, &rstr, 1)) && curr_rune) {
    int sz;
    if (n < 0) return n;
    if (curr_rune == '$') {
      n = re_nextc(&curr_rune, &rstr, 1);
      if (n < 0) return n;
      switch (curr_rune) {
        case '&':
          sz = loot->sub[0].end - loot->sub[0].start;
          size += sz;
          dstsub->sub[dstsub->subexpr_num++] = loot->sub[0];
          break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
          int sbn = dec(curr_rune);
          if (0 == sbn && rstr[0] && isdigitrune(rstr[0])) {
            n = re_nextc(&curr_rune, &rstr, 1);
            if (n < 0) return n;
            sz = dec(curr_rune);
            sbn = sbn * 10 + sz;
          }
          if (sbn >= loot->subexpr_num) break;
          sz = loot->sub[sbn].end - loot->sub[sbn].start;
          size += sz;
          dstsub->sub[dstsub->subexpr_num++] = loot->sub[sbn];
          break;
        }
        case '`':
          sz = loot->sub[0].start - src;
          size += sz;
          dstsub->sub[dstsub->subexpr_num].start = src;
          dstsub->sub[dstsub->subexpr_num++].end = loot->sub[0].start;
          break;
        case '\'':
          sz = strlen(loot->sub[0].end);
          size += sz;
          dstsub->sub[dstsub->subexpr_num].start = loot->sub[0].end;
          dstsub->sub[dstsub->subexpr_num++].end = loot->sub[0].end + sz;
          break;
        case '$':
          size++;
          dstsub->sub[dstsub->subexpr_num].start = rstr - 1;
          dstsub->sub[dstsub->subexpr_num++].end = rstr;
          break;
        default:
          return BAD_CHAR_AFTER_USD;
      }
    } else {
      char tmps[300], *d = tmps;
      size += (sz = runetochar(d, &curr_rune));
      if (!dstsub->subexpr_num ||
          dstsub->sub[dstsub->subexpr_num - 1].end != rstr - sz) {
        dstsub->sub[dstsub->subexpr_num].start = rstr - sz;
        dstsub->sub[dstsub->subexpr_num++].end = rstr;
      } else
        dstsub->sub[dstsub->subexpr_num - 1].end = rstr;
    }
  }
  return size;
}

#ifdef RE_TEST

int re_replace(struct Resub *loot, const char *src, const char *rstr,
               char **dst) {
  struct Resub newsub;
  struct re_tok *t = newsub.sub;
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

static struct v7_val_flags get_flags(const char *ch) {
  struct v7_val_flags flags = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t rep = 1;
  for (; rep && *ch; ch++) {
    switch (*ch) {
      case 'g':
        flags.re_g = 1;
        rep = 1;
        break;
      case 'i':
        flags.re_i = 1;
        rep = 1;
        break;
      case 'm':
        flags.re_m = 1;
        rep = 1;
        break;
      case 'r':
        flags.re = 1;
        rep = 1;
        break;
      default:
        rep = 0;
    }
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
        obj->fl.fl.re_g = 1;
        break;
      case 'i':
        obj->fl.fl.re_i = 1;
        break;
      case 'm':
        obj->fl.fl.re_m = 1;
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
  v7_init_bool(result, this_obj->fl.fl.re_g);
}

V7_PRIVATE void Regex_ignoreCase(struct v7_val *this_obj, struct v7_val *arg,
                                 struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_bool(result, this_obj->fl.fl.re_i);
}

V7_PRIVATE void Regex_multiline(struct v7_val *this_obj, struct v7_val *arg,
                                struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_bool(result, this_obj->fl.fl.re_m);
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
    re_obj->v.str.prog = re_compiler(re_obj->v.str.buf, re_obj->fl.fl, NULL);
    if (-1 == (int)re_obj->v.str.prog) return V7_REGEXP_ERROR;
    if (NULL == re_obj->v.str.prog) return V7_OUT_OF_MEMORY;
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Regex_exec(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0], *arr = NULL;
  struct Resub sub;
  struct re_tok *ptok = sub.sub;

  if (cfa->num_args > 0) {
    char *begin;
    TRY(check_str_re_conv(v7, &arg, 0));
    begin = arg->v.str.buf;
    if (cfa->this_obj->fl.fl.re_g)
      begin = utfnshift(begin, cfa->this_obj->v.str.lastIndex);
    TRY(regex_check_prog(cfa->this_obj));
    if (!re_exec(cfa->this_obj->v.str.prog, cfa->this_obj->fl.fl, begin,
                 &sub)) {
      int i;
      arr = v7_push_new_object(v7);
      v7_set_class(arr, V7_CLASS_ARRAY);
      for (i = 0; i < sub.subexpr_num; i++, ptok++)
        v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, ptok->start,
                                  ptok->end - ptok->start, 1));
      if (cfa->this_obj->fl.fl.re_g)
        cfa->this_obj->v.str.lastIndex = utfnlen(begin, sub.sub->end - begin);
      return V7_OK;
    }
  }
  TRY(v7_make_and_push(v7, V7_TYPE_NULL));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Regex_test(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0];
  struct Resub sub;
  int found = 0;

  if (cfa->num_args > 0) {
    TRY(check_str_re_conv(v7, &arg, 0));
    TRY(regex_check_prog(cfa->this_obj));
    found = !re_exec(cfa->this_obj->v.str.prog, cfa->this_obj->fl.fl,
                     arg->v.str.buf, &sub);
  }
  v7_push_bool(v7, found);
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
