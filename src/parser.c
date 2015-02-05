/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#define ACCEPT(t)                                   \
  (((v7)->cur_tok == (t)) ? next_tok((v7)), 1 : 0)

#define PARSE(p) TRY(parse_ ## p(v7, a))
#define PARSE_ARG(p, arg) TRY(parse_ ## p(v7, a, arg))
#define PARSE_ARG_2(p, a1, a2) TRY(parse_ ## p(v7, a, a1, a2))

#define TRY(call)           \
  do {                      \
    enum v7_err _e = call;  \
    CHECK(_e == V7_OK, _e); \
  } while (0)

#define THROW(err_code)                                                       \
  do {                                                                        \
    snprintf(v7->error_msg, sizeof(v7->error_msg), "Parse error: %s line %d", \
             v7->pstate.file_name, v7->pstate.line_no);                       \
    return (err_code);                                                        \
  } while (0)

#define CHECK(cond, code)     \
  do {                        \
    if (!(cond)) THROW(code); \
  } while (0)

#define EXPECT(t)                                     \
  do {                                                \
    if ((v7)->cur_tok != (t)) return V7_SYNTAX_ERROR; \
    next_tok(v7);                                     \
  } while (0)

static enum v7_err parse_expression(struct v7 *, struct ast *);
static enum v7_err parse_statement(struct v7 *, struct ast *);
static enum v7_err parse_terminal(struct v7 *, struct ast *);
static enum v7_err parse_assign(struct v7 *, struct ast *);
static enum v7_err parse_memberexpr(struct v7 *, struct ast *);
static enum v7_err parse_funcdecl(struct v7 *, struct ast *, int, int);
static enum v7_err parse_block(struct v7 *, struct ast *);
static enum v7_err parse_body(struct v7 *, struct ast *, enum v7_tok);
static enum v7_err parse_use_strict(struct v7 *, struct ast *);

static enum v7_tok lookahead(const struct v7 *v7) {
  const char *s = v7->pstate.pc;
  double d;
  return get_tok(&s, &d, v7->cur_tok);
}

static enum v7_tok next_tok(struct v7 *v7) {
  int prev_line_no = v7->pstate.prev_line_no;
  v7->pstate.prev_line_no = v7->pstate.line_no;
  v7->pstate.line_no += skip_to_next_tok(&v7->pstate.pc);
  v7->after_newline = prev_line_no != v7->pstate.line_no;
  v7->tok = v7->pstate.pc;
  v7->cur_tok = get_tok(&v7->pstate.pc, &v7->cur_tok_dbl, v7->cur_tok);
  v7->tok_len = v7->pstate.pc - v7->tok;
  v7->pstate.line_no += skip_to_next_tok(&v7->pstate.pc);
  return v7->cur_tok;
}

static enum v7_err parse_ident(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok == TOK_IDENTIFIER) {
    ast_add_inlined_node(a, AST_IDENT, v7->tok, v7->tok_len);
    next_tok(v7);
    return V7_OK;
  }
  return V7_SYNTAX_ERROR;
}

static enum v7_err parse_ident_allow_reserved_words(struct v7 *v7,
                                                     struct ast *a) {
  /* Allow reserved words as property names. */
  if (is_reserved_word_token(v7->cur_tok)) {
    ast_add_inlined_node(a, AST_IDENT, v7->tok, v7->tok_len);
    next_tok(v7);
  } else {
    PARSE(ident);
  }
  return V7_OK;
}

static enum v7_err parse_prop(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok == TOK_IDENTIFIER &&
      strncmp(v7->tok, "get", v7->tok_len) == 0 &&
      lookahead(v7) != TOK_COLON) {
    next_tok(v7);
    ast_add_node(a, AST_GETTER);
    parse_funcdecl(v7, a, 1, 1);
  } else if (v7->cur_tok == TOK_IDENTIFIER &&
             strncmp(v7->tok, "set", v7->tok_len) == 0 &&
             lookahead(v7) != TOK_COLON) {
    next_tok(v7);
    ast_add_node(a, AST_SETTER);
    parse_funcdecl(v7, a, 1, 1);
  } else {
    /* Allow reserved words as property names. */
    if (is_reserved_word_token(v7->cur_tok) ||
        v7->cur_tok == TOK_IDENTIFIER ||
        v7->cur_tok == TOK_NUMBER) {
      ast_add_inlined_node(a, AST_PROP, v7->tok, v7->tok_len);
    } else if (v7->cur_tok == TOK_STRING_LITERAL) {
      ast_add_inlined_node(a, AST_PROP, v7->tok + 1, v7->tok_len - 2);
    } else {
      return V7_SYNTAX_ERROR;
    }
    next_tok(v7);
    EXPECT(TOK_COLON);
    PARSE(assign);
  }
  return V7_OK;
}

