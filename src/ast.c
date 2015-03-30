/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

typedef unsigned short ast_skip_t;

#ifndef V7_DISABLE_AST_TAG_NAMES
#define AST_NAME(n) (n),
#else
#define AST_NAME(n)
#endif

/*
 * The structure of AST nodes cannot be described in portable ANSI C,
 * since they are variable length and packed (unaligned).
 *
 * Here each node's body is described with a pseudo-C structure notation.
 * The pseudo type `child` represents a variable length byte sequence
 * representing a fully serialized child node.
 *
 * `child body[]` represents a sequence of such subtrees.
 *
 * Pseudo-labels, such as `end:` represent the targets of skip fields
 * with the same name (e.g. `ast_skip_t end`).
 *
 * Skips allow skipping a subtree or sequence of subtrees.
 *
 * Sequences of subtrees (i.e. `child []`) have to be terminated by a skip:
 * they don't have a termination tag; all nodes whose position is before the
 *skip
 * are part of the sequence.
 *
 * Skips are encoded as network-byte-order 16-bit offsets counted from the
 * first byte of the node body (i.e. not counting the tag itself).
 * This currently limits the the maximum size of a function body to 64k.
 *
 * Notes:
 *
 * - Some nodes contain skips just for performance or because it simplifies
 * the implementation of the interpreter. For example, technically, the FOR
 * node doesn't need the `body` skip in order to be correctly traversed.
 * However, being able to quickly skip the `iter` expression is useful
 * also because it allows the interpreter to avoid traversing the expression
 * subtree without evaluating it, just in order to find the next subtree.
 *
 * - The name `skip` was chosen because `offset` was too overloaded in general
 * and label` is part of our domain model (i.e. JS has a label AST node type).
 *
 */
