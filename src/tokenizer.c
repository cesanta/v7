/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

/*
 * NOTE(lsm): Must be in the same order as enum for keywords. See comment
 * for function get_tok() for rationale for that.
 */
static struct v7_vec s_keywords[] = {
    V7_VEC("break"),      V7_VEC("case"),      V7_VEC("catch"),
    V7_VEC("continue"),   V7_VEC("debugger"),  V7_VEC("default"),
    V7_VEC("delete"),     V7_VEC("do"),        V7_VEC("else"),
    V7_VEC("false"),      V7_VEC("finally"),   V7_VEC("for"),
    V7_VEC("function"),   V7_VEC("if"),        V7_VEC("in"),
    V7_VEC("instanceof"), V7_VEC("new"),       V7_VEC("null"),
    V7_VEC("return"),     V7_VEC("switch"),    V7_VEC("this"),
    V7_VEC("throw"),      V7_VEC("true"),      V7_VEC("try"),
    V7_VEC("typeof"),     V7_VEC("var"),       V7_VEC("void"),
    V7_VEC("while"),      V7_VEC("with")};

/*
 * Move ptr to the next token, skipping comments and whitespaces.
 * Return number of new line characters detected.
 */
V7_PRIVATE int skip_to_next_tok(const char **ptr) {
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

/* TODO(lsm): use lookup table to speed it up */
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
        memcmp(s_keywords[(tok - TOK_BREAK) + i].p + 1, s + 1, len - 1) == 0)
      break;
  }

  return i == ntoks ? TOK_IDENTIFIER : (enum v7_tok)(tok + i);
}

