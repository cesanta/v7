/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

#include <stdio.h>
#include "internal.h"

#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

/* TODO(mkm): reorder */
enum ast_tag {
  AST_NOP,
  AST_SCRIPT,
  AST_VAR,
  AST_VAR_DECL,
  AST_FUNC_DECL,
  AST_IF,
  AST_FUNC,

  AST_ASSIGN,
  AST_REM_ASSIGN,
  AST_MUL_ASSIGN,
  AST_DIV_ASSIGN,
  AST_XOR_ASSIGN,
  AST_PLUS_ASSIGN,
  AST_MINUS_ASSIGN,
  AST_OR_ASSIGN,
  AST_AND_ASSIGN,
  AST_LSHIFT_ASSIGN,
  AST_RSHIFT_ASSIGN,
  AST_URSHIFT_ASSIGN,

  AST_NUM,
  AST_IDENT,
  AST_STRING,
  AST_REGEX,
  AST_LABEL,

  AST_SEQ,
  AST_WHILE,
  AST_DOWHILE,
  AST_FOR,
  AST_FOR_IN,
  AST_COND,

  AST_DEBUGGER,
  AST_BREAK,
  AST_LABELED_BREAK,
  AST_CONTINUE,
  AST_LABELED_CONTINUE,
  AST_RETURN,
  AST_VALUE_RETURN,
  AST_THROW,

  AST_TRY,
  AST_SWITCH,
  AST_CASE,
  AST_DEFAULT,
  AST_WITH,

  AST_LOGICAL_OR,
  AST_LOGICAL_AND,
  AST_OR,
  AST_XOR,
  AST_AND,

  AST_EQ,
  AST_EQ_EQ,
  AST_NE,
  AST_NE_NE,

  AST_LE,
  AST_LT,
  AST_GE,
  AST_GT,
  AST_IN,
  AST_INSTANCEOF,

  AST_LSHIFT,
  AST_RSHIFT,
  AST_URSHIFT,

  AST_ADD,
  AST_SUB,

  AST_REM,
  AST_MUL,
  AST_DIV,

  AST_POSITIVE,
  AST_NEGATIVE,
  AST_NOT,
  AST_LOGICAL_NOT,
  AST_VOID,
  AST_DELETE,
  AST_TYPEOF,
  AST_PREINC,
  AST_PREDEC,

  AST_POSTINC,
  AST_POSTDEC,

  AST_MEMBER,
  AST_INDEX,
  AST_CALL,

  AST_NEW,

  AST_ARRAY,
  AST_OBJECT,
  AST_PROP,
  AST_GETTER,
  AST_SETTER,

  AST_THIS,
  AST_TRUE,
  AST_FALSE,
  AST_NULL,
  AST_UNDEFINED,

  AST_USE_STRICT,

  AST_MAX_TAG
};

struct ast {
  struct mbuf mbuf;
};

typedef unsigned long ast_off_t;

struct ast_node_def {
  const char *name;      /* tag name, for debugging and serialization */
  unsigned char has_varint;    /* has a varint body */
  unsigned char has_inlined;   /* inlined data whose size is in the varint field */
  unsigned char num_skips;     /* number of skips */
  unsigned char num_subtrees;  /* number of fixed subtrees */
};
extern const struct ast_node_def ast_node_defs[];

enum ast_which_skip {
  AST_END_SKIP = 0,
  AST_VAR_NEXT_SKIP = 1,
  AST_SCRIPT_FIRST_VAR_SKIP = AST_VAR_NEXT_SKIP,
  AST_FOR_BODY_SKIP = 1,
  AST_DO_WHILE_COND_SKIP = 1,
  AST_END_IF_TRUE_SKIP = 1,
  AST_TRY_CATCH_SKIP = 1,
  AST_TRY_FINALLY_SKIP = 2,
  AST_FUNC_FIRST_VAR_SKIP = AST_VAR_NEXT_SKIP,
  AST_FUNC_BODY_SKIP = 2,
  AST_SWITCH_DEFAULT_SKIP = 1
};

V7_PRIVATE void ast_init(struct ast *, size_t);
V7_PRIVATE void ast_optimize(struct ast *);
V7_PRIVATE void ast_free(struct ast *);
V7_PRIVATE ast_off_t ast_add_node(struct ast *, enum ast_tag);
V7_PRIVATE ast_off_t ast_insert_node(struct ast *, ast_off_t, enum ast_tag);
V7_PRIVATE ast_off_t ast_set_skip(struct ast *, ast_off_t, enum ast_which_skip);
V7_PRIVATE ast_off_t ast_get_skip(struct ast *, ast_off_t, enum ast_which_skip);
V7_PRIVATE ast_off_t ast_modify_skip(struct ast *, ast_off_t, ast_off_t,
                                     enum ast_which_skip);
V7_PRIVATE enum ast_tag ast_fetch_tag(struct ast *, ast_off_t *);
V7_PRIVATE void ast_move_to_children(struct ast *, ast_off_t *);

V7_PRIVATE void ast_add_inlined_node(struct ast *, enum ast_tag, const char *,
                                     size_t);
V7_PRIVATE void ast_insert_inlined_node(struct ast *, ast_off_t, enum ast_tag,
                                        const char *, size_t);

V7_PRIVATE char *ast_get_inlined_data(struct ast *, ast_off_t, size_t *);
V7_PRIVATE void ast_get_num(struct ast *, ast_off_t, double *);

V7_PRIVATE void ast_skip_tree(struct ast *, ast_off_t *);
V7_PRIVATE void ast_dump(FILE *, struct ast *, ast_off_t);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */

#endif  /* AST_H_INCLUDED */
