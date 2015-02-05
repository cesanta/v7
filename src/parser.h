/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_PARSER_H_INCLUDED
#define V7_PARSER_H_INCLUDED

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
  int in_function;  /* True if in a function */
  int in_loop;      /* True if in a loop */
  int in_switch;    /* True if in a switch block */
  int in_strict;    /* True if in strict mode */
};

V7_PRIVATE enum v7_err parse(struct v7 *, struct ast *, const char*, int);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */

#endif  /* V7_PARSER_H_INCLUDED */