const struct ast_node_def ast_node_defs[] = {
    {AST_NAME("NOP") 0, 0, 0, 0}, /* struct {} */

    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t first_var;
     *   child body[];
     * end:
     * }
     */
    {AST_NAME("SCRIPT") 0, 0, 2, 0},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t next;
     *   child decls[];
     * end:
     * }
     */
    {AST_NAME("VAR") 0, 0, 2, 0},
    /*
     * struct {
     *   varint len;
     *   char name[len];
     *   child expr;
     * }
     */
    {AST_NAME("VAR_DECL") 1, 1, 0, 1},
    /*
     * struct {
     *   varint len;
     *   char name[len];
     *   child expr;
     * }
     */
    {AST_NAME("FUNC_DECL") 1, 1, 0, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t end_true;
     *   child cond;
     *   child iftrue[];
     * end_true:
     *   child iffalse[];
     * end:
     * }
     */
    {AST_NAME("IF") 0, 0, 2, 1},
    /*
     * TODO(mkm) distinguish function expressions
     * from function statements.
     * Function statements behave like vars and need a
     * next field for hoisting.
     * We can also ignore the name for function expressions
     * if it's only needed for debugging.
     *
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t first_var;
     *   ast_skip_t body;
     *   child name;
     *   child params[];
     * body:
     *   child body[];
     * end:
     * }
     */
    {AST_NAME("FUNC") 0, 0, 3, 1},
    {AST_NAME("ASSIGN") 0, 0, 0, 2},         /* struct { child left, right; } */
    {AST_NAME("REM_ASSIGN") 0, 0, 0, 2},     /* struct { child left, right; } */
    {AST_NAME("MUL_ASSIGN") 0, 0, 0, 2},     /* struct { child left, right; } */
    {AST_NAME("DIV_ASSIGN") 0, 0, 0, 2},     /* struct { child left, right; } */
    {AST_NAME("XOR_ASSIGN") 0, 0, 0, 2},     /* struct { child left, right; } */
    {AST_NAME("PLUS_ASSIGN") 0, 0, 0, 2},    /* struct { child left, right; } */
    {AST_NAME("MINUS_ASSIGN") 0, 0, 0, 2},   /* struct { child left, right; } */
    {AST_NAME("OR_ASSIGN") 0, 0, 0, 2},      /* struct { child left, right; } */
    {AST_NAME("AND_ASSIGN") 0, 0, 0, 2},     /* struct { child left, right; } */
    {AST_NAME("LSHIFT_ASSIGN") 0, 0, 0, 2},  /* struct { child left, right; } */
    {AST_NAME("RSHIFT_ASSIGN") 0, 0, 0, 2},  /* struct { child left, right; } */
    {AST_NAME("URSHIFT_ASSIGN") 0, 0, 0, 2}, /* struct { child left, right; } */
    {AST_NAME("NUM") 1, 1, 0, 0},    /* struct { varint len, char s[len]; } */
    {AST_NAME("IDENT") 1, 1, 0, 0},  /* struct { varint len, char s[len]; } */
    {AST_NAME("STRING") 1, 1, 0, 0}, /* struct { varint len, char s[len]; } */
    {AST_NAME("REGEX") 1, 1, 0, 0},  /* struct { varint len, char s[len]; } */
    {AST_NAME("LABEL") 1, 1, 0, 0},  /* struct { varint len, char s[len]; } */

    /*
     * struct {
     *   ast_skip_t end;
     *   child body[];
     * end:
     * }
     */
    {AST_NAME("SEQ") 0, 0, 1, 0},
    /*
     * struct {
     *   ast_skip_t end;
     *   child cond;
     *   child body[];
     * end:
     * }
     */
    {AST_NAME("WHILE") 0, 0, 1, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t cond;
     *   child body[];
     * cond:
     *   child cond;
     * end:
     * }
     */
    {AST_NAME("DOWHILE") 0, 0, 2, 0},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t body;
     *   child init;
     *   child cond;
     *   child iter;
     * body:
     *   child body[];
     * end:
     * }
     */
    {AST_NAME("FOR") 0, 0, 2, 3},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t dummy; // allows to quickly promote a for to a for in
     *   child var;
     *   child expr;
     *   child dummy;
     *   child body[];
     * end:
     * }
     */
    {AST_NAME("FOR_IN") 0, 0, 2, 3},
    {AST_NAME("COND") 0, 0, 0, 3}, /* struct { child cond, iftrue, iffalse; } */
    {AST_NAME("DEBUGGER") 0, 0, 0, 0}, /* struct {} */
    {AST_NAME("BREAK") 0, 0, 0, 0},    /* struct {} */

    /*
     * struct {
     *   child label; // TODO(mkm): inline
     * }
     */
    {AST_NAME("LAB_BREAK") 0, 0, 0, 1},
    {AST_NAME("CONTINUE") 0, 0, 0, 0}, /* struct {} */

    /*
     * struct {
     *   child label; // TODO(mkm): inline
     * }
     */
    {AST_NAME("LAB_CONTINUE") 0, 0, 0, 1},
    {AST_NAME("RETURN") 0, 0, 0, 0},     /* struct {} */
    {AST_NAME("VAL_RETURN") 0, 0, 0, 1}, /* struct { child expr; } */
    {AST_NAME("THROW") 0, 0, 0, 1},      /* struct { child expr; } */

    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t catch;
     *   ast_skip_t finally;
     *   child try[];
     * catch:
     *   child var; // TODO(mkm): inline
     *   child catch[];
     * finally:
     *   child finally[];
     * end:
     * }
     */
    {AST_NAME("TRY") 0, 0, 3, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t def;
     *   child expr;
     *   child cases[];
     * def:
     *   child default?; // optional
     * end:
     * }
     */
    {AST_NAME("SWITCH") 0, 0, 2, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   child val;
     *   child stmts[];
     * end:
     * }
     */
    {AST_NAME("CASE") 0, 0, 1, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   child stmts[];
     * end:
     * }
     */
    {AST_NAME("DEFAULT") 0, 0, 1, 0},
    /*
     * struct {
     *   ast_skip_t end;
     *   child expr;
     *   child body[];
     * end:
     * }
     */
    {AST_NAME("WITH") 0, 0, 1, 1},
    {AST_NAME("LOG_OR") 0, 0, 0, 2},      /* struct { child left, right; } */
    {AST_NAME("LOG_AND") 0, 0, 0, 2},     /* struct { child left, right; } */
    {AST_NAME("OR") 0, 0, 0, 2},          /* struct { child left, right; } */
    {AST_NAME("XOR") 0, 0, 0, 2},         /* struct { child left, right; } */
    {AST_NAME("AND") 0, 0, 0, 2},         /* struct { child left, right; } */
    {AST_NAME("EQ") 0, 0, 0, 2},          /* struct { child left, right; } */
    {AST_NAME("EQ_EQ") 0, 0, 0, 2},       /* struct { child left, right; } */
    {AST_NAME("NE") 0, 0, 0, 2},          /* struct { child left, right; } */
    {AST_NAME("NE_NE") 0, 0, 0, 2},       /* struct { child left, right; } */
    {AST_NAME("LE") 0, 0, 0, 2},          /* struct { child left, right; } */
    {AST_NAME("LT") 0, 0, 0, 2},          /* struct { child left, right; } */
    {AST_NAME("GE") 0, 0, 0, 2},          /* struct { child left, right; } */
    {AST_NAME("GT") 0, 0, 0, 2},          /* struct { child left, right; } */
    {AST_NAME("IN") 0, 0, 0, 2},          /* struct { child left, right; } */
    {AST_NAME("INSTANCEOF") 0, 0, 0, 2},  /* struct { child left, right; } */
    {AST_NAME("LSHIFT") 0, 0, 0, 2},      /* struct { child left, right; } */
    {AST_NAME("RSHIFT") 0, 0, 0, 2},      /* struct { child left, right; } */
    {AST_NAME("URSHIFT") 0, 0, 0, 2},     /* struct { child left, right; } */
    {AST_NAME("ADD") 0, 0, 0, 2},         /* struct { child left, right; } */
    {AST_NAME("SUB") 0, 0, 0, 2},         /* struct { child left, right; } */
    {AST_NAME("REM") 0, 0, 0, 2},         /* struct { child left, right; } */
    {AST_NAME("MUL") 0, 0, 0, 2},         /* struct { child left, right; } */
    {AST_NAME("DIV") 0, 0, 0, 2},         /* struct { child left, right; } */
    {AST_NAME("POS") 0, 0, 0, 1},         /* struct { child expr; } */
    {AST_NAME("NEG") 0, 0, 0, 1},         /* struct { child expr; } */
    {AST_NAME("NOT") 0, 0, 0, 1},         /* struct { child expr; } */
    {AST_NAME("LOGICAL_NOT") 0, 0, 0, 1}, /* struct { child expr; } */
    {AST_NAME("VOID") 0, 0, 0, 1},        /* struct { child expr; } */
    {AST_NAME("DELETE") 0, 0, 0, 1},      /* struct { child expr; } */
    {AST_NAME("TYPEOF") 0, 0, 0, 1},      /* struct { child expr; } */
    {AST_NAME("PREINC") 0, 0, 0, 1},      /* struct { child expr; } */
    {AST_NAME("PREDEC") 0, 0, 0, 1},      /* struct { child expr; } */
    {AST_NAME("POSTINC") 0, 0, 0, 1},     /* struct { child expr; } */
    {AST_NAME("POSTDEC") 0, 0, 0, 1},     /* struct { child expr; } */

    /*
     * struct {
     *   varint len;
     *   char ident[len];
     *   child expr;
     * }
     */
    {AST_NAME("MEMBER") 1, 1, 0, 1},
    /*
     * struct {
     *   child expr;
     *   child index;
     * }
     */
    {AST_NAME("INDEX") 0, 0, 0, 2},
    /*
     * struct {
     *   ast_skip_t end;
     *   child expr;
     *   child args[];
     * end:
     * }
     */
    {AST_NAME("CALL") 0, 0, 1, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   child expr;
     *   child args[];
     * end:
     * }
     */
    {AST_NAME("NEW") 0, 0, 1, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   child elements[];
     * end:
     * }
     */
    {AST_NAME("ARRAY") 0, 0, 1, 0},
    /*
     * struct {
     *   ast_skip_t end;
     *   child props[];
     * end:
     * }
     */
    {AST_NAME("OBJECT") 0, 0, 1, 0},
    /*
     * struct {
     *   varint len;
     *   char name[len];
     *   child expr;
     * }
     */
    {AST_NAME("PROP") 1, 1, 0, 1},
    /*
     * struct {
     *   child func;
     * }
     */
    {AST_NAME("GETTER") 0, 0, 0, 1},
    /*
     * struct {
     *   child func;
     * end:
     * }
     */
    {AST_NAME("SETTER") 0, 0, 0, 1},
    {AST_NAME("THIS") 0, 0, 0, 0},       /* struct {} */
    {AST_NAME("TRUE") 0, 0, 0, 0},       /* struct {} */
    {AST_NAME("FALSE") 0, 0, 0, 0},      /* struct {} */
    {AST_NAME("NULL") 0, 0, 0, 0},       /* struct {} */
    {AST_NAME("UNDEF") 0, 0, 0, 0},      /* struct {} */
    {AST_NAME("USE_STRICT") 0, 0, 0, 0}, /* struct {} */
};

