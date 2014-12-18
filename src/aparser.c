/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#define ACCEPT(t)                                   \
  (((v7)->cur_tok == (t)) ? next_tok((v7)), 1 : 0)

#define PARSE(p) TRY(aparse_ ## p(v7, a))
#define PARSE_ARG(p, arg) TRY(aparse_ ## p(v7, a, arg))

static enum v7_err aparse_expression(struct v7 *, struct ast *);
static enum v7_err aparse_statement(struct v7 *, struct ast *);
static enum v7_err aparse_statements(struct v7 *, struct ast *, int);
static enum v7_err aparse_terminal(struct v7 *, struct ast *);
static enum v7_err aparse_assign(struct v7 *, struct ast *);
static enum v7_err aparse_memberexpr(struct v7 *, struct ast *);
static enum v7_err aparse_funcdecl(struct v7 *, struct ast *, int);

static enum v7_err aparse_ident(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok == TOK_IDENTIFIER) {
    ast_add_ident(a, v7->tok, v7->tok_len);
    next_tok(v7);
    return V7_OK;
  }
  return V7_ERROR;
}

static enum v7_err aparse_prop(struct v7 *v7, struct ast *a) {
  size_t start;
  if (v7->cur_tok == TOK_IDENTIFIER &&
      strncmp(v7->tok, "get", v7->tok_len) == 0 &&
      lookahead(v7) != TOK_COLON) {
    start = ast_add_node(a, AST_GETTER);
    next_tok(v7);
    PARSE(ident);
    EXPECT(TOK_OPEN_PAREN);
    EXPECT(TOK_CLOSE_PAREN);
    aparse_statements(v7, a, 1);
    ast_set_skip(a, start, AST_END_SKIP);
  } else if (v7->cur_tok == TOK_IDENTIFIER &&
             strncmp(v7->tok, "set", v7->tok_len) == 0 &&
             lookahead(v7) != TOK_COLON) {
    start = ast_add_node(a, AST_SETTER);
    next_tok(v7);
    PARSE(ident);
    EXPECT(TOK_OPEN_PAREN);
    PARSE(ident);
    EXPECT(TOK_CLOSE_PAREN);
    aparse_statements(v7, a, 1);
    ast_set_skip(a, start, AST_END_SKIP);
  } else {
    ast_add_node(a, AST_PROP);
    PARSE(terminal);
    EXPECT(TOK_COLON);
    PARSE(assign);
  }
  return V7_OK;
}

static enum v7_err aparse_terminal(struct v7 *v7, struct ast *a) {
  size_t start;
  switch (v7->cur_tok) {
    case TOK_OPEN_PAREN:
      next_tok(v7);
      PARSE(expression);
      EXPECT(TOK_CLOSE_PAREN);
      break;
    case TOK_OPEN_BRACKET:
      next_tok(v7);
      start = ast_add_node(a, AST_ARRAY);
      if (v7->cur_tok != TOK_CLOSE_BRACKET) {
        /* TODO(mkm): simplify please */
        do {
          if (v7->cur_tok == TOK_COMMA) {
            ast_add_node(a, AST_NOP);
            if (lookahead(v7) == TOK_CLOSE_BRACKET) {
              next_tok(v7);
              break;
            }
          } else {
            PARSE(assign);
          }
        } while(ACCEPT(TOK_COMMA));
      }
      EXPECT(TOK_CLOSE_BRACKET);
      ast_set_skip(a, start, AST_END_SKIP);
      break;
    case TOK_OPEN_CURLY:
      next_tok(v7);
      start = ast_add_node(a, AST_OBJECT);
      if (v7->cur_tok != TOK_CLOSE_CURLY) {
        do {
          if (v7->cur_tok == TOK_CLOSE_CURLY) {
            break;
          }
          PARSE(prop);
        } while(ACCEPT(TOK_COMMA));
      }
      EXPECT(TOK_CLOSE_CURLY);
      ast_set_skip(a, start, AST_END_SKIP);
      break;
    case TOK_THIS:
      next_tok(v7);
      ast_add_node(a, AST_THIS);
      break;
    case TOK_TRUE:
      next_tok(v7);
      ast_add_node(a, AST_TRUE);
      break;
    case TOK_FALSE:
      next_tok(v7);
      ast_add_node(a, AST_FALSE);
      break;
    case TOK_NULL:
      next_tok(v7);
      ast_add_node(a, AST_NULL);
      break;
    case TOK_NUMBER:
      ast_add_num(a, v7->cur_tok_dbl);
      next_tok(v7);
      break;
    case TOK_STRING_LITERAL:
      ast_add_string(a, v7->tok + 1, v7->tok_len - 2);
      next_tok(v7);
      break;
    case TOK_REGEX_LITERAL:
      ast_add_regex(a, v7->tok, v7->tok_len);
      next_tok(v7);
      break;
    case TOK_IDENTIFIER:
      if (strncmp(v7->tok, "undefined", v7->tok_len) == 0) {
        ast_add_node(a, AST_UNDEFINED);
        next_tok(v7);
        break;
      }
      /* fall through */
    default:
      PARSE(ident);
  }
  return V7_OK;
}

