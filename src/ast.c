/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

typedef unsigned short ast_skip_t;

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
 * they don't have a termination tag; all nodes whose position is before the skip
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
  {"NOP", 0, 0, 0, 0},  /* struct {} */
  /*
   * struct {
   *   ast_skip_t end;
   *   ast_skip_t first_var;
   *   child body[];
   * end:
   * }
   */
  {"SCRIPT", 0, 0, 2, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   ast_skip_t next;
   *   child decls[];
   * end:
   * }
   */
  {"VAR", 0, 0, 2, 0},
  /*
   * struct {
   *   varint len;
   *   char name[len];
   *   child expr;
   * }
   */
  {"VAR_DECL", 1, 1, 0, 1},
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
  {"IF", 0, 0, 2, 1},
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
  {"FUNC", 0, 0, 3, 1},
  {"ASSIGN", 0, 0, 0, 2},         /* struct { child left, right; } */
  {"REM_ASSIGN", 0, 0, 0, 2},     /* struct { child left, right; } */
  {"MUL_ASSIGN", 0, 0, 0, 2},     /* struct { child left, right; } */
  {"DIV_ASSIGN", 0, 0, 0, 2},     /* struct { child left, right; } */
  {"XOR_ASSIGN", 0, 0, 0, 2},     /* struct { child left, right; } */
  {"PLUS_ASSIGN", 0, 0, 0, 2},    /* struct { child left, right; } */
  {"MINUS_ASSIGN", 0, 0, 0, 2},   /* struct { child left, right; } */
  {"OR_ASSIGN", 0, 0, 0, 2},      /* struct { child left, right; } */
  {"AND_ASSIGN", 0, 0, 0, 2},     /* struct { child left, right; } */
  {"LSHIFT_ASSIGN", 0, 0, 0, 2},  /* struct { child left, right; } */
  {"RSHIFT_ASSIGN", 0, 0, 0, 2},  /* struct { child left, right; } */
  {"URSHIFT_ASSIGN", 0, 0, 0, 2}, /* struct { child left, right; } */
  {"NUM", 1, 1, 0, 0},            /* struct { varint len, char s[len]; } */
  {"IDENT", 1, 1, 0, 0},          /* struct { varint len, char s[len]; } */
  {"STRING", 1, 1, 0, 0},         /* struct { varint len, char s[len]; } */
  {"REGEX", 1, 1, 0, 0},          /* struct { varint len, char s[len]; } */
  {"LABEL", 1, 1, 0, 0},          /* struct { varint len, char s[len]; } */
  /*
   * struct {
   *   ast_skip_t end;
   *   child body[];
   * end:
   * }
   */
  {"SEQ", 0, 0, 1, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   child cond;
   *   child body[];
   * end:
   * }
   */
  {"WHILE", 0, 0, 1, 1},
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
  {"DOWHILE", 0, 0, 2, 0},
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
  {"FOR", 0, 0, 2, 3},
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
  {"FOR_IN", 0, 0, 2, 3},
  {"COND", 0, 0, 0, 3},  /* struct { child cond, iftrue, iffalse; } */
  {"DEBUGGER", 0, 0, 0, 0},  /* struct {} */
  {"BREAK", 0, 0, 0, 0},     /* struct {} */
  /*
   * struct {
   *   child label; // TODO(mkm): inline
   * }
   */
  {"LAB_BREAK", 0, 0, 0, 1},
  {"CONTINUE", 0, 0, 0, 0},  /* struct {} */
  /*
   * struct {
   *   child label; // TODO(mkm): inline
   * }
   */
  {"LAB_CONTINUE", 0, 0, 0, 1},
  {"RETURN", 0, 0, 0, 0},     /* struct {} */
  {"VAL_RETURN", 0, 0, 0, 1}, /* struct { child expr; } */
  {"THROW", 0, 0, 0, 1},      /* struct { child expr; } */
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
  {"TRY", 0, 0, 3, 1},
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
  {"SWITCH", 0, 0, 2, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child val;
   *   child stmts[];
   * end:
   * }
   */
  {"CASE", 0, 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child stmts[];
   * end:
   * }
   */
  {"DEFAULT", 0, 0, 1, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   child expr;
   *   child body[];
   * end:
   * }
   */
  {"WITH", 0, 0, 1, 1},
  {"LOG_OR", 0, 0, 0, 2},      /* struct { child left, right; } */
  {"LOG_AND", 0, 0, 0, 2},     /* struct { child left, right; } */
  {"OR", 0, 0, 0, 2},          /* struct { child left, right; } */
  {"XOR", 0, 0, 0, 2},         /* struct { child left, right; } */
  {"AND", 0, 0, 0, 2},         /* struct { child left, right; } */
  {"EQ", 0, 0, 0, 2},          /* struct { child left, right; } */
  {"EQ_EQ", 0, 0, 0, 2},       /* struct { child left, right; } */
  {"NE", 0, 0, 0, 2},          /* struct { child left, right; } */
  {"NE_NE", 0, 0, 0, 2},       /* struct { child left, right; } */
  {"LE", 0, 0, 0, 2},          /* struct { child left, right; } */
  {"LT", 0, 0, 0, 2},          /* struct { child left, right; } */
  {"GE", 0, 0, 0, 2},          /* struct { child left, right; } */
  {"GT", 0, 0, 0, 2},          /* struct { child left, right; } */
  {"IN", 0, 0, 0, 2},          /* struct { child left, right; } */
  {"INSTANCEOF", 0, 0, 0, 2},  /* struct { child left, right; } */
  {"LSHIFT", 0, 0, 0, 2},      /* struct { child left, right; } */
  {"RSHIFT", 0, 0, 0, 2},      /* struct { child left, right; } */
  {"URSHIFT", 0, 0, 0, 2},     /* struct { child left, right; } */
  {"ADD", 0, 0, 0, 2},         /* struct { child left, right; } */
  {"SUB", 0, 0, 0, 2},         /* struct { child left, right; } */
  {"REM", 0, 0, 0, 2},         /* struct { child left, right; } */
  {"MUL", 0, 0, 0, 2},         /* struct { child left, right; } */
  {"DIV", 0, 0, 0, 2},         /* struct { child left, right; } */
  {"POS", 0, 0, 0, 1},         /* struct { child expr; } */
  {"NEG", 0, 0, 0, 1},         /* struct { child expr; } */
  {"NOT", 0, 0, 0, 1},         /* struct { child expr; } */
  {"LOGICAL_NOT", 0, 0, 0, 1}, /* struct { child expr; } */
  {"VOID", 0, 0, 0, 1},        /* struct { child expr; } */
  {"DELETE", 0, 0, 0, 1},      /* struct { child expr; } */
  {"TYPEOF", 0, 0, 0, 1},      /* struct { child expr; } */
  {"PREINC", 0, 0, 0, 1},      /* struct { child expr; } */
  {"PREDEC", 0, 0, 0, 1},      /* struct { child expr; } */
  {"POSTINC", 0, 0, 0, 1},     /* struct { child expr; } */
  {"POSTDEC", 0, 0, 0, 1},     /* struct { child expr; } */
  /*
   * struct {
   *   varint len;
   *   char ident[len];
   *   child expr;
   * }
   */
  {"MEMBER", 1, 1, 0, 1},
  /*
   * struct {
   *   child expr;
   *   child index;
   * }
   */
  {"INDEX", 0, 0, 0, 2},
  /*
   * struct {
   *   ast_skip_t end;
   *   child expr;
   *   child args[];
   * end:
   * }
   */
  {"CALL", 0, 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child expr;
   *   child args[];
   * end:
   * }
   */
  {"NEW", 0, 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child elements[];
   * end:
   * }
   */
  {"ARRAY", 0, 0, 1, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   child props[];
   * end:
   * }
   */
  {"OBJECT", 0, 0, 1, 0},
  /*
   * struct {
   *   varint len;
   *   char name[len];
   *   child expr;
   * }
   */
  {"PROP", 1, 1, 0, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child name; // TODO(mkm): inline
   *   child body[];
   * end:
   * }
   */
  {"GETTER", 0, 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child name; // TODO(mkm): inline
   *   child param; // TODO(mkm): reuse func decl?
   *   child body[];
   * end:
   * }
   */
  {"SETTER", 0, 0, 1, 2},
  {"THIS", 0, 0, 0, 0},  /* struct {} */
  {"TRUE", 0, 0, 0, 0},  /* struct {} */
  {"FALSE", 0, 0, 0, 0}, /* struct {} */
  {"NULL", 0, 0, 0, 0},  /* struct {} */
  {"UNDEF", 0, 0, 0, 0}, /* struct {} */
};

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

  mbuf_append(&a->mbuf, (char *)&t, sizeof(t));
  mbuf_append(&a->mbuf, NULL, sizeof(ast_skip_t) * d->num_skips);
  return start + 1;
}

