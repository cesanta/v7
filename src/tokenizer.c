#include "internal.h"

// NOTE(lsm): Must be in the same order as enum for keywords
struct { const char *p; int len; } s_keywords[] = {
  {"break", 5}, {"case", 4}, {"catch", 4}, {"continue", 8}, {"debugger", 8},
  {"default", 7}, {"delete", 6}, {"do", 2}, {"else", 4}, {"false", 5},
  {"finally", 7}, {"for", 3}, {"function", 8}, {"if", 2}, {"in", 2},
  {"instanceof", 10}, {"new", 3}, {"null", 4}, {"return", 6}, {"switch", 6},
  {"this", 4}, {"throw", 5}, {"true", 4}, {"try", 3}, {"typeof", 6},
  {"undefined", 9}, {"var", 3}, {"void", 4}, {"while", 5}, {"with", 4}
};

// Move ptr to the next token, skipping comments and whitespaces.
// Return number of new line characters detected.
static int skip_to_next_tok(const char **ptr) {
  const char *s = *ptr, *p = NULL;
  int num_lines = 0;

  while (s != p && *s != '\0' && (isspace((unsigned char) *s) || *s == '/')) {
    p = s;
    while (*s != '\0' && isspace((unsigned char) *s)) {
      if (*s == '\n') num_lines++;
      s++;
    }
    if (s[0] == '/' && s[1] == '/') {
      s += 2;
      while (s[0] != '\0' && s[0] != '\n') s++;
    }
    if (s[0] == '/' && s[1] == '*') {
      s += 2;
      while (s[0] != '\0' && !(s[-1] == '/' && s[-2] == '*')) {
        if (s[0] == '\n') num_lines++;
        s++;
      }
    }
  }
  *ptr = s;

  return num_lines;
}

// TODO(lsm): use lookup table to speed it up
static int is_ident_char(int ch) {
  return ch == '$' || ch == '_' || isalnum(ch);
}

static void ident(const char **s) {
  while (is_ident_char((unsigned char) s[0][0])) (*s)++;
}

static enum v7_tok kw(const char *s, int len, int ntoks, enum v7_tok tok) {
  int i;

  for (i = 0; i < ntoks; i++) {
    if (s_keywords[(tok - TOK_BREAK) + i].len == len &&
        memcmp(s_keywords[(tok - TOK_BREAK) + i].p + 1, s + 1,
               len - 1) == 0) break;
  }

  return i == ntoks ? TOK_IDENTIFIER :(enum v7_tok) (tok + i);
}

static enum v7_tok punct1(const char **s, int ch1,
                          enum v7_tok tok1, enum v7_tok tok2) {

  (*s)++;
  if (s[0][0] == ch1) {
    (*s)++; return tok1;
  } else {
    return tok2;
  }
}

static enum v7_tok punct2(const char **s, int ch1, enum v7_tok tok1, int ch2,
                          enum v7_tok tok2, enum v7_tok tok3) {

  if (s[0][1] == ch1 && s[0][2] == ch2) {
    (*s) += 3;
    return tok2;
  }

  return punct1(s, ch1, tok1, tok3);
}

static enum v7_tok punct3(const char **s, int ch1, enum v7_tok tok1, int ch2,
                          enum v7_tok tok2, enum v7_tok tok3) {
  (*s)++;
  if (s[0][0] == ch1) {
    (*s)++;
    return tok1;
  } else if (s[0][0] == ch2) {
    (*s)++;
    return tok2;
  } else {
    return tok3;
  }
}

static void parse_number(const char *s, const char **end, double *num) {
  *num = strtod(s, (char **) end);
}

static enum v7_tok parse_str_literal(const char **p) {
  const char *s = *p;
  int quote = *s++;

  // Scan string literal into the buffer, handle escape sequences
  while (*s != quote && *s != '\0') {
    switch (*s) {
      case '\\':
        s++;
        switch (*s) {
          case 'b': case 'f': case 'n': case 'r': case 't':
          case 'v': case '\\': s++; break;
          default: if (*s == quote) s++; break;
        }
        break;
      default: break;
    }
    s++;
  }

  if (*s == quote) {
    s++;
    *p = s;
    return TOK_STRING_LITERAL;
  } else {
    return TOK_END_OF_INPUT;
  }
}

static enum v7_tok get_tok(const char **s, double *n) {
  const char *p = *s;