static enum v7_err aparse_arglist(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok != TOK_CLOSE_PAREN) {
    do {
      PARSE(assign);
    } while (ACCEPT(TOK_COMMA));
  }
  return V7_OK;
}

static enum v7_err aparse_newexpr(struct v7 *v7, struct ast *a) {
  size_t start;
  switch (v7->cur_tok) {
    case TOK_NEW:
      next_tok(v7);
      start = ast_add_node(a, AST_NEW);
      PARSE(memberexpr);
      if (ACCEPT(TOK_OPEN_PAREN)) {
        PARSE(arglist);
        EXPECT(TOK_CLOSE_PAREN);
      }
      ast_set_skip(a, start, AST_END_SKIP);
      break;
    case TOK_FUNCTION:
      next_tok(v7);
      PARSE_ARG(funcdecl, 0);
      break;
    default:
      PARSE(terminal);
      break;
  }
  return V7_OK;
}

static enum v7_err aparse_memberexpr(struct v7 *v7, struct ast *a) {
  size_t pos = a->len;
  PARSE(newexpr);

  for (;;) {
    switch (v7->cur_tok) {
      case TOK_DOT:
        next_tok(v7);
        PARSE(ident);
        ast_insert_node(a, pos, AST_MEMBER);
        break;
      case TOK_OPEN_BRACKET:
        next_tok(v7);
        PARSE(expression);
        EXPECT(TOK_CLOSE_BRACKET);
        ast_insert_node(a, pos, AST_INDEX);
        break;
      default:
        return V7_OK;
    }
  }
}

static enum v7_err aparse_callexpr(struct v7 *v7, struct ast *a) {
  size_t pos = a->len;
  PARSE(newexpr);

  for (;;) {
    switch (v7->cur_tok) {
      case TOK_DOT:
        next_tok(v7);
        PARSE(ident);
        ast_insert_node(a, pos, AST_MEMBER);
        break;
      case TOK_OPEN_BRACKET:
        next_tok(v7);
        PARSE(expression);
        EXPECT(TOK_CLOSE_BRACKET);
        ast_insert_node(a, pos, AST_INDEX);
        break;
      case TOK_OPEN_PAREN:
        next_tok(v7);
        PARSE(arglist);
        EXPECT(TOK_CLOSE_PAREN);
        ast_insert_node(a, pos, AST_CALL);
        break;
      default:
        return V7_OK;
    }
  }
}

static enum v7_err aparse_postfix(struct v7 *v7, struct ast *a) {
  size_t pos = a->len;
  PARSE(callexpr);

  if (v7->after_newline) {
    return V7_OK;
  }
  switch (v7->cur_tok) {
    case TOK_PLUS_PLUS:
      next_tok(v7);
      ast_insert_node(a, pos, AST_POSTINC);
      break;
    case TOK_MINUS_MINUS:
      next_tok(v7);
      ast_insert_node(a, pos, AST_POSTDEC);
      break;
    default:
      break;  /* nothing */
  }
  return V7_OK;
}