V7_STATIC_ASSERT(AST_MAX_TAG < 256, ast_tag_should_fit_in_char);
V7_STATIC_ASSERT(AST_MAX_TAG == ARRAY_SIZE(ast_node_defs), bad_node_defs);

/*
 * Begins an AST node by appending a tag to the AST.
 *
 * It also allocates space for the fixed_size payload and the space for
 * the skips.
 *
 * The caller is responsible for appending children.
 *
 * Returns the offset of the node payload (one byte after the tag).
 * This offset can be passed to `ast_set_skip`.
 */
V7_PRIVATE ast_off_t ast_add_node(struct ast *a, enum ast_tag tag) {
  ast_off_t start = a->mbuf.len;
  uint8_t t = (uint8_t) tag;
  const struct ast_node_def *d = &ast_node_defs[tag];

  assert(tag < AST_MAX_TAG);

  mbuf_append(&a->mbuf, (char *) &t, sizeof(t));
  mbuf_append(&a->mbuf, NULL, sizeof(ast_skip_t) * d->num_skips);
  return start + 1;
}

V7_PRIVATE ast_off_t
ast_insert_node(struct ast *a, ast_off_t start, enum ast_tag tag) {
  uint8_t t = (uint8_t) tag;
  const struct ast_node_def *d = &ast_node_defs[tag];

  assert(tag < AST_MAX_TAG);

  mbuf_insert(&a->mbuf, start, NULL, sizeof(ast_skip_t) * d->num_skips);
  mbuf_insert(&a->mbuf, start, (char *) &t, sizeof(t));

  if (d->num_skips) {
    ast_set_skip(a, start + 1, AST_END_SKIP);
  }

  return start + 1;
}