static enum v7_err parse_terminal(struct v7 *v7, struct ast *a) {
  ast_off_t start;
  switch (v7->cur_tok) {
    case TOK_OPEN_PAREN:
      next_tok(v7);
      PARSE(expression);
      EXPECT(TOK_CLOSE_PAREN);
      break;
    case TOK_OPEN_BRACKET:
      next_tok(v7);
      start = ast_add_node(a, AST_ARRAY);
      while (v7->cur_tok != TOK_CLOSE_BRACKET) {
        if (v7->cur_tok == TOK_COMMA) {
          /* Array literals allow missing elements, e.g. [,,1,] */
          ast_add_node(a, AST_NOP);
        } else {
          PARSE(assign);
        }
        ACCEPT(TOK_COMMA);
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
    case TOK_STRING_LITERAL:
      ast_add_inlined_node(a, AST_STRING, v7->tok + 1, v7->tok_len - 2);
      next_tok(v7);
      break;
    case TOK_NUMBER:
      ast_add_inlined_node(a, AST_NUM, v7->tok, v7->tok_len);
      next_tok(v7);
      break;
    case TOK_REGEX_LITERAL:
      ast_add_inlined_node(a, AST_REGEX, v7->tok, v7->tok_len);
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

static enum v7_err parse_arglist(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok != TOK_CLOSE_PAREN) {
    do {
      PARSE(assign);
    } while (ACCEPT(TOK_COMMA));
  }
  return V7_OK;
}

static enum v7_err parse_newexpr(struct v7 *v7, struct ast *a) {
  ast_off_t start;
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
      PARSE_ARG_2(funcdecl, 0, 0);
      break;
    default:
      PARSE(terminal);
      break;
  }
  return V7_OK;
}

static enum v7_err parse_member(struct v7 *v7, struct ast *a, ast_off_t pos) {
  switch (v7->cur_tok) {
    case TOK_DOT:
      next_tok(v7);
      /* Allow reserved words as member identifiers */
      if (is_reserved_word_token(v7->cur_tok) ||
          v7->cur_tok == TOK_IDENTIFIER) {
        ast_insert_inlined_node(a, pos, AST_MEMBER, v7->tok, v7->tok_len);
        next_tok(v7);
      } else {
        return V7_SYNTAX_ERROR;
      }
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
  return V7_OK;
}

static enum v7_err parse_memberexpr(struct v7 *v7, struct ast *a) {
  ast_off_t pos = a->mbuf.len;
  PARSE(newexpr);

  for (;;) {
    switch (v7->cur_tok) {
      case TOK_DOT:
      case TOK_OPEN_BRACKET:
        PARSE_ARG(member, pos);
        break;
      default:
        return V7_OK;
    }
  }
}

static enum v7_err parse_callexpr(struct v7 *v7, struct ast *a) {
  ast_off_t pos = a->mbuf.len;
  PARSE(newexpr);

  for (;;) {
    switch (v7->cur_tok) {
      case TOK_DOT:
      case TOK_OPEN_BRACKET:
        PARSE_ARG(member, pos);
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

static enum v7_err parse_postfix(struct v7 *v7, struct ast *a) {
  ast_off_t pos = a->mbuf.len;
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

enum v7_err parse_prefix(struct v7 *v7, struct ast *a) {
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
        return parse_postfix(v7, a);
    }
  }
}

static enum v7_err parse_binary(struct v7 *v7, struct ast *a,
                                 int level, ast_off_t pos) {
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
    TRY(parse_binary(v7, a, level + 1, a->mbuf.len));
  }

  for (i = 0; i < levels[level].len; i++) {
    tok = levels[level].parts[i].start_tok;
    ast = levels[level].parts[i].start_ast;
    do {
      if (v7->pstate.inhibit_in && tok == TOK_IN) {
        continue;
      }

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
          TRY(parse_binary(v7, a, level, pos));
        } else {
          TRY(parse_binary(v7, a, level, a->mbuf.len));
          ast_insert_node(a, pos, ast);
        }
      }
    } while(ast++, tok++ < levels[level].parts[i].end_tok);
  }