V7_PRIVATE ast_off_t ast_insert_node(struct ast *a, ast_off_t start,
                                     enum ast_tag tag) {
  uint8_t t = (uint8_t) tag;
  const struct ast_node_def *d = &ast_node_defs[tag];

  assert(tag < AST_MAX_TAG);

  mbuf_insert(&a->mbuf, start, NULL, sizeof(ast_skip_t) * d->num_skips);
  mbuf_insert(&a->mbuf, start, (char *)&t, sizeof(t));

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
V7_PRIVATE ast_off_t ast_set_skip(struct ast *a, ast_off_t start,
                                  enum ast_which_skip skip) {
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
  enum ast_tag tag = (enum ast_tag) (uint8_t) * (a->mbuf.buf + start - 1);
  const struct ast_node_def *def = &ast_node_defs[tag];

  /* assertion, to be optimizable out */
  assert((int) skip < def->num_skips);

  p[0] = delta >> 8;
  p[1] = delta & 0xff;
  return where;
}

V7_PRIVATE ast_off_t ast_get_skip(struct ast *a, ast_off_t pos,
                                  enum ast_which_skip skip) {
  uint8_t *p;
  assert(pos + skip * sizeof(ast_skip_t) < a->mbuf.len);
  p = (uint8_t *) a->mbuf.buf + pos + skip * sizeof(ast_skip_t);
  return pos + (p[1] | p[0] << 8);
}

V7_PRIVATE enum ast_tag ast_fetch_tag(struct ast *a, ast_off_t *pos) {
  assert(*pos < a->mbuf.len);
  return (enum ast_tag) (uint8_t) * (a->mbuf.buf + (*pos)++);
}

/*
 * Assumes a cursor positioned right after a tag.
 *
 * TODO(mkm): add doc, find better name.
 */
V7_PRIVATE void ast_move_to_children(struct ast *a, ast_off_t *pos) {
  enum ast_tag tag = (enum ast_tag) (uint8_t) * (a->mbuf.buf + *pos - 1);
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
  embed_string(&a->mbuf, ast_add_node(a, tag), name, len);
}

/* Helper to add a node with inlined data. */
V7_PRIVATE void ast_insert_inlined_node(struct ast *a, ast_off_t start,
                                        enum ast_tag tag, const char *name,
                                        size_t len) {
  assert(ast_node_defs[tag].has_inlined);
  embed_string(&a->mbuf, ast_insert_node(a, start, tag), name, len);
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

  fprintf(fp, "%s", def->name);

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
        if (*pos == ast_get_skip(a, skips, s)) {
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
