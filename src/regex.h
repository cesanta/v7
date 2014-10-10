
#ifndef regex_h
#define regex_h

#define RE_MAX_SUB 32

struct RE_FLAGS{
	uint16_t unused:5;
	/* execution flag */
	uint16_t re_g:1;
	/* compiler flags */
	uint16_t re_i:1;
	uint16_t re_m:1;
	/* parser flag */
	uint16_t re:1;
};

struct re_tok{
	const char *start;	/* points to the beginning of the token */
	const char *end;	/* points to the end of the token */
};
/*
 *	Sub expression matches
 */
struct Resub{
	unsigned int subexpr_num;
	struct re_tok sub[RE_MAX_SUB];
};

struct Rerange{ Rune s; Rune e; };
/*
 *	character class, each pair of rune's defines a range
 */
struct Reclass{
	struct Rerange *end;
	struct Rerange spans[32];
};

/*
 * Parser Information
 */
struct Renode{
	uint8_t type;
	union{
		Rune c;			/* character */
		struct Reclass *cp;	/* class pointer */
		struct{
			struct Renode *x;
			union{
				struct Renode *y;
				uint8_t n;
				struct{
					uint8_t ng;	/* not greedy flag */
					uint16_t min;
					uint16_t max;
				};
			};
		};
	};
};

/*
 *	Machine instructions
 */
struct Reinst{
	uint8_t opcode;
	union{
		uint8_t n;
		Rune c;			/* character */
		struct Reclass *cp;	/* class pointer */
		struct{
			struct Reinst *x;
			union{
				struct{
					uint16_t min;
					uint16_t max;
				};
				struct Reinst *y;
			};
		};
	};
};

/*
 *	struct Reprogram definition
 */
struct Reprog{
	struct Reinst *start, *end;
	struct RE_FLAGS flags;
	unsigned int subexpr_num;
	struct Reclass charset[16];
};

/*
 *	struct Rethread definition
 */
struct Rethread{
	struct Reinst *pc;
	const char *start;
	struct Resub sub;
};

struct Reprog *re_compiler(const char *pattern, struct RE_FLAGS flags, const char **errorp);
uint8_t re_exec(struct Reprog *prog, const char *string, struct Resub *loot);
void re_free(struct Reprog *prog);

int re_replace(struct Resub *loot, const char *src, const char *rstr, char **dst);

#endif
