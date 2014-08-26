#include "internal.h"
#include <ctype.h>

struct v7_vec {
  const char *p;
  int len;
};

enum v7_tok {
  TOK_END_OF_INPUT, TOK_NUMBER, TOK_STRING_LITERAL, TOK_IDENTIFIER,

  // Punctuators
  TOK_OPEN_CURLY, TOK_CLOSE_CURLY, TOK_OPEN_PAREN, TOK_CLOSE_PAREN,
  TOK_OPEN_BRACKET, TOK_CLOSE_BRACKET, TOK_DOT, TOK_COLON, TOK_SEMICOLON,
  TOK_COMMA, TOK_EQ_EQ, TOK_EQ, TOK_ASSIGN,

  // Keywords
  TOK_BREAK, TOK_CASE, TOK_CATCH, TOK_CONTINUE, TOK_DEBUGGER, TOK_DEFAULT,
  TOK_DELETE, TOK_DO, TOK_ELSE, TOK_FALSE, TOK_FINALLY, TOK_FOR, TOK_FUNCTION,
  TOK_IF, TOK_IN, TOK_INSTANCEOF, TOK_NEW, TOK_NULL, TOK_RETURN, TOK_SWITCH,
  TOK_THIS, TOK_THROW, TOK_TRUE, TOK_TRY, TOK_TYPEOF, TOK_UNDEFINED, TOK_VAR,
  TOK_VOID, TOK_WHILE, TOK_WITH,

  // TODO(lsm): process reserved words too
  TOK_CLASS, TOK_ENUM, TOK_EXTENDS, TOK_SUPER, TOK_CONST, TOK_EXPORT,
  TOK_IMPORT, TOK_IMPLEMENTS, TOK_LET, TOK_PRIVATE, TOK_PUBLIC,
  TOK_INTERFACE, TOK_PACKAGE, TOK_PROTECTED, TOK_STATIC, TOK_YIELD,

  NUM_TOKENS
};

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

#if 0
static const char *s_punctuators[] = {
  "{", "}", "(", ")", "[", "]", ".", ";", ",",

  "<", ">", "<=", ">=", "==",
  "!=", "===", "!==", "+", "-", "*", "%", "++", "--", "<<", ">>", ">>>", "&",
  "|", "^", "!", "~", "&&", "||", "?", ":", "=", "+=", "-=", "*=", "%=", "<<=",
  ">>=", ">>>=", "&=", "|=", "^=", "/", "/="
};
#endif

static int is_valid_identifier_char(int ch) {
  return ch == '$' || ch == '_' || isalnum(ch);
}

static enum v7_tok ident(const char *src, struct v7_vec *vec) {
  while (is_valid_identifier_char((unsigned char) src[vec->len])) vec->len++;
  return TOK_IDENTIFIER;
}

static int eq(const char *src, struct v7_vec *vec, const char *s, int len) {
  ident(src, vec);
  return len == vec->len && memcmp(src + 1, s + 1, len - 1) == 0;
}

static enum v7_tok kw(const struct v7_vec *vec, ...) {
  enum v7_tok tok;
  va_list ap;

  va_start(ap, vec);
  while ((tok = va_arg(ap, int)) > 0) {
    const struct v7_vec *k = &s_keywords[tok - TOK_BREAK];
    if (k->len == vec->len &&
      memcmp(vec->p + 1, k->p + 1, k->len - 1) == 0) break;

  }
  va_end(ap);

  return tok > 0 ? tok : TOK_IDENTIFIER;
}

V7_PRIVATE enum v7_tok get_next_token(const char *s, struct v7_vec *vec) {
  vec->p = s;
  vec->len = 1;

  switch (s[0]) {
    case '=': return eq(s, vec, "===", 3) ? TOK_EQ_EQ :
    eq(s, vec, "==", 2) ? TOK_EQ : TOK_ASSIGN;

    // Letters
    case 'a': ident(s, vec); return TOK_IDENTIFIER;
    case 'b': ident(s, vec); return kw(vec, TOK_BREAK, 0);
    case 'c': ident(s, vec); return kw(vec, TOK_CATCH, TOK_CASE, 0);
    case 'd': ident(s, vec); return kw(vec, TOK_DEBUGGER, TOK_DEFAULT, TOK_DELETE, TOK_DO, 0);
    case 'e': ident(s, vec); return kw(vec, TOK_ELSE, 0);
    case 'f': ident(s, vec); return kw(vec, TOK_FALSE, TOK_FINALLY, TOK_FOR, TOK_FUNCTION, 0);
    case 'g': return ident(s, vec);
    case 'h': return ident(s, vec);
    case 'i': ident(s, vec); return kw(vec, TOK_IF, TOK_IN, TOK_INSTANCEOF, 0);
    case 'j': return ident(s, vec);
    case 'k': return ident(s, vec);
    case 'l': return ident(s, vec);
    case 'm': return ident(s, vec);
    case 'n': ident(s, vec); return kw(vec, TOK_NEW, TOK_NULL, 0);
    case 'o': return ident(s, vec);
    case 'p': return ident(s, vec);
    case 'q': return ident(s, vec);
    case 'r': ident(s, vec); return kw(vec, TOK_RETURN, 0);
    case 's': ident(s, vec); return kw(vec, TOK_SWITCH, 0);
    case 't': ident(s, vec); return kw(vec, TOK_THIS, TOK_THROW, TOK_TRUE, TOK_TRY, TOK_TYPEOF, 0);
    case 'u': ident(s, vec); return kw(vec, TOK_UNDEFINED, 0);
    case 'v': ident(s, vec); return kw(vec, TOK_VAR, TOK_VOID, 0);
    case 'w': ident(s, vec); return kw(vec, TOK_WHILE, TOK_WITH, 0);
    case 'x': return ident(s, vec);
    case 'y': return ident(s, vec);
    case 'z': return ident(s, vec);

    // Digits
    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9': return TOK_NUMBER;

    case '{': return TOK_OPEN_CURLY;
    case '}': return TOK_CLOSE_CURLY;
    case '(': return TOK_OPEN_PAREN;
    case ')': return TOK_CLOSE_PAREN;
    case '[': return TOK_OPEN_BRACKET;
    case ']': return TOK_CLOSE_BRACKET;
    case '.': return TOK_DOT;
    case ';': return TOK_SEMICOLON;
    case ':': return TOK_COLON;

    default: vec->len = 0; return TOK_END_OF_INPUT;
  }
}

int main(void) {
  const char *src = "for (var fo = 1; fore < 12; foo++) { print(23, 'x')} ";
  struct v7_vec vec;
  enum v7_tok tok;

  skip_to_next_tok(&src);
  while ((tok = get_next_token(src, &vec)) != TOK_END_OF_INPUT) {
    printf("%d [%.*s]\n", tok, vec.len, vec.p);
    src = vec.p + vec.len;
    skip_to_next_tok(&src);
  }
  printf("%d [%.*s]\n", tok, vec.len, vec.p);

  return 0;
}