enum v7_err aparse_prefix(struct v7 *v7, struct ast *a) {
  for (;;) {
    switch (v7->cur_tok) {
      case TOK_PLUS:
        next_tok(v7);
        ast_add_node(a, AST_POSITIVE);
        break;
      case TOK_MINUS:
        next_tok(v7);
        ast_add_node(a, AST_NEGATIVE);
        break;
      case TOK_PLUS_PLUS:
        next_tok(v7);
        ast_add_node(a, AST_PREINC);
        break;
      case TOK_MINUS_MINUS:
        next_tok(v7);
        ast_add_node(a, AST_PREDEC);
        break;
      case TOK_TILDA:
        next_tok(v7);
        ast_add_node(a, AST_NOT);
        break;
      case TOK_NOT:
        next_tok(v7);
        ast_add_node(a, AST_LOGICAL_NOT);
        break;
      case TOK_VOID:
        next_tok(v7);
        ast_add_node(a, AST_VOID);
        break;
      case TOK_DELETE:
        next_tok(v7);
        ast_add_node(a, AST_DELETE);
        break;
      case TOK_TYPEOF:
        next_tok(v7);
        ast_add_node(a, AST_TYPEOF);
        break;
      default:
        return aparse_postfix(v7, a);
    }
  }
}

static enum v7_err aparse_binary(struct v7 *v7, struct ast *a,
                                 int level, size_t pos) {
  struct {
    int len, left_to_right;
    struct {
      enum v7_tok start_tok, end_tok;
      enum ast_tag start_ast;
    } parts[2];
  } levels[] = {
    {1, 0, {{TOK_ASSIGN, TOK_URSHIFT_ASSIGN, AST_ASSIGN}, {0, 0, 0}}},
    {1, 0, {{TOK_QUESTION, TOK_QUESTION, AST_COND}, {0, 0, 0}}},
    {1, 1, {{TOK_LOGICAL_OR, TOK_LOGICAL_OR, AST_LOGICAL_OR}, {0, 0, 0}}},
    {1, 1, {{TOK_LOGICAL_AND, TOK_LOGICAL_AND, AST_LOGICAL_AND}, {0, 0, 0}}},
    {1, 1, {{TOK_OR, TOK_OR, AST_OR}, {0, 0, 0}}},
    {1, 1, {{TOK_XOR, TOK_XOR, AST_XOR}, {0, 0, 0}}},
    {1, 1, {{TOK_AND, TOK_AND, AST_AND}, {0, 0, 0}}},
    {1, 1, {{TOK_EQ, TOK_NE_NE, AST_EQ}, {0, 0, 0}}},
    {2, 1, {{TOK_LE, TOK_GT, AST_LE}, {TOK_IN, TOK_INSTANCEOF, AST_IN}}},
    {1, 1, {{TOK_LSHIFT, TOK_URSHIFT, AST_LSHIFT}, {0, 0, 0}}},
    {1, 1, {{TOK_PLUS, TOK_MINUS, AST_ADD}, {0, 0, 0}}},
    {1, 1, {{TOK_REM, TOK_DIV, AST_REM}, {0, 0, 0}}}
  };

  int i;
  enum v7_tok tok;
  enum ast_tag ast;

  if (level == (int) ARRAY_SIZE(levels) - 1) {
    PARSE(prefix);
  } else {
    TRY(aparse_binary(v7, a, level + 1, a->len));
  }

  for (i = 0; i < levels[level].len; i++) {
    tok = levels[level].parts[i].start_tok;
    ast = levels[level].parts[i].start_ast;
    do {
      /*
       * Ternary operator sits in the middle of the binary operator
       * precedence chain. Deal with it as an exception and don't break
       * the chain.
       */
      if (tok == TOK_QUESTION && v7->cur_tok == TOK_QUESTION) {
        next_tok(v7);
        PARSE(assign);
        EXPECT(TOK_COLON);
        PARSE(assign);
        ast_insert_node(a, pos, AST_COND);
        return V7_OK;
      } else if (ACCEPT(tok)) {
        if (levels[level].left_to_right) {
          ast_insert_node(a, pos, ast);
          TRY(aparse_binary(v7, a, level, pos));
        } else {
          TRY(aparse_binary(v7, a, level, a->len));
          ast_insert_node(a, pos, ast);
        }
      }
    } while(ast++, tok++ < levels[level].parts[i].end_tok);
  }

  return V7_OK;
}

static enum v7_err aparse_assign(struct v7 *v7, struct ast *a) {
  return aparse_binary(v7, a, 0, a->len);
}

static enum v7_err aparse_expression(struct v7 *v7, struct ast *a) {
  size_t pos = a->len;
  int group = 0;
  do {
    PARSE(assign);
  } while(ACCEPT(TOK_COMMA) && (group = 1));
  if (group) {
    ast_insert_node(a, pos, AST_SEQ);
  }
  return V7_OK;
}