V7_STATIC_ASSERT(sizeof(ast_skip_t) == 2, ast_skip_t_len_should_be_2);

/*
 * Patches a given skip slot for an already emitted node with the
 * current write cursor position (e.g. AST length).
 *
 * This is intended to be invoked when a node with a variable number
 * of child subtrees is closed, or when the consumers need a shortcut
 * to the next sibling.
 *
 * Each node type has a different number and semantic for skips,
 * all of them defined in the `ast_which_skip` enum.
 * All nodes having a variable number of child subtrees must define
 * at least the `AST_END_SKIP` skip, which effectively skips a node
 * boundary.
 *
 * Every tree reader can assume this and safely skip unknown nodes.
 */
V7_PRIVATE ast_off_t
ast_set_skip(struct ast *a, ast_off_t start, enum ast_which_skip skip) {
  return ast_modify_skip(a, start, a->mbuf.len, skip);
}

/*
 * Patches a given skip slot for an already emitted node with the value
 * (stored as delta relative to the `start` node) of the `where` argument.
 */
V7_PRIVATE ast_off_t ast_modify_skip(struct ast *a, ast_off_t start,
                                     ast_off_t where,
                                     enum ast_which_skip skip) {
  uint8_t *p = (uint8_t *) a->mbuf.buf + start + skip * sizeof(ast_skip_t);
  uint16_t delta = where - start;
  enum ast_tag tag = (enum ast_tag)(uint8_t) * (a->mbuf.buf + start - 1);
  const struct ast_node_def *def = &ast_node_defs[tag];

  /* assertion, to be optimizable out */
  assert((int) skip < def->num_skips);

  p[0] = delta >> 8;
  p[1] = delta & 0xff;
  return where;
}

V7_PRIVATE ast_off_t
ast_get_skip(struct ast *a, ast_off_t pos, enum ast_which_skip skip) {
  uint8_t *p;
  assert(pos + skip * sizeof(ast_skip_t) < a->mbuf.len);
  p = (uint8_t *) a->mbuf.buf + pos + skip * sizeof(ast_skip_t);
  return pos + (p[1] | p[0] << 8);
}

V7_PRIVATE enum ast_tag ast_fetch_tag(struct ast *a, ast_off_t *pos) {
  assert(*pos < a->mbuf.len);
  return (enum ast_tag)(uint8_t) * (a->mbuf.buf + (*pos)++);
}

/*
 * Assumes a cursor positioned right after a tag.
 *
 * TODO(mkm): add doc, find better name.
 */
V7_PRIVATE void ast_move_to_children(struct ast *a, ast_off_t *pos) {
  enum ast_tag tag = (enum ast_tag)(uint8_t) * (a->mbuf.buf + *pos - 1);
  const struct ast_node_def *def = &ast_node_defs[tag];
  assert(*pos - 1 < a->mbuf.len);
  if (def->has_varint) {
    int llen;
    size_t slen = decode_varint((unsigned char *) a->mbuf.buf + *pos, &llen);
    *pos += llen;
    if (def->has_inlined) {
      *pos += slen;
    }
  }

  *pos += def->num_skips * sizeof(ast_skip_t);
}