  return V7_OK;
}

static enum v7_err parse_assign(struct v7 *v7, struct ast *a) {
  return parse_binary(v7, a, 0, a->mbuf.len);
}

static enum v7_err parse_expression(struct v7 *v7, struct ast *a) {
  ast_off_t pos = a->mbuf.len;
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
  return V7_SYNTAX_ERROR;
}

static enum v7_err parse_var(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_VAR);
  ast_modify_skip(a, v7->last_var_node, start, AST_FUNC_FIRST_VAR_SKIP);
  /* zero out var node pointer */
  ast_modify_skip(a, start, start, AST_FUNC_FIRST_VAR_SKIP);
  v7->last_var_node = start;
  do {
    ast_add_inlined_node(a, AST_VAR_DECL, v7->tok, v7->tok_len);
    EXPECT(TOK_IDENTIFIER);
    if (ACCEPT(TOK_ASSIGN)) {
      PARSE(assign);
    } else {
      ast_add_node(a, AST_NOP);
    }
  } while (ACCEPT(TOK_COMMA));
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static int parse_optional(struct v7 *v7, struct ast *a,
                    enum v7_tok terminator) {
  if (v7->cur_tok != terminator) {
    return 1;
  }
  ast_add_node(a, AST_NOP);
  return 0;
}

static enum v7_err parse_if(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_IF);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  PARSE(statement);
  ast_set_skip(a, start, AST_END_IF_TRUE_SKIP);
  if (ACCEPT(TOK_ELSE)) {
    PARSE(statement);
  }
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err parse_while(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_WHILE);
  int saved_in_loop = v7->pstate.in_loop;
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  v7->pstate.in_loop = 1;
  PARSE(statement);
  ast_set_skip(a, start, AST_END_SKIP);
  v7->pstate.in_loop = saved_in_loop;
  return V7_OK;
}

static enum v7_err parse_dowhile(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_DOWHILE);
  int saved_in_loop = v7->pstate.in_loop;
  v7->pstate.in_loop = 1;
  PARSE(statement);
  v7->pstate.in_loop = saved_in_loop;
  ast_set_skip(a, start, AST_DO_WHILE_COND_SKIP);
  EXPECT(TOK_WHILE);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err parse_for(struct v7 *v7, struct ast *a) {
  /* TODO(mkm): for of, for each in */
  ast_off_t start = ast_add_node(a, AST_FOR);
  int saved_in_loop = v7->pstate.in_loop;

  EXPECT(TOK_OPEN_PAREN);

  if(parse_optional(v7, a, TOK_SEMICOLON)) {
    /*
     * TODO(mkm): make this reentrant otherwise this pearl won't parse:
     * for((function(){return 1 in o.a ? o : x})().a in [1,2,3])
     */
    v7->pstate.inhibit_in = 1;
    if (ACCEPT(TOK_VAR)) {
      parse_var(v7, a);
    } else {
      PARSE(expression);
    }
    v7->pstate.inhibit_in = 0;

    if (ACCEPT(TOK_IN)) {
      PARSE(expression);
      ast_add_node(a, AST_NOP);
      /*
       * Assumes that for and for in have the same AST format which is
       * suboptimal but avoids the need of fixing up the var offset chain.
       * TODO(mkm) improve this
       */
      a->mbuf.buf[start - 1] = AST_FOR_IN;
      goto body;
    }
  }

  EXPECT(TOK_SEMICOLON);
  if (parse_optional(v7, a, TOK_SEMICOLON)) {
    PARSE(expression);
  }
  EXPECT(TOK_SEMICOLON);
  if (parse_optional(v7, a, TOK_CLOSE_PAREN)) {
    PARSE(expression);
  }

body:
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_FOR_BODY_SKIP);
  v7->pstate.in_loop = 1;
  PARSE(statement);
  v7->pstate.in_loop = saved_in_loop;
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err parse_switch(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_SWITCH);
  int saved_in_switch = v7->pstate.in_switch;
  ast_set_skip(a, start, AST_SWITCH_DEFAULT_SKIP); /* clear out */
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  EXPECT(TOK_OPEN_CURLY);
  v7->pstate.in_switch = 1;
  while (v7->cur_tok != TOK_CLOSE_CURLY) {
    ast_off_t case_start;
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
        return V7_SYNTAX_ERROR;
    }
  }
  EXPECT(TOK_CLOSE_CURLY);
  ast_set_skip(a, start, AST_END_SKIP);
  v7->pstate.in_switch = saved_in_switch;
  return V7_OK;
}

