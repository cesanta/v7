// Copyright (c) 2013-2014 Cesanta Software Limited
// All rights reserved
//
// This software is dual-licensed: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation. For the terms of this
// license, see <http://www.gnu.org/licenses/>.
//
// You are free to use this software under the terms of the GNU General
// Public License, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// Alternatively, you can license this software under a commercial
// license, as set out in <http://cesanta.com/products.html>.

#ifndef regex_h
#define regex_h

#define RE_MAX_SUB 32

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
	struct v7_val_flags flags;
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

struct Reprog *re_compiler(const char *pattern, struct v7_val_flags flags, const char **errorp);
uint8_t re_exec(struct Reprog *prog, const char *string, struct Resub *loot);
void re_free(struct Reprog *prog);

int re_replace(struct Resub *loot, const char *src, const char *rstr, char **dst);

#endif