static enum v7_err end_of_statement(struct v7 *v7) {
  if (v7->cur_tok == TOK_SEMICOLON ||
      v7->cur_tok == TOK_END_OF_INPUT ||
      v7->cur_tok == TOK_CLOSE_CURLY ||
      v7->after_newline) {
    return V7_OK;
  }
  return V7_ERROR;
}

static enum v7_err aparse_statements(struct v7 *v7, struct ast *a,
                                     int require_block) {
  if (ACCEPT(TOK_OPEN_CURLY)) {
    while (!ACCEPT(TOK_CLOSE_CURLY)) {
      PARSE(statement);
    }
    return V7_OK;
  } else if (!require_block) {
    PARSE(statement);
    return V7_OK;
  }
  return V7_ERROR;
}

static enum v7_err aparse_var(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_VAR);
  do {
    ast_add_node(a, AST_VAR_ITEM);
    PARSE(ident);
    if (ACCEPT(TOK_ASSIGN)) {
      PARSE(assign);
    } else {
      ast_add_node(a, AST_NOP);
    }
  } while (ACCEPT(TOK_COMMA));
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static int aparse_optional(struct v7 *v7, struct ast *a,
                    enum v7_tok terminator) {
  if (v7->cur_tok != terminator) {
    return 1;
  }
  ast_add_node(a, AST_NOP);
  return 0;
}

static enum v7_err aparse_if(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_IF);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  PARSE_ARG(statements, 0);
  ast_set_skip(a, start, AST_END_IF_TRUE_SKIP);
  if (ACCEPT(TOK_ELSE)) {
    PARSE_ARG(statements, 0);
  }
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_while(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_WHILE);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  PARSE_ARG(statements, 0);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_dowhile(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_DOWHILE);
  PARSE_ARG(statements, 0);
  ast_set_skip(a, start, AST_DO_WHILE_COND_SKIP);
  EXPECT(TOK_WHILE);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_for(struct v7 *v7, struct ast *a) {
  /* TODO(mkm): for in, for of, for each in */
  size_t start = ast_add_node(a, AST_FOR);
  EXPECT(TOK_OPEN_PAREN);

  if(aparse_optional(v7, a, TOK_SEMICOLON)) {
    if (ACCEPT(TOK_VAR)) {
      aparse_var(v7, a);
    } else {
      PARSE(expression);
    }
  }
  EXPECT(TOK_SEMICOLON);
  if (aparse_optional(v7, a, TOK_SEMICOLON)) {
    PARSE(expression);
  }
  EXPECT(TOK_SEMICOLON);
  if (aparse_optional(v7, a, TOK_CLOSE_PAREN)) {
    PARSE(expression);
  }
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_FOR_BODY_SKIP);
  PARSE_ARG(statements, 0);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_switch(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_SWITCH);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  EXPECT(TOK_OPEN_CURLY);
  while (v7->cur_tok != TOK_CLOSE_CURLY) {
    size_t case_start;
    switch (v7->cur_tok) {
      case TOK_CASE:
        next_tok(v7);
        case_start = ast_add_node(a, AST_CASE);
        PARSE(expression);
        EXPECT(TOK_COLON);
        while (v7->cur_tok != TOK_CASE &&
               v7->cur_tok != TOK_DEFAULT &&
               v7->cur_tok != TOK_CLOSE_CURLY) {
          PARSE(statement);
        }
        ast_set_skip(a, case_start, AST_END_SKIP);
        break;
      case TOK_DEFAULT:
        next_tok(v7);
        EXPECT(TOK_COLON);
        ast_set_skip(a, start, AST_SWITCH_DEFAULT_SKIP);
        case_start = ast_add_node(a, AST_DEFAULT);
        while (v7->cur_tok != TOK_CASE &&
               v7->cur_tok != TOK_DEFAULT &&
               v7->cur_tok != TOK_CLOSE_CURLY) {
          PARSE(statement);
        }
        ast_set_skip(a, case_start, AST_END_SKIP);
        break;
      default:
        return V7_ERROR;
    }
  }
  EXPECT(TOK_CLOSE_CURLY);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_try(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_TRY);
  PARSE_ARG(statements, 1);
  ast_set_skip(a, start, AST_TRY_CATCH_SKIP);
  if (ACCEPT(TOK_CATCH)) {
    EXPECT(TOK_OPEN_PAREN);
    PARSE(ident);
    EXPECT(TOK_CLOSE_PAREN);
    PARSE_ARG(statements, 1);
  }
  ast_set_skip(a, start, AST_TRY_FINALLY_SKIP);
  if (ACCEPT(TOK_FINALLY)) {
    PARSE_ARG(statements, 1);
  }
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_with(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_WITH);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  PARSE_ARG(statements, 0);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

