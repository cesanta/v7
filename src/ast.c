/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

typedef unsigned short ast_skip_t;

struct ast_node_def {
  const char *name;  /* tag name, for debugging and serialization */
  size_t fixed_len;  /* bytes */
  int num_skips;     /* number of skips */
  int num_subtrees;  /* number of fixed subtrees */
};

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
V7_PRIVATE struct ast_node_def ast_node_defs[] = {
  {"NOP", 0, 0, 0}, /* struct {} */
  /*
   * struct {
   *   ast_skip_t end;
   *   child body[];
   * end:
   * }
   */
  {"SCRIPT", 0, 1, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   child decls[];
   * end:
   * }
   */
  {"VAR", 0, 1, 0},
  /*
   * struct {
   *   child name; // TODO(mkm): inline
   *   child expr;
   * }
   */
  {"VAR_DECL", 0, 0, 2},
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
  {"IF", 0, 2, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   ast_skip_t body;
   *   child name;
   *   child params[];
   * body:
   *   child body[];
   * end:
   * }
   */
  {"FUNC", 0, 2, 1},
  {"ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"REM_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"MUL_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"DIV_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"XOR_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"PLUS_ASSIGN", 0, 0, 2}, /* struct { child left, right; } */
  {"MINUS_ASSIGN", 0, 0, 2},   /* struct { child left, right; } */
  {"OR_ASSIGN", 0, 0, 2},      /* struct { child left, right; } */
  {"AND_ASSIGN", 0, 0, 2},     /* struct { child left, right; } */
  {"LSHIFT_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"RSHIFT_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"URSHIFT_ASSIGN", 0, 0, 2}, /* struct { child left, right; } */
  {"IDENT", 4 + sizeof(char *), 0, 0},  /* struct { char var; } */
  {"NUM", 8, 0, 0},                     /* struct { double n; } */
  {"STRING", 4 + sizeof(char *), 0, 0}, /* struct { uint32_t len, char *s; } */
  /*
   * struct {
   *   ast_skip_t end;
   *   child body[];
   * end:
   * }
   */
  {"SEQ", 0, 1, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   child cond;
   *   child body[];
   * end:
   * }
   */
  {"WHILE", 0, 1, 1},
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
  {"DOWHILE", 0, 2, 0},
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
  {"FOR", 0, 2, 3},
  {"COND", 0, 0, 3},     /* struct { child cond, iftrue, iffalse; } */
  {"DEBUGGER", 0, 0, 0}, /* struct {} */
  {"BREAK", 0, 0, 0},    /* struct {} */
  /*
   * struct {
   *   child label; // TODO(mkm): inline
   * }
   */
  {"LAB_BREAK", 0, 0, 1},
  {"CONTINUE", 0, 0, 0},  /* struct {} */
  /*
   * struct {
   *   child label; // TODO(mkm): inline
   * }
   */
  {"LAB_CONTINUE", 0, 0, 1},
  {"RETURN", 0, 0, 0},     /* struct {} */
  {"VAL_RETURN", 0, 0, 1}, /* struct { child expr; } */
  {"THROW", 0, 0, 1},      /* struct { child expr; } */
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
  {"TRY", 0, 3, 1},
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
  {"SWITCH", 0, 2, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child val;
   *   child stmts[];
   * end:
   * }
   */
  {"CASE", 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child stmts[];
   * end:
   * }
   */
  {"DEFAULT", 0, 1, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   child expr;
   *   child body[];
   * end:
   * }
   */
  {"WITH", 0, 1, 1},
  {"LOG_OR", 0, 0, 2},  /* struct { child left, right; } */
  {"LOG_AND", 0, 0, 2}, /* struct { child left, right; } */
  {"OR", 0, 0, 2},      /* struct { child left, right; } */
  {"XOR", 0, 0, 2},     /* struct { child left, right; } */
  {"AND", 0, 0, 2},     /* struct { child left, right; } */
  {"EQ", 0, 0, 2},      /* struct { child left, right; } */
  {"EQ_EQ", 0, 0, 2},   /* struct { child left, right; } */
  {"NE", 0, 0, 2},      /* struct { child left, right; } */
  {"NE_NE", 0, 0, 2},   /* struct { child left, right; } */
  {"LE", 0, 0, 2},      /* struct { child left, right; } */
  {"LT", 0, 0, 2},      /* struct { child left, right; } */
  {"GE", 0, 0, 2},      /* struct { child left, right; } */
  {"GT", 0, 0, 2},      /* struct { child left, right; } */
  {"IN", 0, 0, 2},      /* struct { child left, right; } */
  {"INSTANCEOF", 0, 0, 2},  /* struct { child left, right; } */
  {"LSHIFT", 0, 0, 2},      /* struct { child left, right; } */
  {"RSHIFT", 0, 0, 2},      /* struct { child left, right; } */
  {"URSHIFT", 0, 0, 2},     /* struct { child left, right; } */
  {"ADD", 0, 0, 2},         /* struct { child left, right; } */
  {"SUB", 0, 0, 2},         /* struct { child left, right; } */
  {"REM", 0, 0, 2},         /* struct { child left, right; } */
  {"MUL", 0, 0, 2},         /* struct { child left, right; } */
  {"DIV", 0, 0, 2},         /* struct { child left, right; } */
  {"POS", 0, 0, 1},         /* struct { child expr; } */
  {"NEG", 0, 0, 1},         /* struct { child expr; } */
  {"NOT", 0, 0, 1},         /* struct { child expr; } */
  {"LOGICAL_NOT", 0, 0, 1}, /* struct { child expr; } */
  {"VOID", 0, 0, 1},        /* struct { child expr; } */
  {"DELETE", 0, 0, 1},      /* struct { child expr; } */
  {"TYPEOF", 0, 0, 1},      /* struct { child expr; } */
  {"PREINC", 0, 0, 1},      /* struct { child expr; } */
  {"PREDEC", 0, 0, 1},      /* struct { child expr; } */
  {"POSTINC", 0, 0, 1},     /* struct { child expr; } */
  {"POSTDEC", 0, 0, 1},     /* struct { child expr; } */
  /*
   * struct {
   *   child expr;
   *   child ident; // TODO(mkm): inline
   * }
   */
  {"MEMBER", 0, 0, 2},
  /*
   * struct {
   *   child expr;
   *   child index;
   * }
   */
  {"INDEX", 0, 0, 2},
  /*
   * struct {
   *   ast_skip_t end;
   *   child expr;
   *   child args[];
   * end:
   * }
   */
  {"CALL", 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child expr;
   *   child args[];
   * end:
   * }
   */
  {"NEW", 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child elements[];
   * end:
   * }
   */
  {"ARRAY", 0, 1, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   child props[];
   * end:
   * }
   */
  {"OBJECT", 0, 1, 0},
  /*
   * struct {
   *   child name; // TODO(mkm): inline
   *   child expr;
   * }
   */
  {"PROP", 0, 0, 2},
  /*
   * struct {
   *   ast_skip_t end;
   *   child name; // TODO(mkm): inline
   *   child body[];
   * end:
   * }
   */
  {"GETTER", 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child name; // TODO(mkm): inline
   *   child param; // TODO(mkm): reuse func decl?
   *   child body[];
   * end:
   * }
   */
  {"SETTER", 0, 1, 2},
  {"THIS", 0, 0, 0},  /* struct {} */
  {"TRUE", 0, 0, 0},  /* struct {} */
  {"FALSE", 0, 0, 0}, /* struct {} */
  {"NULL", 0, 0, 0},  /* struct {} */
  {"UNDEF", 0, 0, 0}, /* struct {} */
};

V7_STATIC_ASSERT(AST_MAX_TAG == ARRAY_SIZE(ast_node_defs), bad_node_defs);

/*
 * Code and API based on Fossa IO buffers.
 * TODO(mkm): optimize to our specific use case or fully reuse fossa.
 */

/* Initializes an AST buffer. */
V7_PRIVATE void ast_init(struct ast *ast, size_t initial_size) {
  ast->len = ast->size = 0;
  ast->buf = NULL;
  ast_resize(ast, initial_size);
}

/* Frees the space allocated for the iobuffer and resets the iobuf structure. */
V7_PRIVATE void ast_free(struct ast *ast) {
  if (ast->buf != NULL) {
    free(ast->buf);
    ast_init(ast, 0);
  }
}

/*
 * Resize an AST buffer.
 *
 * If `new_size` is smaller than buffer's `len`, the
 * resize is not performed.
 */
V7_PRIVATE void ast_resize(struct ast *a, size_t new_size) {
  char *p;
  if ((new_size > a->size || (new_size < a->size && new_size >= a->len)) &&
      (p = (char *) realloc(a->buf, new_size)) != NULL) {
    a->size = new_size;
    a->buf = p;
  }
}

/* Shrinks the ast size to just fit it's length. */
V7_PRIVATE void ast_trim(struct ast *ast) {
  ast_resize(ast, ast->len);
}

/*
 * Appends data to the AST.
 *
 * It returns the amount of bytes appended.
 */
V7_PRIVATE size_t ast_append(struct ast *a, const char *buf, size_t len) {
  return ast_insert(a, a->len, buf, len);
}

/*
 * Inserts data at a specified offset in the AST.
 *
 * Existing data will be shifted forwards and the buffer will
 * be grown if necessary.
 * It returns the amount of bytes inserted.
 */
V7_PRIVATE size_t ast_insert(struct ast *a, size_t off, const char *buf,
                             size_t len) {
  char *p = NULL;

  assert(a != NULL);
  assert(a->len <= a->size);
  assert(off <= a->len);

  /* check overflow */
  if (~(size_t)0 - (size_t)a->buf < len)
    return 0;

  if (a->len + len <= a->size) {
    memmove(a->buf + off + len, a->buf + off, a->len - off);
    if (buf != NULL) {
      memcpy(a->buf + off, buf, len);
    }
    a->len += len;
  } else if ((p = (char *)
              realloc(a->buf,
                      (a->len + len) * AST_SIZE_MULTIPLIER)) != NULL) {
    a->buf = p;
    memmove(a->buf + off + len, a->buf + off, a->len - off);
    if (buf != NULL) {
      memcpy(a->buf + off, buf, len);
    }
    a->len += len;
    a->size = a->len * AST_SIZE_MULTIPLIER;
  } else {
    len = 0;
  }

  return len;
}

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
V7_PRIVATE size_t ast_add_node(struct ast *a, enum ast_tag tag) {
  size_t start = a->len;
  uint8_t t = (uint8_t) tag;
  struct ast_node_def *d = &ast_node_defs[tag];

  assert(tag < AST_MAX_TAG);

  ast_append(a, (char *)&t, sizeof(t));
  ast_append(a, NULL, d->fixed_len + sizeof(ast_skip_t) * d->num_skips);
  return start + 1;
}

V7_PRIVATE size_t ast_insert_node(struct ast *a, size_t start,
                                  enum ast_tag tag) {
  uint8_t t = (uint8_t) tag;
  struct ast_node_def *d = &ast_node_defs[tag];

  assert(tag < AST_MAX_TAG);

  ast_insert(a, start, NULL, d->fixed_len + sizeof(ast_skip_t) * d->num_skips);
  ast_insert(a, start, (char *)&t, sizeof(t));

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
V7_PRIVATE size_t ast_set_skip(struct ast *a, size_t start,
                               enum ast_which_skip skip) {
  uint8_t *p = (uint8_t *) a->buf + start + skip * sizeof(ast_skip_t);
  uint16_t delta = a->len - start;
  enum ast_tag tag;

  /* assertion, to be optimizable out */
  tag = (enum ast_tag) (uint8_t) * (a->buf + start - 1);
  struct ast_node_def *def = &ast_node_defs[tag];
  assert((int) skip < def->num_skips);

  p[0] = delta >> 8;
  p[1] = delta & 0xff;
  return a->len;
}

V7_PRIVATE size_t ast_get_skip(struct ast *a, ast_off_t pos,
                               enum ast_which_skip skip) {
  uint8_t * p = (uint8_t *) a->buf + pos + skip * sizeof(ast_skip_t);
  return pos + (p[1] | p[0] << 8);
}

V7_PRIVATE enum ast_tag ast_fetch_tag(struct ast *a, ast_off_t *pos) {
  return (enum ast_tag) (uint8_t) * (a->buf + (*pos)++);
}

/*
 * Assumes a cursor positioned right after a tag.
 *
 * TODO(mkm): add doc, find better name.
 */
V7_PRIVATE void ast_move_to_children(struct ast *a, ast_off_t *pos) {
  enum ast_tag tag;
  tag = (enum ast_tag) (uint8_t) * (a->buf + *pos - 1);
  struct ast_node_def *def = &ast_node_defs[tag];
  *pos += def->fixed_len + def->num_skips * sizeof(ast_skip_t);
}

/* Helper to add a NUM node. */
V7_PRIVATE void ast_add_num(struct ast *a, double num) {
  size_t start = ast_add_node(a, AST_NUM);
  memcpy(a->buf + start, &num, sizeof(num));
}

static void ast_set_string(char *buf, const char *name, size_t len) {
  uint32_t slen = (uint32_t) len;
  /* 4GB ought to be enough for anybody */
  if (sizeof(size_t) == 8) {
    assert((len & 0xFFFFFFFF00000000) == 0);
  }
  memcpy(buf, &slen, sizeof(slen));
  memcpy(buf + sizeof(slen), &name, sizeof(char *));
}

/* Helper to add an IDENT node. */
V7_PRIVATE void ast_add_ident(struct ast *a, const char *name, size_t len) {
  size_t start = ast_add_node(a, AST_IDENT);
  ast_set_string(a->buf + start, name, len);
}

/* Helper to add a STRING node. */
V7_PRIVATE void ast_add_string(struct ast *a, const char *name, size_t len) {
  size_t start = ast_add_node(a, AST_STRING);
  ast_set_string(a->buf + start, name, len);
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

static void ast_dump_tree(FILE *fp, struct ast *a, ast_off_t *pos, int depth) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  struct ast_node_def *def = &ast_node_defs[tag];
  ast_off_t skips = *pos;
  int i;
  double dv;

  for (i = 0; i < depth; i++) {
    fprintf(fp, "  ");
  }

  fprintf(fp, "%s", def->name);

  switch (tag) {
    case AST_NUM:
      memcpy(&dv, a->buf + *pos, sizeof(dv));
      fprintf(fp, " %lf\n", dv);
      break;
    case AST_IDENT:
      fprintf(fp, " %.*s\n", * (int *) (a->buf + *pos),
              * (char **) (a->buf + *pos + sizeof(uint32_t)));
      break;
    case AST_STRING:
      fprintf(fp, " \"%.*s\"\n", * (int *) (a->buf + *pos),
              * (char **) (a->buf + *pos + sizeof(uint32_t)));
      break;
    default:
      fprintf(fp, "\n");
  }
  *pos += def->fixed_len;
  *pos += sizeof(ast_skip_t) * def->num_skips;

  for (i = 0; i < def->num_subtrees; i++) {
    ast_dump_tree(fp, a, pos, depth + 1);
  }

  if (ast_node_defs[tag].num_skips) {
    /*
     * first skip always encodes end of the last children sequence.
     * so unless we care how the subtree sequences are grouped together
     * (and we currently don't) we can just read until the end of that skip.
     */
    size_t end = ast_get_skip(a, skips, AST_END_SKIP);

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

/* Dumps an AST to stdout. */
V7_PRIVATE void ast_dump(FILE *fp, struct ast *a, ast_off_t pos) {
  ast_dump_tree(fp, a, &pos, 0);
}