static enum v7_tok punct1(const char **s, int ch1, enum v7_tok tok1,
                          enum v7_tok tok2) {
  (*s)++;
  if (s[0][0] == ch1) {
    (*s)++;
    return tok1;
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

  /* Scan string literal into the buffer, handle escape sequences */
  while (*s != quote && *s != '\0') {
    switch (*s) {
      case '\\':
        s++;
        switch (*s) {
          case 'b':
          case 'f':
          case 'n':
          case 'r':
          case 't':
          case 'v':
          case '\\':
            s++;
            break;
          default:
            if (*s == quote) s++;
            break;
        }
        break;
      default:
        break;
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


/*
 * This function is the heart of the tokenizer.
 * Organized as a giant switch statement.
 * Switch statement is by the first character of the input stream. If first
 * character begins with a letter, it could be either keyword or identifier.
 * get_tok() calls ident() which shifts `s` pointer to the end of the word.
 * Now, tokenizer knows that the word begins at `p` and ends at `s`.
 * It calls function kw() to scan over the keywords that start with `p[0]`
 * letter. Therefore, keyword tokens and keyword strings must be in the
 * same order, to let kw() function work properly.
 * If kw() finds a keyword match, it returns keyword token.
 * Otherwise, it returns TOK_IDENTIFIER.
 */
V7_PRIVATE enum v7_tok get_tok(const char **s, double *n) {
  const char *p = *s;

  switch (*p) {
    /* Letters */
    case 'a':
      ident(s);
      return TOK_IDENTIFIER;
    case 'b':
      ident(s);
      return kw(p, *s - p, 1, TOK_BREAK);
    case 'c':
      ident(s);
      return kw(p, *s - p, 3, TOK_CASE);
    case 'd':
      ident(s);
      return kw(p, *s - p, 4, TOK_DEBUGGER);
    case 'e':
      ident(s);
      return kw(p, *s - p, 1, TOK_ELSE);
    case 'f':
      ident(s);
      return kw(p, *s - p, 4, TOK_FALSE);
    case 'g':
    case 'h':
      ident(s);
      return TOK_IDENTIFIER;
    case 'i':
      ident(s);
      return kw(p, *s - p, 3, TOK_IF);
    case 'j':
    case 'k':
    case 'l':
    case 'm':
      ident(s);
      return TOK_IDENTIFIER;
    case 'n':
      ident(s);
      return kw(p, *s - p, 2, TOK_NEW);
    case 'o':
    case 'p':
    case 'q':
      ident(s);
      return TOK_IDENTIFIER;
    case 'r':
      ident(s);
      return kw(p, *s - p, 1, TOK_RETURN);
    case 's':
      ident(s);
      return kw(p, *s - p, 1, TOK_SWITCH);
    case 't':
      ident(s);
      return kw(p, *s - p, 5, TOK_THIS);
    case 'u':
      ident(s);
      return TOK_IDENTIFIER;
    case 'v':
      ident(s);
      return kw(p, *s - p, 2, TOK_VAR);
    case 'w':
      ident(s);
      return kw(p, *s - p, 2, TOK_WHILE);
    case 'x':
    case 'y':
    case 'z':
      ident(s);
      return TOK_IDENTIFIER;

    case '_':
    case '$':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
      ident(s);
      return TOK_IDENTIFIER;

    /* Numbers */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      parse_number(p, s, n);
      return TOK_NUMBER;

    /* String literals */
    case '\'':
    case '"':
      return parse_str_literal(s);

    /* Punctuators */
    case '=':
      return punct2(s, '=', TOK_EQ, '=', TOK_EQ_EQ, TOK_ASSIGN);
    case '!':
      return punct2(s, '=', TOK_NE, '=', TOK_NE_NE, TOK_NOT);

    case '%':
      return punct1(s, '=', TOK_REM_ASSIGN, TOK_REM);
    case '*':
      return punct1(s, '=', TOK_MUL_ASSIGN, TOK_MUL);
    case '/':
      return punct1(s, '=', TOK_DIV_ASSIGN, TOK_DIV);
    case '^':
      return punct1(s, '=', TOK_XOR_ASSIGN, TOK_XOR);

    case '+':
      return punct3(s, '+', TOK_PLUS_PLUS, '=', TOK_PLUS_ASSIGN, TOK_PLUS);
    case '-':
      return punct3(s, '-', TOK_MINUS_MINUS, '=', TOK_MINUS_ASSIGN, TOK_MINUS);
    case '&':
      return punct3(s, '&', TOK_LOGICAL_AND, '=', TOK_AND_ASSIGN, TOK_AND);
    case '|':
      return punct3(s, '|', TOK_LOGICAL_OR, '=', TOK_OR_ASSIGN, TOK_OR);

    case '<':
      if (s[0][1] == '=') {
        (*s) += 2;
        return TOK_LE;
      }
      return punct2(s, '<', TOK_LSHIFT, '=', TOK_LSHIFT_ASSIGN, TOK_LT);
    case '>':
      if (s[0][1] == '=') {
        (*s) += 2;
        return TOK_GE;
      }
      if (s[0][1] == '>' && s[0][2] == '>' && s[0][3] == '=') {
        (*s) += 4;
        return TOK_URSHIFT_ASSIGN;
      }
      if (s[0][1] == '>' && s[0][2] == '>') {
        (*s) += 3;
        return TOK_URSHIFT;
      }
      return punct2(s, '>', TOK_RSHIFT, '=', TOK_RSHIFT_ASSIGN, TOK_GT);

    case '{':
      (*s)++;
      return TOK_OPEN_CURLY;
    case '}':
      (*s)++;
      return TOK_CLOSE_CURLY;
    case '(':
      (*s)++;
      return TOK_OPEN_PAREN;
    case ')':
      (*s)++;
      return TOK_CLOSE_PAREN;
    case '[':
      (*s)++;
      return TOK_OPEN_BRACKET;
    case ']':
      (*s)++;
      return TOK_CLOSE_BRACKET;
    case '.':
      switch (*(*s + 1)) {
        /* Numbers */
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          parse_number(p, s, n);
          return TOK_NUMBER;
      }
      (*s)++;
      return TOK_DOT;
    case ';':
      (*s)++;
      return TOK_SEMICOLON;
    case ':':
      (*s)++;
      return TOK_COLON;
    case '?':
      (*s)++;
      return TOK_QUESTION;
    case '~':
      (*s)++;
      return TOK_TILDA;
    case ',':
      (*s)++;
      return TOK_COMMA;

    default:
      return TOK_END_OF_INPUT;
  }
}

#ifdef TEST_RUN
int main(void) {
  const char *src =
      "for (var fo++ = -1; /= <= 1.17; x<<) { == <<=, 'x')} "
      "Infinity %=x<<=2";
  enum v7_tok tok;
  double num;
  const char *p = src;

  skip_to_next_tok(&src);
  while ((tok = get_tok(&src, &num)) != TOK_END_OF_INPUT) {
    printf("%d [%.*s]\n", tok, (int)(src - p), p);
    skip_to_next_tok(&src);
    p = src;
  }
  printf("%d [%.*s]\n", tok, (int)(src - p), p);

  return 0;
}
#endif