#define PARSE_WITH_OPT_ARG(tag, arg_tag, arg_parser)  \
  do {                                                \
    if (end_of_statement(v7) == V7_OK) {              \
      ast_add_node(a, tag);                           \
    } else {                                          \
      ast_add_node(a, arg_tag);                       \
      PARSE(arg_parser);                              \
    }                                                 \
  } while(0)                                          \

static enum v7_err aparse_statement(struct v7 *v7, struct ast *a) {
  switch (v7->cur_tok) {
    case TOK_SEMICOLON:
      next_tok(v7);
      return V7_OK;  /* empty statement */
    case TOK_OPEN_CURLY:  /* block */
      aparse_statements(v7, a, 1);
      break;
    case TOK_IF:
      next_tok(v7);
      return aparse_if(v7, a); /* returning because no semicolon required */
    case TOK_WHILE:
      next_tok(v7);
      return aparse_while(v7, a);
    case TOK_DO:
      next_tok(v7);
      return aparse_dowhile(v7, a);
    case TOK_FOR:
      next_tok(v7);
      return aparse_for(v7, a);
    case TOK_TRY:
      next_tok(v7);
      return aparse_try(v7, a);
    case TOK_SWITCH:
      next_tok(v7);
      return aparse_switch(v7, a);
    case TOK_WITH:
      next_tok(v7);
      return aparse_with(v7, a);
    case TOK_BREAK:
      next_tok(v7);
      PARSE_WITH_OPT_ARG(AST_BREAK, AST_LABELED_BREAK, ident);
      break;
    case TOK_CONTINUE:
      next_tok(v7);
      PARSE_WITH_OPT_ARG(AST_CONTINUE, AST_LABELED_CONTINUE, ident);
      break;
    case TOK_RETURN:
      next_tok(v7);
      PARSE_WITH_OPT_ARG(AST_RETURN, AST_VALUE_RETURN, expression);
      break;
    case TOK_THROW:
      next_tok(v7);
      ast_add_node(a, AST_THROW);
      PARSE(expression);
      break;
    case TOK_DEBUGGER:
      next_tok(v7);
      ast_add_node(a, AST_DEBUGGER);
      break;
    case TOK_VAR:
      next_tok(v7);
      aparse_var(v7, a);
      break;
    default:
      PARSE(expression);
      break;
  }

  /* TODO(mkm): labels, function statements */

  TRY(end_of_statement(v7));
  ACCEPT(TOK_SEMICOLON);  /* swallow optional semicolon */
  return V7_OK;
}

static enum v7_err aparse_funcdecl(struct v7 *v7, struct ast *a,
                                   int require_named) {
  size_t start = ast_add_node(a, AST_FUNC);
  if (aparse_ident(v7, a) == V7_ERROR) {
    if (require_named) {
      return V7_ERROR;
    }
    ast_add_ident(a, "?", 1);
  }
  EXPECT(TOK_OPEN_PAREN);
  PARSE(arglist);
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_FUNC_BODY_SKIP);
  PARSE_ARG(statements, 1);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_body(struct v7 *v7, struct ast *a,
                               enum v7_tok end) {
  while (v7->cur_tok != end) {
    if (ACCEPT(TOK_FUNCTION)) {
      PARSE_ARG(funcdecl, 1);
    } else {
      PARSE(statement);
    }
  }
  return V7_OK;
}

static enum v7_err aparse_script(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_SCRIPT);
  PARSE_ARG(body, TOK_END_OF_INPUT);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

V7_PRIVATE enum v7_err aparse(struct ast *a, const char *src, int verbose) {
  enum v7_err err;
  struct v7 *v7 = v7_create();
  v7->pstate.source_code = v7->pstate.pc = src;
  v7->pstate.file_name = "<stdin>";
  v7->pstate.line_no = 1;

  next_tok(v7);
  err = aparse_script(v7, a);
  if (err == V7_OK && v7->cur_tok != TOK_END_OF_INPUT) {
    printf("WARNING parse input not consumed\n");
  }
  if (verbose && err != V7_OK) {
      printf("Parse error at at line %d\n", v7->pstate.line_no);
  }
  return err;
}
