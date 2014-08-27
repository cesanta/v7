#include "internal.h"

// NOTE(lsm): Must be in the same order as enum for keywords
struct v7_vec s_keywords[] = {
  {"break", 5}, {"case", 4}, {"catch", 4}, {"continue", 8}, {"debugger", 8},
  {"default", 7}, {"delete", 6}, {"do", 2}, {"else", 4}, {"false", 5},
  {"finally", 7}, {"for", 3}, {"function", 8}, {"if", 2}, {"in", 2},
  {"instanceof", 10}, {"new", 3}, {"null", 4}, {"return", 6}, {"switch", 6},
  {"this", 4}, {"throw", 5}, {"true", 4}, {"try", 3}, {"typeof", 6},
  {"undefined", 9}, {"var", 3}, {"void", 4}, {"while", 5}, {"with", 4}
};

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

static int is_ident_char(int ch) {
  return ch == '$' || ch == '_' || isalnum(ch);
}

static void ident(const char *src, struct v7_vec *vec) {
  while (is_ident_char((unsigned char) src[vec->len])) vec->len++;
}

static enum v7_tok kw(const struct v7_vec *vec, ...) {
  enum v7_tok tok;
  va_list ap;

  va_start(ap, vec);
  while ((tok = (enum v7_tok) va_arg(ap, int)) > 0) {
    const struct v7_vec *k = &s_keywords[tok - TOK_BREAK];
    if (k->len == vec->len &&
      memcmp(vec->p + 1, k->p + 1, k->len - 1) == 0) break;

  }
  va_end(ap);

  return tok > 0 ? tok : TOK_IDENTIFIER;
}

static enum v7_tok punct1(const char *s, struct v7_vec *vec,
  int ch1, enum v7_tok tok1, enum v7_tok tok2) {

  if (s[1] == ch1) {
    vec->len++;
    return tok1;
  }

  return tok2;
}

static enum v7_tok punct2(const char *s, struct v7_vec *vec,
  int ch1, enum v7_tok tok1, int ch2, enum v7_tok tok2, enum v7_tok tok3) {

  if (s[1] == ch1 && s[2] == ch2) {
    vec->len += 2;
    return tok2;
  }

  return punct1(s, vec, ch1, tok1, tok3);
}

static enum v7_tok punct3(const char *s, struct v7_vec *vec,
  int ch1, enum v7_tok tok1, int ch2, enum v7_tok tok2, enum v7_tok tok3) {

  if (s[1] == ch1) {
    vec->len++;
    return tok1;
  } else if (s[1] == ch2) {
    vec->len++;
    return tok2;
  }
  return tok3;
}

static int parse_number(const char *s, double *num) {
  char *end;
  double value = strtod(s, &end);
  if (num) *num = value;
  return (int) (end - s);
}

static int parse_str_literal(const char *s) {
  int len = 0, quote = *s++;

  // Scan string literal into the buffer, handle escape sequences
  while (s[len] != quote && s[len] != '\0') {
    switch (s[len]) {
      case '\\':
        len++;
        switch (s[len]) {
          case 'b': case 'f': case 'n': case 'r': case 't':
          case 'v': case '\\': len++; break;
          default: if (s[len] == quote) len++; break;
        }
        break;
      default: break;
    }
    len++;
  }

  return len;
}