  switch (*p) {
    // Letters
    case 'a': ident(s); return TOK_IDENTIFIER;
    case 'b': ident(s); return kw(p, *s - p, 1, TOK_BREAK);
    case 'c': ident(s); return kw(p, *s - p, 2, TOK_CASE);
    case 'd': ident(s); return kw(p, *s - p, 4, TOK_DEBUGGER);
    case 'e': ident(s); return kw(p, *s - p, 1, TOK_ELSE);
    case 'f': ident(s); return kw(p, *s - p, 4, TOK_FALSE);
    case 'g':
    case 'h': ident(s); return TOK_IDENTIFIER;
    case 'i': ident(s); return kw(p, *s - p, 3, TOK_IF);
    case 'j':
    case 'k':
    case 'l':
    case 'm': ident(s); return TOK_IDENTIFIER;
    case 'n': ident(s); return kw(p, *s - p, 2, TOK_NEW);
    case 'o':
    case 'p':
    case 'q': ident(s); return TOK_IDENTIFIER;
    case 'r': ident(s); return kw(p, *s - p, 1, TOK_RETURN);
    case 's': ident(s); return kw(p, *s - p, 1, TOK_SWITCH);
    case 't': ident(s); return kw(p, *s - p, 5, TOK_THIS);
    case 'u': ident(s); return kw(p, *s - p, 1, TOK_UNDEFINED);
    case 'v': ident(s); return kw(p, *s - p, 2, TOK_VAR);
    case 'w': ident(s); return kw(p, *s - p, 2, TOK_WHILE);
    case 'x':
    case 'y':
    case 'z': ident(s); return TOK_IDENTIFIER;

    case '_': case '$':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y':
    case 'Z': ident(s); return TOK_IDENTIFIER;

    // Numbers
    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8':
    case '9': parse_number(p, s, n); return TOK_NUMBER;

    // String literals
    case '\'':
    case '"': return parse_str_literal(s);

    // Punctuators
    case '=': return punct2(s, '=', TOK_EQ, '=', TOK_EQ_EQ, TOK_ASSIGN);
    case '!': return punct2(s, '=', TOK_NE, '=', TOK_NE_NE, TOK_NOT);

    case '%': return punct1(s, '=', TOK_REM_ASSIGN, TOK_REM);
    case '*': return punct1(s, '=', TOK_MUL_ASSIGN, TOK_MUL);
    case '/': return punct1(s, '=', TOK_DIV_ASSIGN, TOK_DIV);
    case '^': return punct1(s, '=', TOK_XOR_ASSIGN, TOK_XOR);

    case '+': return punct3(s, '+', TOK_PLUS_PLUS, '=',
                            TOK_PLUS_ASSIGN, TOK_PLUS);
    case '-': return punct3(s, '-', TOK_MINUS_MINUS, '=',
                            TOK_MINUS_ASSIGN, TOK_MINUS);
    case '&': return punct3(s, '&', TOK_LOGICAL_AND, '=',
                            TOK_LOGICAL_AND_ASSING, TOK_AND);
    case '|': return punct3(s, '|', TOK_LOGICAL_OR, '=',
                            TOK_LOGICAL_OR_ASSING, TOK_OR);

    case '<':
      if (s[0][1] == '=') { (*s) += 2; return TOK_LE; }
      return punct2(s, '<', TOK_LSHIFT, '=', TOK_LSHIFT_ASSIGN, TOK_LT);
    case '>':
      if (s[0][1] == '=') { (*s) += 2; return TOK_GE; }
      return punct2(s, '<', TOK_RSHIFT, '=', TOK_RSHIFT_ASSIGN, TOK_GT);

    case '{': (*s)++; return TOK_OPEN_CURLY;
    case '}': (*s)++; return TOK_CLOSE_CURLY;
    case '(': (*s)++; return TOK_OPEN_PAREN;
    case ')': (*s)++; return TOK_CLOSE_PAREN;
    case '[': (*s)++; return TOK_OPEN_BRACKET;
    case ']': (*s)++; return TOK_CLOSE_BRACKET;
    case '.': (*s)++; return TOK_DOT;
    case ';': (*s)++; return TOK_SEMICOLON;
    case ':': (*s)++; return TOK_COLON;
    case '?': (*s)++; return TOK_QUESTION;
    case '~': (*s)++; return TOK_TILDA;
    case ',': (*s)++; return TOK_COMMA;

    default: return TOK_END_OF_INPUT;
  }
}

V7_PRIVATE enum v7_tok lookahead(const struct v7 *v7) {
  const char *s = v7->pstate.pc;
  double d;
  return get_tok(&s, &d);
}

V7_PRIVATE enum v7_tok next_tok(struct v7 *v7) {
  v7->pstate.line_no += skip_to_next_tok(&v7->pstate.pc);
  v7->tok = v7->pstate.pc;
  v7->cur_tok = get_tok(&v7->pstate.pc, &v7->cur_tok_dbl);
  v7->tok_len = v7->pstate.pc - v7->tok;
  v7->pstate.line_no += skip_to_next_tok(&v7->pstate.pc);
  TRACE_CALL("==tok=> %d [%.*s] %d\n", v7->cur_tok, (int) v7->tok_len, v7->tok,
             v7->pstate.line_no);
  return v7->cur_tok;
}

#ifdef TEST_RUN
int main(void) {
  const char *src = "for (var fo++ = -1; /= <= 1.17; x<<) { == <<=, 'x')} "
    "Infinity %=x<<=2";
  enum v7_tok tok;
  double num;
  const char *p = src;

  skip_to_next_tok(&src);
  while ((tok = get_tok(&src, &num)) != TOK_END_OF_INPUT) {
    printf("%d [%.*s]\n", tok, (int) (src - p), p);
    skip_to_next_tok(&src);
    p = src;
  }
  printf("%d [%.*s]\n", tok, (int) (src - p), p);

  return 0;
}
#endif
