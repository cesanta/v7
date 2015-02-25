/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 *
 * This software is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively, you can license this software under a commercial
 * license, as set out in <http://cesanta.com/>.
 */

#ifndef SLRE_HEADER_INCLUDED
#define SLRE_HEADER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* Regex flags */
#define SLRE_FLAG_G 1     /* Global - match in the whole string */
#define SLRE_FLAG_I 2     /* Ignore case */
#define SLRE_FLAG_M 4     /* Multiline */
#define SLRE_FLAG_RE 8    /* flag RegExp/String */

/* Describes single capture */
struct slre_cap {
  const char *start; /* points to the beginning of the capture group */
  const char *end;   /* points to the end of the capture group */
};

/* Describes all captures */
#define SLRE_MAX_CAPS 32
struct slre_loot {
  int num_captures;
  struct slre_cap caps[SLRE_MAX_CAPS];
};

/* Opaque structure that holds compiled regular expression */
struct slre_prog;

/* Return codes for slre_compile() */
enum slre_error {
  SLRE_OK,
  SLRE_INVALID_DEC_DIGIT,
  SLRE_INVALID_HEX_DIGIT,
  SLRE_INVALID_ESC_CHAR,
  SLRE_UNTERM_ESC_SEQ,
  SLRE_SYNTAX_ERROR,
  SLRE_UNMATCH_LBR,
  SLRE_UNMATCH_RBR,
  SLRE_NUM_OVERFLOW,
  SLRE_INF_LOOP_M_EMP_STR,
  SLRE_TOO_MANY_CHARSETS,
  SLRE_INV_CHARSET_RANGE,
  SLRE_CHARSET_TOO_LARGE,
  SLRE_MALFORMED_CHARSET,
  SLRE_INVALID_BACK_REFERENCE,
  SLRE_TOO_MANY_CAPTURES,
  SLRE_INVALID_QUANTIFIER,
  SLRE_BAD_CHAR_AFTER_USD
};

int slre_compile(const char *regexp, size_t regexp_len, const char *flags,
                 size_t flags_len, struct slre_prog **, int is_regex);
int slre_exec(struct slre_prog *, const char *, size_t, struct slre_loot *);
void slre_free(struct slre_prog *prog);

int slre_match(const char *, size_t, const char *, size_t, const char *, size_t,
               struct slre_loot *);
int slre_replace(struct slre_loot *loot, const char *src, const char *replace,
                 struct slre_loot *dst);
int slre_get_flags(struct slre_prog *);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* SLRE_HEADER_INCLUDED */