V7_PRIVATE enum v7_tok next_tok(const char *s, struct v7_vec *vec,double *n) {

  skip_to_next_tok(&s);
  vec->p = s;
  vec->len = 1;

  switch (s[0]) {
    // Letters
    case 'a': ident(s, vec); return TOK_IDENTIFIER;
    case 'b': ident(s, vec); return kw(vec, TOK_BREAK, 0);
    case 'c': ident(s, vec); return kw(vec, TOK_CATCH, TOK_CASE, 0);
    case 'd': ident(s, vec); return kw(vec, TOK_DEBUGGER, TOK_DEFAULT,
                                       TOK_DELETE, TOK_DO, 0);
    case 'e': ident(s, vec); return kw(vec, TOK_ELSE, 0);
    case 'f': ident(s, vec); return kw(vec, TOK_FALSE, TOK_FINALLY, TOK_FOR,
                                       TOK_FUNCTION, 0);
    case 'g':
    case 'h': ident(s, vec); return TOK_IDENTIFIER;
    case 'i': ident(s, vec); return kw(vec, TOK_IF, TOK_IN, TOK_INSTANCEOF, 0);
    case 'j':
    case 'k':
    case 'l':
    case 'm': ident(s, vec); return TOK_IDENTIFIER;
    case 'n': ident(s, vec); return kw(vec, TOK_NEW, TOK_NULL, 0);
    case 'o':
    case 'p':
    case 'q': ident(s, vec); return TOK_IDENTIFIER;
    case 'r': ident(s, vec); return kw(vec, TOK_RETURN, 0);
    case 's': ident(s, vec); return kw(vec, TOK_SWITCH, 0);
    case 't': ident(s, vec); return kw(vec, TOK_THIS, TOK_THROW, TOK_TRUE,
                                       TOK_TRY, TOK_TYPEOF, 0);
    case 'u': ident(s, vec); return kw(vec, TOK_UNDEFINED, 0);
    case 'v': ident(s, vec); return kw(vec, TOK_VAR, TOK_VOID, 0);
    case 'w': ident(s, vec); return kw(vec, TOK_WHILE, TOK_WITH, 0);
    case 'x':
    case 'y':
    case 'z': ident(s, vec); return TOK_IDENTIFIER;

    case '_': case '$':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y':
    case 'Z': ident(s, vec); return TOK_IDENTIFIER;

    // Numbers
    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8':
    case '9': vec->len = parse_number(s, n); return TOK_NUMBER;

    // String literals
    case '\'':
    case '"': vec->len = parse_str_literal(s); return TOK_STRING_LITERAL;

    // Punctuators
    case '=': return punct2(s, vec, '=', TOK_EQ, '=', TOK_EQ_EQ, TOK_ASSIGN);
    case '!': return punct2(s, vec, '=', TOK_NE, '=', TOK_NE_NE, TOK_NOT);

    case '%': return punct1(s, vec, '=', TOK_REM_ASSIGN, TOK_REM);
    case '*': return punct1(s, vec, '=', TOK_MUL_ASSIGN, TOK_MUL);
    case '/': return punct1(s, vec, '=', TOK_DIV_ASSIGN, TOK_DIV);
    case '^': return punct1(s, vec, '=', TOK_XOR_ASSIGN, TOK_XOR);

    case '+': return punct3(s, vec, '+', TOK_PLUS_PLUS, '=',
                            TOK_PLUS_ASSING, TOK_PLUS);
    case '-': return punct3(s, vec, '-', TOK_MINUS_MINUS, '=',
                            TOK_MINUS_ASSING, TOK_MINUS);
    case '&': return punct3(s, vec, '&', TOK_LOGICAL_AND, '=',
                            TOK_LOGICAL_AND_ASSING, TOK_AND);
    case '|': return punct3(s, vec, '|', TOK_LOGICAL_OR, '=',
                            TOK_LOGICAL_OR_ASSING, TOK_OR);

    case '<':
      if (s[1] == '=') { vec->len = 2; return TOK_LE; }
      return punct2(s, vec, '<', TOK_LSHIFT, '=', TOK_LSHIFT_ASSIGN, TOK_LT);
    case '>':
      if (s[1] == '=') { vec->len = 2; return TOK_GE; }
      return punct2(s, vec, '<', TOK_RSHIFT, '=', TOK_RSHIFT_ASSIGN, TOK_GT);

    case '{': return TOK_OPEN_CURLY;
    case '}': return TOK_CLOSE_CURLY;
    case '(': return TOK_OPEN_PAREN;
    case ')': return TOK_CLOSE_PAREN;
    case '[': return TOK_OPEN_BRACKET;
    case ']': return TOK_CLOSE_BRACKET;
    case '.': return TOK_DOT;
    case ';': return TOK_SEMICOLON;
    case ':': return TOK_COLON;
    case '?': return TOK_QUESTION;
    case '~': return TOK_TILDA;

    default: vec->len = 0; return TOK_END_OF_INPUT;
  }
}

#ifdef TEST_RUN
int main(void) {
  const char *src = "for (var fo = 1; fore < 1.17; foo++) { print(23, 'x')} ";
  struct v7_vec vec;
  enum v7_tok tok;
  double num;

  while ((tok = next_tok(src, &vec, &num)) != TOK_END_OF_INPUT) {
    printf("%d [%.*s]\n", tok, vec.len, vec.p);
    src = vec.p + vec.len;
  }
  printf("%d [%.*s]\n", tok, vec.len, vec.p);

  return 0;
}
#endif