static enum v7_err parse_try(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_TRY);
  PARSE(block);
  ast_set_skip(a, start, AST_TRY_CATCH_SKIP);
  if (ACCEPT(TOK_CATCH)) {
    EXPECT(TOK_OPEN_PAREN);
    PARSE(ident);
    EXPECT(TOK_CLOSE_PAREN);
    PARSE(block);
  }
  ast_set_skip(a, start, AST_TRY_FINALLY_SKIP);
  if (ACCEPT(TOK_FINALLY)) {
    PARSE(block);
  }
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err parse_with(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_WITH);
  if (v7->pstate.in_strict) {
    return V7_SYNTAX_ERROR;
  }
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  PARSE(statement);
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

static enum v7_err parse_statement(struct v7 *v7, struct ast *a) {
  switch (v7->cur_tok) {
    case TOK_SEMICOLON:
      next_tok(v7);
      return V7_OK;  /* empty statement */
    case TOK_OPEN_CURLY:  /* block */
      PARSE(block);
      return V7_OK;  /* returning because no semicolon required */
    case TOK_IF:
      next_tok(v7);
      return parse_if(v7, a);
    case TOK_WHILE:
      next_tok(v7);
      return parse_while(v7, a);
    case TOK_DO:
      next_tok(v7);
      return parse_dowhile(v7, a);
    case TOK_FOR:
      next_tok(v7);
      return parse_for(v7, a);
    case TOK_TRY:
      next_tok(v7);
      return parse_try(v7, a);
    case TOK_SWITCH:
      next_tok(v7);
      return parse_switch(v7, a);
    case TOK_WITH:
      next_tok(v7);
      return parse_with(v7, a);
    case TOK_BREAK:
      if (!(v7->pstate.in_loop || v7->pstate.in_switch)) {
        return V7_SYNTAX_ERROR;
      }
      next_tok(v7);
      PARSE_WITH_OPT_ARG(AST_BREAK, AST_LABELED_BREAK, ident);
      break;
    case TOK_CONTINUE:
      if (!v7->pstate.in_loop) {
        return V7_SYNTAX_ERROR;
      }
      next_tok(v7);
      PARSE_WITH_OPT_ARG(AST_CONTINUE, AST_LABELED_CONTINUE, ident);
      break;
    case TOK_RETURN:
      if (!v7->pstate.in_function) {
        return V7_SYNTAX_ERROR;
      }
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
      parse_var(v7, a);
      break;
    case TOK_IDENTIFIER:
      if (lookahead(v7) == TOK_COLON) {
        ast_add_inlined_node(a, AST_LABEL, v7->tok, v7->tok_len);
        next_tok(v7);
        EXPECT(TOK_COLON);
        return V7_OK;
      }
      /* fall through */
    default:
      PARSE(expression);
      break;
  }

  /* TODO(mkm): labels */

  TRY(end_of_statement(v7));
  ACCEPT(TOK_SEMICOLON);  /* swallow optional semicolon */
  return V7_OK;
}

