/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_APARSER_H_INCLUDED
#define V7_APARSER_H_INCLUDED

#include "internal.h"

#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

struct v7_pstate {
  const char *file_name;
  const char *source_code;
  const char *pc; /* Current parsing position */
  int line_no;    /* Line number */
  int prev_line_no; /* Line number of previous token */
  int inhibit_in;   /* True while `in` expressions are inhibited */
};

enum v7_err { V7_OK, V7_ERROR, V7_SYNTAX_ERROR };
V7_PRIVATE enum v7_err aparse(struct ast *, const char*, int);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */

#endif  /* V7_APARSER_H_INCLUDED */
