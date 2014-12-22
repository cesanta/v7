#ifndef SLRE_HEADER_INCLUDED
#define SLRE_HEADER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "utf.h"

/* Limitations */
#define RE_MAX_SUB 32
#define RE_MAX_RANGES 32
#define RE_MAX_SETS 16
#define RE_MAX_REP 0xFFFF
#define RE_MAX_THREADS 1000
#define V7_RE_MAX_REPL_SUB 255

/* Regex compilation flags */
#define RE_FLAG_G 1     /* Global - match in the whole string */
#define RE_FLAG_I 2     /* Ignore case */
#define RE_FLAG_M 4     /* Multiline */
/*#define RE_FLAG_RE 8*/

/* Describes single capture */
struct slre_tok {
  const char *start; /* points to the beginning of the token */
  const char *end;   /* points to the end of the token */
};

/* Sub expression matches */
struct Resub {
  int subexpr_num;
  struct slre_tok sub[RE_MAX_SUB];
};

struct Rerange {
  Rune s;
  Rune e;
};

/* character class, each pair of rune's defines a range */
struct Reclass {
  struct Rerange *end;
  struct Rerange spans[RE_MAX_RANGES];
};

/* Parser Information */
struct Renode {
  unsigned char type;
  union {
    Rune c;             /* character */
    struct Reclass *cp; /* class pointer */
    struct {
      struct Renode *x;
      union {
        struct Renode *y;
        unsigned char n;
        struct {
          unsigned char ng; /* not greedy flag */
          unsigned short min;
          unsigned short max;
        } rp;
      } y;
    } xy;
  } par;
};

/* Machine instructions */
struct Reinst {
  unsigned char opcode;
  union {
    unsigned char n;
    Rune c;             /* character */
    struct Reclass *cp; /* class pointer */
    struct {
      struct Reinst *x;
      union {
        struct {
          unsigned short min;
          unsigned short max;
        } rp;
        struct Reinst *y;
      } y;
    } xy;
  } par;
};

struct Reprog {
  struct Reinst *start, *end;
  unsigned int subexpr_num;
  struct Reclass charset[RE_MAX_SETS];
};

struct Rethread {
  struct Reinst *pc;
  const char *start;
  struct Resub sub;
};

int re_exec(struct Reprog *prog, unsigned char flags, const char *string,
            struct Resub *loot);
void re_free(struct Reprog *prog);
int re_rplc(struct Resub *loot, const char *src, const char *rstr,
            struct Resub *dst);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* SLRE_HEADER_INCLUDED */