static enum v7_err parse_funcdecl(struct v7 *v7, struct ast *a,
                                  int require_named, int reserved_name) {
  ast_off_t start = ast_add_node(a, AST_FUNC);
  ast_off_t outer_last_var_node = v7->last_var_node;
  int saved_in_function = v7->pstate.in_function;
  int saved_in_strict = v7->pstate.in_strict;
  v7->last_var_node = start;
  ast_modify_skip(a, start, start, AST_FUNC_FIRST_VAR_SKIP);
  if ((reserved_name ? parse_ident_allow_reserved_words : parse_ident)(v7, a) ==
      V7_SYNTAX_ERROR) {
    if (require_named) {
      return V7_SYNTAX_ERROR;
    }
    ast_add_node(a, AST_NOP);
  }
  EXPECT(TOK_OPEN_PAREN);
  PARSE(arglist);
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_FUNC_BODY_SKIP);
  v7->pstate.in_function = 1;
  EXPECT(TOK_OPEN_CURLY);
  if (parse_use_strict(v7, a) == V7_OK) {
    v7->pstate.in_strict = 1;
  }
  PARSE_ARG(body, TOK_CLOSE_CURLY);
  EXPECT(TOK_CLOSE_CURLY);
  v7->pstate.in_strict = saved_in_strict;
  v7->pstate.in_function = saved_in_function;
  ast_set_skip(a, start, AST_END_SKIP);
  v7->last_var_node = outer_last_var_node;
  return V7_OK;
}

static enum v7_err parse_block(struct v7 *v7, struct ast *a) {
  EXPECT(TOK_OPEN_CURLY);
  PARSE_ARG(body, TOK_CLOSE_CURLY);
  EXPECT(TOK_CLOSE_CURLY);
  return V7_OK;
}

static enum v7_err parse_body(struct v7 *v7, struct ast *a,
                               enum v7_tok end) {
  ast_off_t start;
  while (v7->cur_tok != end) {
    if (ACCEPT(TOK_FUNCTION)) {
      if (v7->cur_tok != TOK_IDENTIFIER) {
        return V7_SYNTAX_ERROR;
      }
      start = ast_add_node(a, AST_VAR);
      ast_modify_skip(a, v7->last_var_node, start, AST_FUNC_FIRST_VAR_SKIP);
      /* zero out var node pointer */
      ast_modify_skip(a, start, start, AST_FUNC_FIRST_VAR_SKIP);
      v7->last_var_node = start;
      ast_add_inlined_node(a, AST_FUNC_DECL, v7->tok, v7->tok_len);

      PARSE_ARG_2(funcdecl, 1, 0);
      ast_set_skip(a, start, AST_END_SKIP);
    } else {
      PARSE(statement);
    }
  }
  return V7_OK;
}

static enum v7_err parse_use_strict(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok == TOK_STRING_LITERAL &&
      (strncmp(v7->tok, "\"use strict\"", v7->tok_len) == 0 ||
       strncmp(v7->tok, "'use strict'", v7->tok_len) == 0)) {
    next_tok(v7);
    ast_add_node(a, AST_USE_STRICT);
    return V7_OK;
  }
  return V7_SYNTAX_ERROR;
}

static enum v7_err parse_script(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_SCRIPT);
  ast_off_t outer_last_var_node = v7->last_var_node;
  v7->last_var_node = start;
  ast_modify_skip(a, start, 1, AST_FUNC_FIRST_VAR_SKIP);
  parse_use_strict(v7, a);
  PARSE_ARG(body, TOK_END_OF_INPUT);
  ast_set_skip(a, start, AST_END_SKIP);
  v7->last_var_node = outer_last_var_node;
  return V7_OK;
}

static unsigned long get_column(const char *code, const char *pos) {
  const char *p = pos;
  while (p > code && *p != '\n') {
    p--;
  }
  return p == code ? pos - p : pos - (p + 1);
}

V7_PRIVATE enum v7_err parse(struct v7 *v7, struct ast *a, const char *src,
                              int verbose) {
  enum v7_err err;
  v7->pstate.source_code = v7->pstate.pc = src;
  v7->pstate.file_name = "<stdin>";
  v7->pstate.line_no = 1;
  v7->pstate.in_function = 0;
  v7->pstate.in_loop = 0;
  v7->pstate.in_switch = 0;

  next_tok(v7);
  err = parse_script(v7, a);
  if (err == V7_OK && v7->cur_tok != TOK_END_OF_INPUT) {
    fprintf(stderr, "WARNING parse input not consumed\n");
  }
  if (verbose && err != V7_OK) {
    unsigned long col = get_column(v7->pstate.source_code, v7->tok);
    snprintf(v7->error_msg, sizeof(v7->error_msg),
             "parse error at at line %d col %lu: [%.*s]", v7->pstate.line_no,
             col, (int) (col + v7->tok_len), v7->tok - col);
  }
  return err;
}