/* Helper to add a node with inlined data. */
V7_PRIVATE void ast_add_inlined_node(struct ast *a, enum ast_tag tag,
                                     const char *name, size_t len) {
  assert(ast_node_defs[tag].has_inlined);
  embed_string(&a->mbuf, ast_add_node(a, tag), name, len, 0, 1);
}

/* Helper to add a node with inlined data. */
V7_PRIVATE void ast_insert_inlined_node(struct ast *a, ast_off_t start,
                                        enum ast_tag tag, const char *name,
                                        size_t len) {
  assert(ast_node_defs[tag].has_inlined);
  embed_string(&a->mbuf, ast_insert_node(a, start, tag), name, len, 0, 1);
}

V7_PRIVATE char *ast_get_inlined_data(struct ast *a, ast_off_t pos, size_t *n) {
  int llen;
  assert(pos < a->mbuf.len);
  *n = decode_varint((unsigned char *) a->mbuf.buf + pos, &llen);
  return a->mbuf.buf + pos + llen;
}

V7_PRIVATE void ast_get_num(struct ast *a, ast_off_t pos, double *val) {
  char buf[512];
  char *str;
  size_t str_len;
  str = ast_get_inlined_data(a, pos, &str_len);
  if (str_len >= sizeof(buf)) {
    str_len = sizeof(buf) - 1;
  }
  memcpy(buf, str, str_len);
  buf[str_len] = '\0';
  *val = strtod(buf, NULL);
}

static void comment_at_depth(FILE *fp, const char *fmt, int depth, ...) {
  int i;
  char buf[265];
  va_list ap;
  va_start(ap, depth);

  vsnprintf(buf, sizeof(buf), fmt, ap);

  for (i = 0; i < depth; i++) {
    fprintf(fp, "  ");
  }
  fprintf(fp, "/* [%s] */\n", buf);
}

V7_PRIVATE void ast_skip_tree(struct ast *a, ast_off_t *pos) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  const struct ast_node_def *def = &ast_node_defs[tag];
  ast_off_t skips = *pos;
  int i;
  ast_move_to_children(a, pos);

  for (i = 0; i < def->num_subtrees; i++) {
    ast_skip_tree(a, pos);
  }

  if (ast_node_defs[tag].num_skips) {
    ast_off_t end = ast_get_skip(a, skips, AST_END_SKIP);

    while (*pos < end) {
      ast_skip_tree(a, pos);
    }
  }
}

static void ast_dump_tree(FILE *fp, struct ast *a, ast_off_t *pos, int depth) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  const struct ast_node_def *def = &ast_node_defs[tag];
  ast_off_t skips = *pos;
  size_t slen;
  int i, llen;

  for (i = 0; i < depth; i++) {
    fprintf(fp, "  ");
  }

#ifndef V7_DISABLE_AST_TAG_NAMES
  fprintf(fp, "%s", def->name);
#else
  fprintf(fp, "TAG_%d", tag);
#endif

  if (def->has_inlined) {
    slen = decode_varint((unsigned char *) a->mbuf.buf + *pos, &llen);
    fprintf(fp, " %.*s\n", (int) slen, a->mbuf.buf + *pos + llen);
  } else {
    fprintf(fp, "\n");
  }

  ast_move_to_children(a, pos);

  for (i = 0; i < def->num_subtrees; i++) {
    ast_dump_tree(fp, a, pos, depth + 1);
  }

  if (ast_node_defs[tag].num_skips) {
    /*
     * first skip always encodes end of the last children sequence.
     * so unless we care how the subtree sequences are grouped together
     * (and we currently don't) we can just read until the end of that skip.
     */
    ast_off_t end = ast_get_skip(a, skips, AST_END_SKIP);

    comment_at_depth(fp, "...", depth + 1);
    while (*pos < end) {
      int s;
      for (s = ast_node_defs[tag].num_skips - 1; s > 0; s--) {
        if (*pos == ast_get_skip(a, skips, (enum ast_which_skip) s)) {
          comment_at_depth(fp, "%d ->", depth + 1, s);
          break;
        }
      }
      ast_dump_tree(fp, a, pos, depth + 1);
    }
  }
}

V7_PRIVATE void ast_init(struct ast *ast, size_t len) {
  mbuf_init(&ast->mbuf, len);
}

V7_PRIVATE void ast_optimize(struct ast *ast) {
  mbuf_trim(&ast->mbuf);
}

V7_PRIVATE void ast_free(struct ast *ast) {
  mbuf_free(&ast->mbuf);
}

/* Dumps an AST to stdout. */
V7_PRIVATE void ast_dump(FILE *fp, struct ast *a, ast_off_t pos) {
  ast_dump_tree(fp, a, &pos, 0);
}
