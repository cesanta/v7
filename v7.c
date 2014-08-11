// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
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

#include "v7.h"
#include "slre.h"

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#define __unused
#endif

// MSVC6 doesn't have standard C math constants defined
#ifndef M_PI
#define M_E         2.71828182845904523536028747135266250
#define M_LOG2E     1.44269504088896340735992468100189214
#define M_LOG10E    0.434294481903251827651128918916605082
#define M_LN2       0.693147180559945309417232121458176568
#define M_LN10      2.30258509299404568401799145468436421
#define M_PI        3.14159265358979323846264338327950288
#define M_SQRT2     1.41421356237309504880168872420969808
#define M_SQRT1_2   0.707106781186547524400844362104849039
#define NAN         atof("NAN")
#define INFINITY    atof("INFINITY")  // TODO: fix this
#endif

//#define V7_CACHE_OBJS
#define MAX_STRING_LITERAL_LENGTH 2000
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define CHECK(cond, code) do { if (!(cond)) return (code); } while (0)
#define TRY(call) do { enum v7_err e = call; CHECK(e == V7_OK, e); } while (0)
#define TRACE_OBJ(O) do { char x[4000]; printf("==> %s [%s]\n", __func__, \
  O == NULL ? "@" : v7_to_string(O, x, sizeof(x))); } while (0)
#define MKOBJ(_proto) { 0, (_proto), 0, 0, {0}, V7_TYPE_OBJ, 0, 0 }

#define SET_RO_PROP_V(obj, name, val) \
  do { \
    static struct v7_val key = MKOBJ(&s_prototypes[V7_CLASS_STRING]); \
    static struct v7_prop prop = { NULL, &key, &val, 0 }; \
    v7_init_str(&key, (char *) (name), strlen(name), 0); \
    prop.next = obj.props; \
    obj.props = &prop; \
  } while (0)

#define SET_RO_PROP2(_o, _name, _t, _proto, _attr, _initializer, _fl) \
  do { \
    static struct v7_val _val = MKOBJ(_proto); \
    _val.v._attr = (_initializer); \
    _val.type = (_t); \
    _val.flags = (_fl); \
    SET_RO_PROP_V(_o, _name, _val); \
  } while (0)

#define SET_RO_PROP(obj, name, _t, attr, _v) \
    SET_RO_PROP2(obj, name, _t, &s_prototypes[V7_CLASS_OBJECT], attr, _v, 0)

#define SET_PROP_FUNC(_obj, _name, _func) \
    SET_RO_PROP2(_obj, _name, V7_TYPE_NULL, 0, prop_func, _func, V7_PROP_FUNC)

#define SET_METHOD(_obj, _name, _func) \
  do {  \
    static struct v7_val _val = MKOBJ(&s_prototypes[V7_CLASS_STRING]); \
    v7_set_class(&_val, V7_CLASS_FUNCTION); \
    _val.v.c_func = (_func); \
    SET_RO_PROP_V(_obj, _name, _val); \
  } while (0)

enum {
  OP_INVALID,

  // Relational ops
  OP_GREATER_THEN,    //  >
  OP_LESS_THEN,       //  <
  OP_GREATER_EQUAL,   //  >=
  OP_LESS_EQUAL,      //  <=

  // Equality ops
  OP_EQUAL,           //  ==
  OP_NOT_EQUAL,       //  !=
  OP_EQUAL_EQUAL,     //  ===
  OP_NOT_EQUAL_EQUAL, //  !==

  // Assignment ops
  OP_ASSIGN,          //  =
  OP_PLUS_ASSIGN,     //  +=
  OP_MINUS_ASSIGN,    //  -=
  OP_MUL_ASSIGN,      //  *=
  OP_DIV_ASSIGN,      //  /=
  OP_REM_ASSIGN,      //  %=
  OP_AND_ASSIGN,      //  &=
  OP_XOR_ASSIGN,      //  ^=
  OP_OR_ASSIGN,       //  |=
  OP_RSHIFT_ASSIGN,   //  >>=
  OP_LSHIFT_ASSIGN,   //  <<=
  OP_RRSHIFT_ASSIGN,  //  >>>=

  NUM_OPS
};

// Forward declarations
static enum v7_err parse_expression(struct v7 *);
static enum v7_err parse_statement(struct v7 *, int *is_return);
static int cmp(const struct v7_val *a, const struct v7_val *b);
static enum v7_err do_exec(struct v7 *v7, const char *, int);
static const int s_op_lengths[NUM_OPS] = {
  -1,
  1, 1, 2, 2,
  2, 2, 3, 3,
  1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4
};

static struct v7_val s_constructors[V7_NUM_CLASSES];
static struct v7_val s_prototypes[V7_NUM_CLASSES];
static struct v7_val s_math = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
static struct v7_val s_global = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
static struct v7_val s_json = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
static void inc_ref_count(struct v7_val *v) {
  assert(v != NULL);
  assert(v->ref_count >= 0);
  assert(!(v->flags & V7_VAL_DEALLOCATED));
  v->ref_count++;
}

static char *v7_strdup(const char *ptr, unsigned long len) {
  char *p = (char *) malloc(len + 1);
  if (p == NULL) return NULL;
  memcpy(p, ptr, len);
  p[len] = '\0';
  return p;
}

static int instanceof(const struct v7_val *obj, const struct v7_val *ctor) {
  if (obj->type == V7_TYPE_OBJ && ctor != NULL) {
    while (obj->ctor != NULL) {
      if (obj->ctor == ctor) return 1;
      obj = obj->ctor;
      if (obj->ctor == obj) break;  // Break on circular reference
    }
  }
  return 0;
}

int v7_is_class(const struct v7_val *obj, enum v7_class cls) {
  return instanceof(obj, &s_constructors[cls]);
}

static struct v7_val *common_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : cfa->result;
  obj->type = V7_TYPE_OBJ;
  return obj;
}

void v7_init_str(struct v7_val *v, char *p, unsigned long len, int own) {
  v->type = V7_TYPE_STR;
  v->proto = &s_prototypes[V7_CLASS_STRING];
  v->v.str.buf = p;
  v->v.str.len = len;
  v->flags &= ~V7_STR_ALLOCATED;
  if (own) {
    if (len < sizeof(v->v.str.loc) - 1) {
      v->v.str.buf = v->v.str.loc;
      memcpy(v->v.str.loc, p, len);
      v->v.str.loc[len] = '\0';
    } else {
      v->v.str.buf = v7_strdup(p, len);
      v->flags |= V7_STR_ALLOCATED;
    }
  }
}

void v7_init_num(struct v7_val *v, double num) {
  v->type = V7_TYPE_NUM;
  v->proto = &s_prototypes[V7_CLASS_NUMBER];
  v->v.num = num;
}

void v7_init_bool(struct v7_val *v, int is_true) {
  v->type = V7_TYPE_BOOL;
  v->proto = &s_prototypes[V7_CLASS_BOOLEAN];
  v->v.num = is_true ? 1.0 : 0.0;
}

void v7_init_func(struct v7_val *v, v7_c_func_t func) {
  v7_set_class(v, V7_CLASS_FUNCTION);
  v->v.c_func = func;
}
#ifndef V7_DISABLE_CRYPTO

//////////////////////////////// START OF MD5 THIRD PARTY CODE
/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 */

#define	MD5_BLOCK_LENGTH		64
#define	MD5_DIGEST_LENGTH		16

typedef struct MD5Context {
	uint32_t state[4];			/* state */
	uint64_t count;			/* number of bits, mod 2^64 */
	uint8_t buffer[MD5_BLOCK_LENGTH];	/* input buffer */
} MD5_CTX;

#define PUT_64BIT_LE(cp, value) do {					\
(cp)[7] = (value) >> 56;					\
(cp)[6] = (value) >> 48;					\
(cp)[5] = (value) >> 40;					\
(cp)[4] = (value) >> 32;					\
(cp)[3] = (value) >> 24;					\
(cp)[2] = (value) >> 16;					\
(cp)[1] = (value) >> 8;						\
(cp)[0] = (value); } while (0)

#define PUT_32BIT_LE(cp, value) do {					\
(cp)[3] = (value) >> 24;					\
(cp)[2] = (value) >> 16;					\
(cp)[1] = (value) >> 8;						\
(cp)[0] = (value); } while (0)

static uint8_t PADDING[MD5_BLOCK_LENGTH] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void MD5Init(MD5_CTX *ctx) {
	ctx->count = 0;
	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xefcdab89;
	ctx->state[2] = 0x98badcfe;
	ctx->state[3] = 0x10325476;
}

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

void MD5Transform(u_int32_t state[4], const u_int8_t block[MD5_BLOCK_LENGTH]) {
	uint32_t a, b, c, d, in[MD5_BLOCK_LENGTH / 4];

#if BYTE_ORDER == LITTLE_ENDIAN
	bcopy(block, in, sizeof(in));
#else
	for (a = 0; a < MD5_BLOCK_LENGTH / 4; a++) {
		in[a] = (u_int32_t)(
                        (u_int32_t)(block[a * 4 + 0]) |
                        (u_int32_t)(block[a * 4 + 1]) <<  8 |
                        (u_int32_t)(block[a * 4 + 2]) << 16 |
                        (u_int32_t)(block[a * 4 + 3]) << 24);
	}
#endif

	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];

	MD5STEP(F1, a, b, c, d, in[ 0] + 0xd76aa478,  7);
	MD5STEP(F1, d, a, b, c, in[ 1] + 0xe8c7b756, 12);
	MD5STEP(F1, c, d, a, b, in[ 2] + 0x242070db, 17);
	MD5STEP(F1, b, c, d, a, in[ 3] + 0xc1bdceee, 22);
	MD5STEP(F1, a, b, c, d, in[ 4] + 0xf57c0faf,  7);
	MD5STEP(F1, d, a, b, c, in[ 5] + 0x4787c62a, 12);
	MD5STEP(F1, c, d, a, b, in[ 6] + 0xa8304613, 17);
	MD5STEP(F1, b, c, d, a, in[ 7] + 0xfd469501, 22);
	MD5STEP(F1, a, b, c, d, in[ 8] + 0x698098d8,  7);
	MD5STEP(F1, d, a, b, c, in[ 9] + 0x8b44f7af, 12);
	MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
	MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
	MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122,  7);
	MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
	MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
	MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

	MD5STEP(F2, a, b, c, d, in[ 1] + 0xf61e2562,  5);
	MD5STEP(F2, d, a, b, c, in[ 6] + 0xc040b340,  9);
	MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
	MD5STEP(F2, b, c, d, a, in[ 0] + 0xe9b6c7aa, 20);
	MD5STEP(F2, a, b, c, d, in[ 5] + 0xd62f105d,  5);
	MD5STEP(F2, d, a, b, c, in[10] + 0x02441453,  9);
	MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
	MD5STEP(F2, b, c, d, a, in[ 4] + 0xe7d3fbc8, 20);
	MD5STEP(F2, a, b, c, d, in[ 9] + 0x21e1cde6,  5);
	MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6,  9);
	MD5STEP(F2, c, d, a, b, in[ 3] + 0xf4d50d87, 14);
	MD5STEP(F2, b, c, d, a, in[ 8] + 0x455a14ed, 20);
	MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905,  5);
	MD5STEP(F2, d, a, b, c, in[ 2] + 0xfcefa3f8,  9);
	MD5STEP(F2, c, d, a, b, in[ 7] + 0x676f02d9, 14);
	MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

	MD5STEP(F3, a, b, c, d, in[ 5] + 0xfffa3942,  4);
	MD5STEP(F3, d, a, b, c, in[ 8] + 0x8771f681, 11);
	MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
	MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
	MD5STEP(F3, a, b, c, d, in[ 1] + 0xa4beea44,  4);
	MD5STEP(F3, d, a, b, c, in[ 4] + 0x4bdecfa9, 11);
	MD5STEP(F3, c, d, a, b, in[ 7] + 0xf6bb4b60, 16);
	MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
	MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6,  4);
	MD5STEP(F3, d, a, b, c, in[ 0] + 0xeaa127fa, 11);
	MD5STEP(F3, c, d, a, b, in[ 3] + 0xd4ef3085, 16);
	MD5STEP(F3, b, c, d, a, in[ 6] + 0x04881d05, 23);
	MD5STEP(F3, a, b, c, d, in[ 9] + 0xd9d4d039,  4);
	MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
	MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	MD5STEP(F3, b, c, d, a, in[2 ] + 0xc4ac5665, 23);

	MD5STEP(F4, a, b, c, d, in[ 0] + 0xf4292244,  6);
	MD5STEP(F4, d, a, b, c, in[7 ] + 0x432aff97, 10);
	MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
	MD5STEP(F4, b, c, d, a, in[5 ] + 0xfc93a039, 21);
	MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3,  6);
	MD5STEP(F4, d, a, b, c, in[3 ] + 0x8f0ccc92, 10);
	MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
	MD5STEP(F4, b, c, d, a, in[1 ] + 0x85845dd1, 21);
	MD5STEP(F4, a, b, c, d, in[8 ] + 0x6fa87e4f,  6);
	MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	MD5STEP(F4, c, d, a, b, in[6 ] + 0xa3014314, 15);
	MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
	MD5STEP(F4, a, b, c, d, in[4 ] + 0xf7537e82,  6);
	MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
	MD5STEP(F4, c, d, a, b, in[2 ] + 0x2ad7d2bb, 15);
	MD5STEP(F4, b, c, d, a, in[9 ] + 0xeb86d391, 21);

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

void MD5Update(MD5_CTX *ctx, const unsigned char *input, size_t len) {
	size_t have, need;

	/* Check how many bytes we already have and how many more we need. */
	have = (size_t)((ctx->count >> 3) & (MD5_BLOCK_LENGTH - 1));
	need = MD5_BLOCK_LENGTH - have;

	/* Update bitcount */
	ctx->count += (u_int64_t)len << 3;

	if (len >= need) {
		if (have != 0) {
			bcopy(input, ctx->buffer + have, need);
			MD5Transform(ctx->state, ctx->buffer);
			input += need;
			len -= need;
			have = 0;
		}

		/* Process data in MD5_BLOCK_LENGTH-byte chunks. */
		while (len >= MD5_BLOCK_LENGTH) {
			MD5Transform(ctx->state, input);
			input += MD5_BLOCK_LENGTH;
			len -= MD5_BLOCK_LENGTH;
		}
	}

	/* Handle any remaining bytes of data. */
	if (len != 0)
		bcopy(input, ctx->buffer + have, len);
}

void MD5Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5_CTX *ctx) {
	uint8_t count[8];
	size_t padlen;
	int i;

	/* Convert count to 8 bytes in little endian order. */
	PUT_64BIT_LE(count, ctx->count);

	/* Pad out to 56 mod 64. */
	padlen = MD5_BLOCK_LENGTH -
  ((ctx->count >> 3) & (MD5_BLOCK_LENGTH - 1));
	if (padlen < 1 + 8)
		padlen += MD5_BLOCK_LENGTH;
	MD5Update(ctx, PADDING, padlen - 8);		/* padlen - 8 <= 64 */
	MD5Update(ctx, count, 8);

	if (digest != NULL) {
		for (i = 0; i < 4; i++)
			PUT_32BIT_LE(digest + i * 4, ctx->state[i]);
	}
	memset(ctx, 0, sizeof(*ctx));	/* in case it's sensitive */
}
/////////////////////////////////// END OF MD5 THIRD PARTY CODE

static struct v7_val s_crypto = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);

static void v7_md5(const struct v7_val *v, char *buf) {
  MD5_CTX ctx;
  MD5Init(&ctx);
  MD5Update(&ctx, (const unsigned char *) v->v.str.buf, v->v.str.len);
  MD5Final((unsigned char *) buf, &ctx);
}

static void bin2str(char *to, const unsigned char *p, size_t len) {
  static const char *hex = "0123456789abcdef";

  for (; len--; p++) {
    *to++ = hex[p[0] >> 4];
    *to++ = hex[p[0] & 0x0f];
  }
  *to = '\0';
}

static void Crypto_md5(struct v7_c_func_arg *cfa) {
  v7_init_str(cfa->result, NULL, 0, 0);
  if (cfa->num_args == 1 && cfa->args[0]->type == V7_TYPE_STR) {
    cfa->result->v.str.len = 16;
    cfa->result->v.str.buf = (char *) calloc(1, cfa->result->v.str.len + 1);
    v7_md5(cfa->args[0], cfa->result->v.str.buf);
  }
}

static void Crypto_md5_hex(struct v7_c_func_arg *cfa) {
  v7_init_str(cfa->result, NULL, 0, 0);
  if (cfa->num_args == 1 && cfa->args[0]->type == V7_TYPE_STR) {
    char hash[16];
    v7_md5(cfa->args[0], hash);
    cfa->result->v.str.len = 32;
    cfa->result->v.str.buf = (char *) calloc(1, cfa->result->v.str.len + 1);
    bin2str(cfa->result->v.str.buf, (unsigned char *) hash, sizeof(hash));
  }
}
#endif  // V7_DISABLE_CRYPTO

static void Array_ctor(struct v7_c_func_arg *cfa)    { common_ctor(cfa); }
static void Boolean_ctor(struct v7_c_func_arg *cfa)  { common_ctor(cfa); }
static void Date_ctor(struct v7_c_func_arg *cfa)     { common_ctor(cfa); }
static void Error_ctor(struct v7_c_func_arg *cfa)    { common_ctor(cfa); }
static void Function_ctor(struct v7_c_func_arg *cfa) { common_ctor(cfa); }
static void Number_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = common_ctor(cfa);
  obj->type = cfa->called_as_constructor ? V7_TYPE_OBJ : V7_TYPE_NUM;
  obj->v.num = cfa->num_args > 0 ? cfa->args[0]->v.num : 0.0;
}
static void Object_ctor(struct v7_c_func_arg *cfa)    { common_ctor(cfa); }
static void Regex_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = common_ctor(cfa);
  v7_set_class(obj, V7_CLASS_REGEXP);
}
static void String_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = common_ctor(cfa), *arg = cfa->args[0];
  if (cfa->num_args == 1 && arg->type == V7_TYPE_STR) {
    v7_init_str(obj, arg->v.str.buf, arg->v.str.len, 1);
  } else {
    v7_init_str(obj, NULL, 0, 0);
  }
}

static void Obj_toString(struct v7_c_func_arg *cfa) {
  char buf[4000];
  v7_to_string(cfa->this_obj, buf, sizeof(buf));
  v7_init_str(cfa->result, buf, strlen(buf), 1);
}

static void Obj_keys(struct v7_c_func_arg *cfa) {
  struct v7_prop *p;
  v7_set_class(cfa->result, V7_CLASS_ARRAY);
  for (p = cfa->this_obj->props; p != NULL; p = p->next) {
    v7_append(cfa->v7, cfa->result, p->key);
  }
}

static void Math_random(struct v7_c_func_arg *cfa) {
  srand((unsigned long) cfa->result);   // TODO: make better randomness
  v7_init_num(cfa->result, (double) rand() / RAND_MAX);
}

static void Math_sin(struct v7_c_func_arg *cfa) {
  v7_init_num(cfa->result, cfa->num_args == 1 ? sin(cfa->args[0]->v.num) : 0.0);
}

static void Math_sqrt(struct v7_c_func_arg *cfa) {
  v7_init_num(cfa->result, cfa->num_args == 1 ? sqrt(cfa->args[0]->v.num) : 0.0);
}

static void Math_tan(struct v7_c_func_arg *cfa) {
  v7_init_num(cfa->result, cfa->num_args == 1 ? tan(cfa->args[0]->v.num) : 0.0);
}

static void Math_pow(struct v7_c_func_arg *cfa) {
  v7_init_num(cfa->result, cfa->num_args == 2 ?
    pow(cfa->args[0]->v.num, cfa->args[1]->v.num) : 0.0);
}

static void Str_length(struct v7_val *this_obj, struct v7_val *result) {
  v7_init_num(result, this_obj->v.str.len);
}

static void Str_charCodeAt(struct v7_c_func_arg *cfa) {
  double idx = cfa->num_args > 0 && cfa->args[0]->type == V7_TYPE_NUM ?
    cfa->args[0]->v.num : NAN;
  const struct v7_string *str = &cfa->this_obj->v.str;

  v7_init_num(cfa->result, NAN);
  if (!isnan(idx) && cfa->this_obj->type == V7_TYPE_STR && fabs(idx) < str->len) {
    cfa->result->v.num = ((unsigned char *) str->buf)[(int) idx];
  }
}

static void Str_charAt(struct v7_c_func_arg *cfa) {
  double idx = cfa->num_args > 0 && cfa->args[0]->type == V7_TYPE_NUM ?
     cfa->args[0]->v.num : NAN;
  const struct v7_string *str = &cfa->this_obj->v.str;

  if (!isnan(idx) && cfa->this_obj->type == V7_TYPE_STR &&
      fabs(idx) < str->len) {
    v7_init_str(cfa->result, &str->buf[(int) idx], 1, 1);
  }
}

static void Str_match(struct v7_c_func_arg *cfa) {
  struct slre_cap caps[100];
  const struct v7_string *s = &cfa->this_obj->v.str;
  int i, n;

  cfa->result->type = V7_TYPE_NULL;
  memset(caps, 0, sizeof(caps));

  if (cfa->num_args == 1 &&
      v7_is_class(cfa->args[0], V7_CLASS_REGEXP) &&
      (n = slre_match(cfa->args[0]->v.regex, s->buf, s->len,
                      caps, ARRAY_SIZE(caps) - 1, 0)) > 0) {
    v7_set_class(cfa->result, V7_CLASS_ARRAY);
    v7_append(cfa->v7, cfa->result,
              v7_mkv(cfa->v7, V7_TYPE_STR, s->buf, (long) n, 1));
    for (i = 0; i < (int) ARRAY_SIZE(caps); i++) {
      if (caps[i].len == 0) break;
      v7_append(cfa->v7, cfa->result,
                v7_mkv(cfa->v7, V7_TYPE_STR, caps[i].ptr, (long) caps[i].len, 1));
    }
  }
}

// Implementation of memmem()
static const char *memstr(const char *a, size_t al, const char *b, size_t bl) {
  const char *end;
  if (al == 0 || bl == 0 || al < bl) return NULL;
  for (end = a + (al - bl); a < end; a++) if (!memcmp(a, b, bl)) return a;
  return NULL;
}

static void Str_split(struct v7_c_func_arg *cfa) {
  const struct v7_string *s = &cfa->this_obj->v.str;
  const char *p1, *p2, *e = s->buf + s->len;
  int limit = cfa->num_args == 2 && cfa->args[1]->type == V7_TYPE_NUM ?
     cfa->args[1]->v.num : -1;
  int num_elems = 0;

  v7_set_class(cfa->result, V7_CLASS_ARRAY);
  if (cfa->num_args == 0) {
    v7_append(cfa->v7, cfa->result,
              v7_mkv(cfa->v7, V7_TYPE_STR, s->buf, s->len, 1));
  } else if (cfa->args[0]->type == V7_TYPE_STR) {
    const struct v7_string *sep = &cfa->args[0]->v.str;
    if (sep->len == 0) {
      // Separator is empty. Split string by characters.
      for (p1 = s->buf; p1 < e; p1++) {
        if (limit >= 0 && limit <= num_elems) return;
        v7_append(cfa->v7, cfa->result, v7_mkv(cfa->v7, V7_TYPE_STR, p1, 1, 1));
        num_elems++;
      }
    } else {
      p1 = s->buf;
      while ((p2 = memstr(p1, e - p1, sep->buf, sep->len)) != NULL) {
        if (limit >= 0 && limit <= num_elems) return;
        v7_append(cfa->v7, cfa->result,
                  v7_mkv(cfa->v7, V7_TYPE_STR, p1, p2 - p1, 1));
        p1 = p2 + sep->len;
        num_elems++;
      }
      if (limit < 0 || limit > num_elems) {
        v7_append(cfa->v7, cfa->result,
                  v7_mkv(cfa->v7, V7_TYPE_STR, p1, e - p1, 1));
      }
    }
  } else if (instanceof(cfa->args[0], &s_constructors[V7_CLASS_REGEXP])) {
    char regex[200];
    struct slre_cap caps[20];
    int n = 0;

    snprintf(regex, sizeof(regex), "(%s)", cfa->args[0]->v.regex);
    p1 = s->buf;
    while ((n = slre_match(regex, p1, e - p1, caps, ARRAY_SIZE(caps), 0)) > 0) {
      if (limit >= 0 && limit <= num_elems) return;
      v7_append(cfa->v7, cfa->result,
                v7_mkv(cfa->v7, V7_TYPE_STR, p1, caps[0].ptr - p1, 1));
      p1 += n;
      num_elems++;
    }
    if (limit < 0 || limit > num_elems) {
      v7_append(cfa->v7, cfa->result,
                v7_mkv(cfa->v7, V7_TYPE_STR, p1, e - p1, 1));
    }
  }
}

static void Str_indexOf(struct v7_c_func_arg *cfa) {
  v7_init_num(cfa->result, -1.0);
  if (cfa->this_obj->type == V7_TYPE_STR &&
      cfa->num_args > 0 &&
      cfa->args[0]->type == V7_TYPE_STR) {
    int i = cfa->num_args > 1 && cfa->args[1]->type == V7_TYPE_NUM ?
      (int) cfa->args[1]->v.num : 0;
    const struct v7_string *a = &cfa->this_obj->v.str,
      *b = &cfa->args[0]->v.str;

    // Scan the string, advancing one byte at a time
    for (; i >= 0 && a->len >= b->len && i <= (int) (a->len - b->len); i++) {
      if (memcmp(a->buf + i, b->buf, b->len) == 0) {
        cfa->result->v.num = i;
        break;
      }
    }
  }
}

static void Str_substr(struct v7_c_func_arg *cfa) {
  long start = 0;

  v7_init_str(cfa->result, NULL, 0, 0);
  if (cfa->num_args > 0 && cfa->args[0]->type == V7_TYPE_NUM) {
    start = (long) cfa->args[0]->v.num;
  }
  if (start < 0) {
    start += (long) cfa->this_obj->v.str.len;
  }
  if (start >= 0 && start < (long) cfa->this_obj->v.str.len) {
    long n = cfa->this_obj->v.str.len - start;
    if (cfa->num_args > 1 && cfa->args[1]->type == V7_TYPE_NUM) {
      n = cfa->args[1]->v.num;
    }
    if (n > 0 && n <= ((long) cfa->this_obj->v.str.len - start)) {
      v7_init_str(cfa->result, cfa->this_obj->v.str.buf + start, n, 1);
    }
  }
}

static void Arr_length(struct v7_val *this_obj, struct v7_val *result) {
  struct v7_prop *p;
  v7_init_num(result, 0.0);
  for (p = this_obj->v.array; p != NULL; p = p->next) {
    result->v.num += 1.0;
  }
}

static void Arr_push(struct v7_c_func_arg *cfa) {
  int i;
  for (i = 0; i < cfa->num_args; i++) {
    v7_append(cfa->v7, cfa->this_obj, cfa->args[i]);
  }
}

static int cmp_prop(const void *pa, const void *pb) {
  const struct v7_prop *p1 = * (struct v7_prop **) pa;
  const struct v7_prop *p2 = * (struct v7_prop **) pb;
  return cmp(p2->val, p1->val);
}

static void Arr_sort(struct v7_c_func_arg *cfa) {
  int i = 0, length = 0;
  struct v7_val *v = cfa->this_obj;
  struct v7_prop *p, **arr;

  // TODO(lsm): do proper error checking
  for (p = v->v.array; p != NULL; p = p->next) {
    length++;
  }
  arr = (struct v7_prop **) malloc(length * sizeof(p));
  for (i = 0, p = v->v.array; p != NULL; p = p->next) {
    arr[i++] = p;
  }
  qsort(arr, length, sizeof(p), cmp_prop);
  v->v.array = NULL;
  for (i = 0; i < length; i++) {
    arr[i]->next = v->v.array;
    v->v.array = arr[i];
  }
  free(arr);
}

static void Num_toFixed(struct v7_c_func_arg *cfa) {
  int len, digits = cfa->num_args > 0 ? (int) cfa->args[0]->v.num : 0;
  char fmt[10], buf[100];

  snprintf(fmt, sizeof(fmt), "%%.%dlf", digits);
  len = snprintf(buf, sizeof(buf), fmt, cfa->this_obj->v.num);
  v7_init_str(cfa->result, buf, len, 1);
}

static void Json_stringify(struct v7_c_func_arg *cfa) {
  v7_init_str(cfa->result, NULL, 0, 0);
  // TODO(lsm): implement JSON.stringify
}

static void Std_print(struct v7_c_func_arg *cfa) {
  char buf[4000];
  int i;
  for (i = 0; i < cfa->num_args; i++) {
    printf("%s", v7_to_string(cfa->args[i], buf, sizeof(buf)));
  }
  putchar('\n');
}

static void Std_load(struct v7_c_func_arg *cfa) {
  int i;

  v7_init_bool(cfa->result, 1);
  for (i = 0; i < cfa->num_args; i++) {
    if (cfa->args[i]->type != V7_TYPE_STR ||
        v7_exec_file(cfa->v7, cfa->args[i]->v.str.buf) != V7_OK) {
      cfa->result->v.num = 0.0;
      break;
    }
  }
}

static void Std_exit(struct v7_c_func_arg *cfa) {
  int exit_code = cfa->num_args > 0 ? (int) cfa->args[0]->v.num : EXIT_SUCCESS;
  exit(exit_code);
}

static void base64_encode(const unsigned char *src, int src_len, char *dst) {
  static const char *b64 =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int i, j, a, b, c;

  for (i = j = 0; i < src_len; i += 3) {
    a = src[i];
    b = i + 1 >= src_len ? 0 : src[i + 1];
    c = i + 2 >= src_len ? 0 : src[i + 2];

    dst[j++] = b64[a >> 2];
    dst[j++] = b64[((a & 3) << 4) | (b >> 4)];
    if (i + 1 < src_len) {
      dst[j++] = b64[(b & 15) << 2 | (c >> 6)];
    }
    if (i + 2 < src_len) {
      dst[j++] = b64[c & 63];
    }
  }
  while (j % 4 != 0) {
    dst[j++] = '=';
  }
  dst[j++] = '\0';
}

// Convert one byte of encoded base64 input stream to 6-bit chunk
static unsigned char from_b64(unsigned char ch) {
  // Inverse lookup map
  static const unsigned char tab[128] = {
    255, 255, 255, 255, 255, 255, 255, 255, //  0
    255, 255, 255, 255, 255, 255, 255, 255, //  8
    255, 255, 255, 255, 255, 255, 255, 255, //  16
    255, 255, 255, 255, 255, 255, 255, 255, //  24
    255, 255, 255, 255, 255, 255, 255, 255, //  32
    255, 255, 255,  62, 255, 255, 255,  63, //  40
     52,  53,  54,  55,  56,  57,  58,  59, //  48
     60,  61, 255, 255, 255, 200, 255, 255, //  56   '=' is 200, on index 61
    255,   0,   1,   2,   3,   4,   5,   6, //  64
      7,   8,   9,  10,  11,  12,  13,  14, //  72
     15,  16,  17,  18,  19,  20,  21,  22, //  80
     23,  24,  25, 255, 255, 255, 255, 255, //  88
    255,  26,  27,  28,  29,  30,  31,  32, //  96
     33,  34,  35,  36,  37,  38,  39,  40, //  104
     41,  42,  43,  44,  45,  46,  47,  48, //  112
     49,  50,  51, 255, 255, 255, 255, 255, //  120
  };
  return tab[ch & 127];
}

static void base64_decode(const unsigned char *s, int len, char *dst) {
  unsigned char a, b, c, d;
  while (len >= 4 &&
         (a = from_b64(s[0])) != 255 &&
         (b = from_b64(s[1])) != 255 &&
         (c = from_b64(s[2])) != 255 &&
         (d = from_b64(s[3])) != 255) {
    if (a == 200 || b == 200) break;  // '=' can't be there
    *dst++ = a << 2 | b >> 4;
    if (c == 200) break;
    *dst++ = b << 4 | c >> 2;
    if (d == 200) break;
    *dst++ = c << 6 | d;
    s += 4;
    len -=4;
  }
  *dst = 0;
}

static void Std_base64_decode(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0];

  v7_init_str(cfa->result, NULL, 0, 0);
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    cfa->result->v.str.len = v->v.str.len * 3 / 4 + 1;
    cfa->result->v.str.buf = (char *) malloc(cfa->result->v.str.len + 1);
    base64_decode((const unsigned char *) v->v.str.buf, v->v.str.len,
                  cfa->result->v.str.buf);
  }
}

static void Std_base64_encode(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0];

  v7_init_str(cfa->result, NULL, 0, 0);
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    cfa->result->v.str.len = v->v.str.len * 3 / 2 + 1;
    cfa->result->v.str.buf = (char *) malloc(cfa->result->v.str.len + 1);
    base64_encode((const unsigned char *) v->v.str.buf, v->v.str.len,
                  cfa->result->v.str.buf);
  }
}

static void Std_eval(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0];
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    int old_line_no = cfa->v7->line_no;
    cfa->v7->line_no = 1;
    do_exec(cfa->v7, v->v.str.buf, cfa->v7->sp);
    cfa->v7->line_no = old_line_no;
#if 0
    if (cfa->v7->sp > old_sp) {
      *cfa->result = *v7_top(cfa->v7)[-1];
      inc_ref_count(cfa->result);
    }
#endif
  }
}

void v7_set_class(struct v7_val *v, enum v7_class cls) {
  v->type = V7_TYPE_OBJ;
  v->proto = &s_prototypes[cls];
  v->ctor = &s_constructors[cls];
}

static void init_stdlib(void) {
  static v7_c_func_t ctors[V7_NUM_CLASSES] = {
    Array_ctor, Boolean_ctor, Date_ctor, Error_ctor, Function_ctor,
    Number_ctor, Object_ctor, Regex_ctor, String_ctor
  };
  int i;

  for (i = 0; i < V7_NUM_CLASSES; i++) {
    s_prototypes[i].type = s_constructors[i].type = V7_TYPE_OBJ;
    s_prototypes[i].ref_count = s_constructors[i].ref_count = 1;
    s_prototypes[i].proto = &s_prototypes[V7_CLASS_OBJECT];
    s_prototypes[i].ctor = &s_constructors[V7_CLASS_FUNCTION];
    s_constructors[i].proto = &s_prototypes[i];
    s_constructors[i].ctor = &s_constructors[V7_CLASS_FUNCTION];
    s_constructors[i].v.c_func = ctors[i];
  }

  SET_METHOD(s_prototypes[V7_CLASS_OBJECT], "toString", Obj_toString);
  SET_METHOD(s_prototypes[V7_CLASS_OBJECT], "keys", Obj_keys);

  SET_METHOD(s_math, "random", Math_random);
  SET_METHOD(s_math, "pow", Math_pow);
  SET_METHOD(s_math, "sin", Math_sin);
  SET_METHOD(s_math, "tan", Math_tan);
  SET_METHOD(s_math, "sqrt", Math_sqrt);

  SET_RO_PROP(s_prototypes[V7_CLASS_NUMBER], "MAX_VALUE",
              V7_TYPE_NUM, num, LONG_MAX);
  SET_RO_PROP(s_prototypes[V7_CLASS_NUMBER], "MIN_VALUE",
              V7_TYPE_NUM, num, LONG_MIN);
  SET_RO_PROP(s_prototypes[V7_CLASS_NUMBER], "NaN",
              V7_TYPE_NUM, num, NAN);
  SET_METHOD(s_prototypes[V7_CLASS_NUMBER], "toFixed", Num_toFixed);

  SET_METHOD(s_json, "stringify", Json_stringify);

#ifndef V7_DISABLE_CRYPTO
  SET_METHOD(s_crypto, "md5", Crypto_md5);
  SET_METHOD(s_crypto, "md5_hex", Crypto_md5_hex);
  SET_RO_PROP_V(s_global, "Crypto", s_crypto);
  s_crypto.ref_count = 1;
#endif

  SET_PROP_FUNC(s_prototypes[V7_CLASS_ARRAY], "length", Arr_length);
  SET_METHOD(s_prototypes[V7_CLASS_ARRAY], "push", Arr_push);
  SET_METHOD(s_prototypes[V7_CLASS_ARRAY], "sort", Arr_sort);

  SET_PROP_FUNC(s_prototypes[V7_CLASS_STRING], "length", Str_length);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charCodeAt", Str_charCodeAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charAt", Str_charAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "indexOf", Str_indexOf);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "substr", Str_substr);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "match", Str_match);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "split", Str_split);

  SET_RO_PROP(s_math, "E", V7_TYPE_NUM, num, M_E);
  SET_RO_PROP(s_math, "PI", V7_TYPE_NUM, num, M_PI);
  SET_RO_PROP(s_math, "LN2", V7_TYPE_NUM, num, M_LN2);
  SET_RO_PROP(s_math, "LN10", V7_TYPE_NUM, num, M_LN10);
  SET_RO_PROP(s_math, "LOG2E", V7_TYPE_NUM, num, M_LOG2E);
  SET_RO_PROP(s_math, "LOG10E", V7_TYPE_NUM, num, M_LOG10E);
  SET_RO_PROP(s_math, "SQRT1_2", V7_TYPE_NUM, num, M_SQRT1_2);
  SET_RO_PROP(s_math, "SQRT2", V7_TYPE_NUM, num, M_SQRT2);

  SET_METHOD(s_global, "print", Std_print);
  SET_METHOD(s_global, "exit", Std_exit);
  SET_METHOD(s_global, "load", Std_load);
  SET_METHOD(s_global, "base64_encode", Std_base64_encode);
  SET_METHOD(s_global, "base64_decode", Std_base64_decode);
  SET_METHOD(s_global, "eval", Std_eval);

  SET_RO_PROP_V(s_global, "Object", s_constructors[V7_CLASS_OBJECT]);
  SET_RO_PROP_V(s_global, "Number", s_constructors[V7_CLASS_NUMBER]);
  SET_RO_PROP_V(s_global, "String", s_constructors[V7_CLASS_STRING]);
  SET_RO_PROP_V(s_global, "Array", s_constructors[V7_CLASS_ARRAY]);
  SET_RO_PROP_V(s_global, "RegExp", s_constructors[V7_CLASS_REGEXP]);
  SET_RO_PROP_V(s_global, "Function", s_constructors[V7_CLASS_FUNCTION]);
  SET_RO_PROP_V(s_global, "Date", s_constructors[V7_CLASS_DATE]);
  SET_RO_PROP_V(s_global, "Error", s_constructors[V7_CLASS_ERROR]);

  SET_RO_PROP_V(s_global, "Math", s_math);
  SET_RO_PROP_V(s_global, "JSON", s_json);

  v7_set_class(&s_math, V7_CLASS_OBJECT);
  v7_set_class(&s_json, V7_CLASS_OBJECT);
  v7_set_class(&s_crypto, V7_CLASS_OBJECT);
  v7_set_class(&s_global, V7_CLASS_OBJECT);
  s_math.ref_count = s_json.ref_count = 1;
  s_global.ref_count = 1;
}

struct v7 *v7_create(void) {
  static int prototypes_initialized = 0;
  struct v7 *v7 = NULL;

  if (prototypes_initialized == 0) {
    prototypes_initialized++;
    init_stdlib();  // One-time initialization
  }

  if ((v7 = (struct v7 *) calloc(1, sizeof(*v7))) != NULL) {
    v7_set_class(&v7->root_scope, V7_CLASS_OBJECT);
    v7->root_scope.proto = &s_global;
    v7->root_scope.ref_count = 1;
  }

  return v7;
}

static void free_prop(struct v7 *v7, struct v7_prop *p) {
  if (p->key != NULL) v7_freeval(v7, p->key);
  v7_freeval(v7, p->val);
  p->val = p->key = NULL;
  if (p->flags & V7_VAL_ALLOCATED) {
#ifdef V7_CACHE_OBJS
    p->next = v7->free_props;
    v7->free_props = p;
  #else
    free(p);
#endif
  }
  p->flags = 0;
}

void v7_freeval(struct v7 *v7, struct v7_val *v) {
  assert(v->ref_count > 0);
  if (--v->ref_count > 0) return;

  if (v->type == V7_TYPE_OBJ) {
    struct v7_prop *p, *tmp;
    for (p = v->props; p != NULL; p = tmp) {
      tmp = p->next;
      free_prop(v7, p);
    }
    v->props = NULL;
  }

  if (v7_is_class(v, V7_CLASS_ARRAY)) {
    struct v7_prop *p, *tmp;
    for (p = v->v.array; p != NULL; p = tmp) {
      tmp = p->next;
      free_prop(v7, p);
    }
    v->v.array = NULL;
  } else if (v->type == V7_TYPE_STR && (v->flags & V7_STR_ALLOCATED)) {
    free(v->v.str.buf);
  } else if (v7_is_class(v, V7_CLASS_REGEXP) && (v->flags & V7_STR_ALLOCATED)) {
    free(v->v.regex);
  } else if (v7_is_class(v, V7_CLASS_FUNCTION)) {
    if ((v->flags & V7_STR_ALLOCATED) && (v->flags & V7_JS_FUNC)) {
      free(v->v.func.source_code);
      v7_freeval(v7, v->v.func.scope);
    }
    if (v->v.func.upper != NULL) v7_freeval(v7, v->v.func.upper);
  }

  if (v->flags & V7_VAL_ALLOCATED) {
    v->flags &= ~V7_VAL_ALLOCATED;
    v->flags |= ~V7_VAL_DEALLOCATED;
    memset(v, 0, sizeof(*v));
#ifdef V7_CACHE_OBJS
    v->next = v7->free_values;
    v7->free_values = v;
#else
    free(v);
#endif
  }
}

struct v7_val *v7_rootns(struct v7 *v7) {
  return &v7->root_scope;
}

static enum v7_err inc_stack(struct v7 *v7, int incr) {
  int i;

  CHECK(v7->sp + incr < (int) ARRAY_SIZE(v7->stack), V7_STACK_OVERFLOW);
  CHECK(v7->sp + incr >= 0, V7_STACK_UNDERFLOW);

  // Free values pushed on stack (like string literals and functions)
  for (i = 0; incr < 0 && i < -incr && i < v7->sp; i++) {
    v7_freeval(v7, v7->stack[v7->sp - (i + 1)]);
    v7->stack[v7->sp - (i + 1)] = NULL;
  }

  v7->sp += incr;
  return V7_OK;
}

enum v7_err v7_pop(struct v7 *v7, int incr) {
  CHECK(incr >= 0, V7_INTERNAL_ERROR);
  return inc_stack(v7, -incr);
}

static void free_values(struct v7 *v7) {
  struct v7_val *v;
  while (v7->free_values != NULL) {
    v = v7->free_values->next;
    free(v7->free_values);
    v7->free_values = v;
  }
}

static void free_props(struct v7 *v7) {
  struct v7_prop *p;
  while (v7->free_props != NULL) {
    p = v7->free_props->next;
    free(v7->free_props);
    v7->free_props = p;
  }
}

void v7_destroy(struct v7 **v7) {
  if (v7 == NULL || v7[0] == NULL) return;
  assert(v7[0]->sp >= 0);
  inc_stack(v7[0], -v7[0]->sp);
  v7[0]->root_scope.ref_count = 1;
  v7_freeval(v7[0], &v7[0]->root_scope);
  free_values(v7[0]);
  free_props(v7[0]);
  free(v7[0]);
  v7[0] = NULL;
}

struct v7_val **v7_top(struct v7 *v7) {
  return &v7->stack[v7->sp];
}

int v7_sp(struct v7 *v7) {
  return (int) (v7_top(v7) - v7->stack);
}

int v7_is_true(const struct v7_val *v) {
  return (v->type == V7_TYPE_BOOL && v->v.num != 0.0) ||
  (v->type == V7_TYPE_NUM && v->v.num != 0.0 && !isnan(v->v.num)) ||
  (v->type == V7_TYPE_STR && v->v.str.len > 0) ||
  (v->type == V7_TYPE_OBJ);
}

static void obj_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_prop *m, *head = v->props;
  int n = snprintf(buf, bsiz, "%s", "{");

  for (m = head; m != NULL && n < bsiz - 1; m = m->next) {
    if (m != head) n += snprintf(buf + n , bsiz - n, "%s", ", ");
    v7_to_string(m->key, buf + n, bsiz - n);
    n = (int) strlen(buf);
    n += snprintf(buf + n , bsiz - n, "%s", ": ");
    v7_to_string(m->val, buf + n, bsiz - n);
    n = (int) strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "}");
}

static void arr_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_prop *m, *head = v->v.array;
  int n = snprintf(buf, bsiz, "%s", "[");

  for (m = head; m != NULL && n < bsiz - 1; m = m->next) {
    if (m != head) n += snprintf(buf + n , bsiz - n, "%s", ", ");
    v7_to_string(m->val, buf + n, bsiz - n);
    n = (int) strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "]");
}

const char *v7_to_string(const struct v7_val *v, char *buf, int bsiz) {
  if (v->type == V7_TYPE_UNDEF) {
    snprintf(buf, bsiz, "%s", "undefined");
  } else if (v->type == V7_TYPE_NULL) {
    snprintf(buf, bsiz, "%s", "null");
  } else if (v->type == V7_TYPE_BOOL || v7_is_class(v, V7_CLASS_BOOLEAN)) {
    snprintf(buf, bsiz, "%s", v->v.num ? "true" : "false");
  } else if (v->type == V7_TYPE_NUM || v7_is_class(v, V7_CLASS_NUMBER)) {
    // TODO: check this on 32-bit arch
    if (v->v.num > ((unsigned long) 1 << 52) || ceil(v->v.num) != v->v.num) {
      snprintf(buf, bsiz, "%lg", v->v.num);
    } else {
      snprintf(buf, bsiz, "%lu", (unsigned long) v->v.num);
    }
  } else if (v->type == V7_TYPE_STR || v7_is_class(v, V7_CLASS_STRING)) {
    snprintf(buf, bsiz, "%.*s", (int) v->v.str.len, v->v.str.buf);
  } else if (v7_is_class(v, V7_CLASS_ARRAY)) {
    arr_to_string(v, buf, bsiz);
  } else if (v7_is_class(v, V7_CLASS_FUNCTION)) {
    if (v->flags & V7_JS_FUNC) {
      snprintf(buf, bsiz, "'function%s'", v->v.func.source_code);
    } else {
      snprintf(buf, bsiz, "'c_func_%p'", v->v.c_func);
    }
  } else if (v7_is_class(v, V7_CLASS_REGEXP)) {
    snprintf(buf, bsiz, "/%s/", v->v.regex);
  } else if (v->type == V7_TYPE_OBJ) {
    obj_to_string(v, buf, bsiz);
  } else {
    snprintf(buf, bsiz, "??");
  }

  buf[bsiz - 1] = '\0';
  return buf;
}

static struct v7_val *make_value(struct v7 *v7, enum v7_type type) {
  struct v7_val *v = NULL;

  if ((v = v7->free_values) != NULL) {
    v7->free_values = v->next;
  } else {
    v = (struct v7_val *) calloc(1, sizeof(*v));
  }

  if (v != NULL) {
    assert(v->ref_count == 0);
    v->flags = V7_VAL_ALLOCATED;
    v->type = type;
    switch (type) {
      case V7_TYPE_NUM: v->proto = &s_prototypes[V7_CLASS_NUMBER]; break;
      case V7_TYPE_STR: v->proto = &s_prototypes[V7_CLASS_STRING]; break;
      case V7_TYPE_BOOL: v->proto = &s_prototypes[V7_CLASS_BOOLEAN]; break;
      default: break;
    }
  }
  return v;
}

enum v7_err v7_push(struct v7 *v7, struct v7_val *v) {
  inc_ref_count(v);
  TRY(inc_stack(v7, 1));
  v7->stack[v7->sp - 1] = v;
  return V7_OK;
}

enum v7_err v7_make_and_push(struct v7 *v7, enum v7_type type) {
  struct v7_val *v = make_value(v7, type);
  CHECK(v != NULL, V7_OUT_OF_MEMORY);
  return v7_push(v7, v);
}

static enum v7_err arith(struct v7_val *a, struct v7_val *b,
                         struct v7_val *res, int op) {
  if (a->type == V7_TYPE_STR && a->type == V7_TYPE_STR && op == '+') {
    char *str = (char *) malloc(a->v.str.len + b->v.str.len + 1);
    CHECK(str != NULL, V7_OUT_OF_MEMORY);
    v7_init_str(res, str, a->v.str.len + b->v.str.len, 0);
    memcpy(str, a->v.str.buf, a->v.str.len);
    memcpy(str + a->v.str.len, b->v.str.buf, b->v.str.len);
    str[res->v.str.len] = '\0';
    return V7_OK;
  } else if (a->type == V7_TYPE_NUM && b->type == V7_TYPE_NUM) {
    v7_init_num(res, res->v.num);
    switch (op) {
      case '+': res->v.num = a->v.num + b->v.num; break;
      case '-': res->v.num = a->v.num - b->v.num; break;
      case '*': res->v.num = a->v.num * b->v.num; break;
      case '/': res->v.num = a->v.num / b->v.num; break;
      case '%': res->v.num = (unsigned long) a->v.num %
        (unsigned long) b->v.num; break;
      case '^': res->v.num = (unsigned long) a->v.num ^
        (unsigned long) b->v.num; break;
    }
    return V7_OK;
  } else {
    return V7_TYPE_MISMATCH;
  }
}

static enum v7_err do_arithmetic_op(struct v7 *v7, int op, int sp1, int sp2) {
  struct v7_val *res, tmp, *v1 = v7->stack[sp1 - 1], *v2 = v7->stack[sp2 - 1];

  CHECK(v7->sp >= 2, V7_STACK_UNDERFLOW);

  memset(&tmp, 0, sizeof(tmp));
  TRY(arith(v1, v2, &tmp, op));
  res = make_value(v7, tmp.type);
  CHECK(res != NULL, V7_OUT_OF_MEMORY);
  res->v = tmp.v;
  TRY(v7_push(v7, res));

  return V7_OK;
}

static struct v7_val str_to_val(const char *buf, size_t len) {
  struct v7_val v;
  memset(&v, 0, sizeof(v));
  v.type = V7_TYPE_STR;
  v.v.str.buf = (char *) buf;
  v.v.str.len = len;
  return v;
}

struct v7_val v7_str_to_val(const char *buf) {
  return str_to_val((char *) buf, strlen(buf));
}

static int cmp(const struct v7_val *a, const struct v7_val *b) {
  int res;
  if (a == NULL || b == NULL) return 1;
  if ((a->type == V7_TYPE_UNDEF || a->type == V7_TYPE_NULL) &&
      (b->type == V7_TYPE_UNDEF || b->type == V7_TYPE_NULL)) return 0;
  if (a->type != b->type) return 1;
  {
    double an = a->v.num, bn = b->v.num;
    const struct v7_string *as = &a->v.str, *bs = &b->v.str;

    switch (a->type) {
      case V7_TYPE_NUM:
        return (isinf(an) && isinf(bn)) ||
        (isnan(an) && isnan(bn)) ? 0 : an - bn;
      case V7_TYPE_BOOL:
        return an != bn;
      case V7_TYPE_STR:
        res = memcmp(as->buf, bs->buf, as->len < bs->len ? as->len : bs->len);
        return res != 0 ? res : (int) as->len - (int) bs->len;
        return as->len != bs->len || memcmp(as->buf, bs->buf, as->len) != 0;
      default:
        return a - b;
    }
  }
}

static struct v7_prop *v7_get(struct v7_val *obj, const struct v7_val *key,
                              int own_prop) {
  struct v7_prop *m;
  for (; obj != NULL; obj = obj->proto) {
    if (v7_is_class(obj, V7_CLASS_ARRAY) && key->type == V7_TYPE_NUM) {
      int i = (int) key->v.num;
      for (m = obj->v.array; m != NULL; m = m->next) {
        if (i-- == 0) return m;
      }
    } else if (obj->type == V7_TYPE_OBJ) {
      for (m = obj->props; m != NULL; m = m->next) {
        if (cmp(m->key, key) == 0) return m;
      }
    }
    if (own_prop) break;
    if (obj->proto == obj) break;
  }
  return NULL;
}

static struct v7_val *get2(struct v7_val *obj, const struct v7_val *key) {
  struct v7_prop *m = v7_get(obj, key, 0);
  return (m == NULL) ? NULL : m->val;
}

struct v7_val *v7_lookup(struct v7_val *obj, const char *key) {
  struct v7_val k = v7_str_to_val(key);
  return get2(obj, &k);
}

static struct v7_prop *mkprop(struct v7 *v7) {
  struct v7_prop *m;
  if ((m = v7->free_props) != NULL) {
    v7->free_props = m->next;
  } else {
    m = (struct v7_prop *) calloc(1, sizeof(*m));
  }
  if (m != NULL) m->flags = V7_PROP_ALLOCATED;
  return m;
}

static enum v7_err vinsert(struct v7 *v7, struct v7_prop **h,
                           struct v7_val *key, struct v7_val *val) {
  struct v7_prop *m = mkprop(v7);
  CHECK(m != NULL, V7_OUT_OF_MEMORY);

  inc_ref_count(key);
  inc_ref_count(val);
  m->key = key;
  m->val = val;
  m->next = *h;
  *h = m;

  return V7_OK;
}

static struct v7_val *find(struct v7 *v7, struct v7_val *key) {
  struct v7_val *v, *f;

  if (v7->no_exec) return NULL;

  // Search in function arguments first
  if (v7->curr_func != NULL &&
      (v = get2(v7->curr_func->v.func.args, key)) != NULL) return v;

  // Search for the name, traversing scopes up to the top level scope
  for (f = v7->curr_func; f != NULL; f = f->v.func.upper) {
    if ((v = get2(f->v.func.scope, key)) != NULL) return v;
  }
  return get2(&v7->root_scope, key);
}

static enum v7_err v7_set(struct v7 *v7, struct v7_val *obj, struct v7_val *k,
                          struct v7_val *v) {
  struct v7_prop *m = NULL;

  CHECK(obj != NULL && k != NULL && v != NULL, V7_INTERNAL_ERROR);
  CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_MISMATCH);

  // Find attribute inside object
  if ((m = v7_get(obj, k, 1)) != NULL) {
    v7_freeval(v7, m->val);
    inc_ref_count(v);
    m->val = v;
  } else {
    TRY(vinsert(v7, &obj->props, k, v));
  }

  return V7_OK;
}

struct v7_val *v7_mkvv(struct v7 *v7, enum v7_type t, va_list *ap) {
  struct v7_val *v = make_value(v7, t);

  // TODO: check for make_value() failure
  switch (t) {
    //case V7_C_FUNC: v->v.c_func = va_arg(*ap, v7_c_func_t); break;
    case V7_TYPE_NUM:
      v->v.num = va_arg(*ap, double);
      break;
    case V7_TYPE_STR: {
      char *buf = va_arg(*ap, char *);
      unsigned long len = va_arg(*ap, unsigned long);
      int own = va_arg(*ap, int);
      v7_init_str(v, buf, len, own);
    }
      break;
    default:
      break;
  }

  return v;
}

struct v7_val *v7_mkv(struct v7 *v7, enum v7_type t, ...) {
  struct v7_val *v = NULL;
  va_list ap;

  va_start(ap, t);
  v = v7_mkvv(v7, t, &ap);
  va_end(ap);

  return v;
}

enum v7_err v7_setv(struct v7 *v7, struct v7_val *obj,
                    enum v7_type key_type, enum v7_type val_type, ...) {
  struct v7_val *k = NULL, *v = NULL;
  va_list ap;

  va_start(ap, val_type);
  k = key_type == V7_TYPE_OBJ ?
    va_arg(ap, struct v7_val *) : v7_mkvv(v7, key_type, &ap);
  v = val_type == V7_TYPE_OBJ ?
    va_arg(ap, struct v7_val *) : v7_mkvv(v7, val_type, &ap);
  va_end(ap);

  // TODO: do not leak here
  CHECK(k != NULL && v != NULL, V7_OUT_OF_MEMORY);

  inc_ref_count(k);
  TRY(v7_set(v7, obj, k, v));
  v7_freeval(v7, k);

  return V7_OK;
}

void v7_copy(struct v7 *v7, struct v7_val *orig, struct v7_val *v) {
  struct v7_prop *p;

  switch (v->type) {
    case V7_TYPE_OBJ:
      for (p = orig->props; p != NULL; p = p->next) {
        v7_set(v7, v, p->key, p->val);
      }
      break;
    // TODO(lsm): add the rest of types
    default: abort(); break;
  }
}

static int is_alpha(int ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static int is_digit(int ch) {
  return ch >= '0' && ch <= '9';
}

static int is_alnum(int ch) {
  return is_digit(ch) || is_alpha(ch);
}

static int is_space(int ch) {
  return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static void skip_whitespaces_and_comments(struct v7 *v7) {
  const char *s = v7->pc, *p = NULL;
  while (s != p && *s != '\0' && (is_space(*s) || *s == '/')) {
    p = s;
    while (*s != '\0' && is_space(*s)) {
      if (*s == '\n') v7->line_no++;
      s++;
    }
    if (s[0] == '/' && s[1] == '/') {
      s += 2;
      while (s[0] != '\0' && s[0] != '\n') s++;
    }
    if (s[0] == '/' && s[1] == '*') {
      s += 2;
      while (s[0] != '\0' && !(s[-1] == '/' && s[-2] == '*')) {
        if (s[0] == '\n') v7->line_no++;
        s++;
      }
    }
  }
  v7->pc = s;
}

static enum v7_err match(struct v7 *v7, int ch) {
  CHECK(*v7->pc++ == ch, V7_SYNTAX_ERROR);
  skip_whitespaces_and_comments(v7);
  return V7_OK;
}

static int test_and_skip_char(struct v7 *v7, int ch) {
  if (*v7->pc == ch) {
    v7->pc++;
    skip_whitespaces_and_comments(v7);
    return 1;
  }
  return 0;
}

static int test_token(struct v7 *v7, const char *kw, unsigned long kwlen) {
  return kwlen == v7->tok_len && memcmp(v7->tok, kw, kwlen) == 0;
}

static enum v7_err parse_num(struct v7 *v7) {
  double value = 0;
  char *end;

  value = strtod(v7->pc, &end);
  // Handle case like 123.toString()
  if (end != NULL && (v7->pc < &end[-1]) && end[-1] == '.') end--;
  CHECK(value != 0 || end > v7->pc, V7_SYNTAX_ERROR);
  v7->pc = end;
  v7->tok_len = (unsigned long) (v7->pc - v7->tok);
  skip_whitespaces_and_comments(v7);

  if (!v7->no_exec) {
    TRY(v7_make_and_push(v7, V7_TYPE_NUM));
    v7_top(v7)[-1]->v.num = value;
  }

  return V7_OK;
}

static int is_valid_start_of_identifier(int ch) {
  return ch == '$' || ch == '_' || is_alpha(ch);
}

static int is_valid_identifier_char(int ch) {
  return ch == '$' || ch == '_' || is_alnum(ch);
}

static enum v7_err parse_identifier(struct v7 *v7) {
  CHECK(is_valid_start_of_identifier(v7->pc[0]), V7_SYNTAX_ERROR);
  v7->tok = v7->pc;
  v7->pc++;
  while (is_valid_identifier_char(v7->pc[0])) v7->pc++;
  v7->tok_len = (unsigned long) (v7->pc - v7->tok);
  skip_whitespaces_and_comments(v7);
  return V7_OK;
}

static int lookahead(struct v7 *v7, const char *str, int str_len) {
  int equal = 0;
  if (memcmp(v7->pc, str, str_len) == 0 &&
      !is_valid_identifier_char(v7->pc[str_len])) {
    equal++;
    v7->pc += str_len;
    skip_whitespaces_and_comments(v7);
  }
  return equal;
}

static enum v7_err parse_compound_statement(struct v7 *v7, int *has_return) {
  if (*v7->pc == '{') {
    int old_sp = v7->sp;
    TRY(match(v7, '{'));
    while (*v7->pc != '}') {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(parse_statement(v7, has_return));
      if (*has_return && !v7->no_exec) return V7_OK;
    }
    TRY(match(v7, '}'));
  } else {
    TRY(parse_statement(v7, has_return));
  }
  return V7_OK;
}

static struct v7_val *cur_scope(struct v7 *v7) {
  return v7->curr_func == NULL ? &v7->root_scope : v7->curr_func->v.func.scope;
}

static enum v7_err parse_function_definition(struct v7 *v7, struct v7_val **v,
                                             int num_params) {
  int i = 0, old_no_exec = v7->no_exec, old_sp = v7->sp, has_return = 0, ln = 0;
  unsigned long func_name_len = 0;
  const char *src = v7->pc, *func_name = NULL;
  struct v7_val args;

  if (*v7->pc != '(') {
    // function name is given, e.g. function foo() {}
    CHECK(v == NULL, V7_SYNTAX_ERROR);
    TRY(parse_identifier(v7));
    func_name = v7->tok;
    func_name_len = v7->tok_len;
    src = v7->pc;
  }

  // If 'v' (func to call) is NULL, that means we're just parsing function
  // definition to save it's body.
  v7->no_exec = v == NULL;
  ln = v7->line_no;  // Line number where function starts
  TRY(match(v7, '('));

  memset(&args, 0, sizeof(args));
  v7_set_class(&args, V7_CLASS_OBJECT);

  while (*v7->pc != ')') {
    TRY(parse_identifier(v7));
    if (!v7->no_exec) {
      // TODO(lsm): use v7_setv() here
      struct v7_val *key = v7_mkv(v7, V7_TYPE_STR, v7->tok, v7->tok_len, 1);
      struct v7_val *val = i < num_params ? v[i + 1] : make_value(v7, V7_TYPE_UNDEF);
      inc_ref_count(key);
      TRY(v7_set(v7, &args, key, val));
      v7_freeval(v7, key);
    }
    i++;
    if (!test_and_skip_char(v7, ',')) break;
  }
  TRY(match(v7, ')'));

  if (!v7->no_exec) {
    assert(v7->curr_func != NULL);
    v7->curr_func->v.func.args = &args;
  }

  TRY(parse_compound_statement(v7, &has_return));

  if (v7->no_exec) {
    struct v7_val *func;
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    func = v7_top(v7)[-1];
    func->flags |= V7_JS_FUNC;
    v7_set_class(func, V7_CLASS_FUNCTION);

    func->v.func.line_no = ln;
    func->v.func.source_code = v7_strdup(src, (unsigned long) (v7->pc - src));
    func->v.func.scope = v7_mkv(v7, V7_TYPE_OBJ);
    func->v.func.scope->ref_count = 1;
    func->v.func.upper = v7->curr_func;
    if (func->v.func.upper != NULL) {
      inc_ref_count(func->v.func.upper);
    }

    if (func_name != NULL) {
      TRY(v7_setv(v7, cur_scope(v7), V7_TYPE_STR, V7_TYPE_OBJ,
                  func_name, func_name_len, 1, func));
    }
  }

  if (!v7->no_exec) {
    // If function didn't have return statement, return UNDEF
    if (!has_return) {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
    }

    // Cleanup arguments
    v7->curr_func->v.func.args = NULL;
    args.ref_count = 1;
    v7_freeval(v7, &args);
  }

  v7->no_exec = old_no_exec;
  return V7_OK;
}

enum v7_err v7_call(struct v7 *v7, struct v7_val *this_obj, int num_args,
                    int called_as_ctor) {
  struct v7_val **top = v7_top(v7), **v = top - (num_args + 1), *f;

  if (v7->no_exec) return V7_OK;
  f = v[0];
  CHECK(v7->sp > num_args, V7_INTERNAL_ERROR);
  CHECK(v7_is_class(f, V7_CLASS_FUNCTION), V7_CALLED_NON_FUNCTION);

  // Push return value on stack
  v7_make_and_push(v7, V7_TYPE_UNDEF);


  // Stack looks as follows:
  //  v   --->  <called_function>     v[0]
  //            <argument_0>        ---+
  //            <argument_1>           |
  //            <argument_2>           |  <-- num_args
  //            ...                    |
  //            <argument_N>        ---+
  // top  --->  <return_value>
  if (f->flags & V7_JS_FUNC) {
    const char *old_pc = v7->pc;
    struct v7_val *old_curr_func = v7->curr_func;
    int old_line_no = v7->line_no;

    // Move control flow to the function body
    v7->pc = f->v.func.source_code;
    v7->line_no = f->v.func.line_no;
    v7->curr_func = f;

    // Execute function body
    TRY(parse_function_definition(v7, v, num_args));

    // Return control flow back
    v7->pc = old_pc;
    v7->line_no = old_line_no;
    v7->curr_func = old_curr_func;
    CHECK(v7_top(v7) >= top, V7_INTERNAL_ERROR);
  } else {
    struct v7_c_func_arg arg = {
      v7, this_obj, v7_top(v7)[-1], v + 1, num_args, called_as_ctor
    };
    f->v.c_func(&arg);
  }
  return V7_OK;
}

static enum v7_err parse_function_call(struct v7 *v7, struct v7_val *this_obj,
                                       int called_as_ctor) {
  struct v7_val **v = v7_top(v7) - 1;
  int num_args = 0;

  //TRACE_OBJ(v[0]);
  CHECK(v7->no_exec || v7_is_class(v[0], V7_CLASS_FUNCTION),
        V7_CALLED_NON_FUNCTION);

  // Push arguments on stack
  TRY(match(v7, '('));
  while (*v7->pc != ')') {
    TRY(parse_expression(v7));
    test_and_skip_char(v7, ',');
    num_args++;
  }
  TRY(match(v7, ')'));

  TRY(v7_call(v7, this_obj, num_args, called_as_ctor));

  return V7_OK;
}

static enum v7_err parse_string_literal(struct v7 *v7) {
  char buf[MAX_STRING_LITERAL_LENGTH];
  const char *begin = v7->pc++;
  struct v7_val *v;
  size_t i = 0;

  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v = v7_top(v7)[-1];

  // Scan string literal into the buffer, handle escape sequences
  while (*v7->pc != *begin && *v7->pc != '\0') {
    switch (*v7->pc) {
      case '\\':
        v7->pc++;
        switch (*v7->pc) {
          // TODO: add escapes for quotes, \XXX, \xXX, \uXXXX
          case 'b': buf[i++] = '\b'; break;
          case 'f': buf[i++] = '\f'; break;
          case 'n': buf[i++] = '\n'; break;
          case 'r': buf[i++] = '\r'; break;
          case 't': buf[i++] = '\t'; break;
          case 'v': buf[i++] = '\v'; break;
          case '\\': buf[i++] = '\\'; break;
          default: if (*v7->pc == *begin) buf[i++] = *begin; break;
        }
        break;
      default:
        buf[i++] = *v7->pc;
        break;
    }
    if (i >= sizeof(buf) - 1) i = sizeof(buf) - 1;
    v7->pc++;
  }
  v7_init_str(v, buf, v7->no_exec ? 0 : i, 1);
  TRY(match(v7, *begin));
  skip_whitespaces_and_comments(v7);

  return V7_OK;
}

enum v7_err v7_append(struct v7 *v7, struct v7_val *arr, struct v7_val *val) {
  struct v7_prop **head, *prop;
  CHECK(v7_is_class(arr, V7_CLASS_ARRAY), V7_INTERNAL_ERROR);
  // Append to the end of the list, to make indexing work
  for (head = &arr->v.array; *head != NULL; head = &head[0]->next);
  prop = mkprop(v7);
  CHECK(prop != NULL, V7_OUT_OF_MEMORY);
  prop->next = *head;
  *head = prop;
  prop->key = NULL;
  prop->val = val;
  inc_ref_count(val);
  return V7_OK;
}

static enum v7_err parse_array_literal(struct v7 *v7) {
  // Push empty array on stack
  if (!v7->no_exec) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    v7_set_class(v7_top(v7)[-1], V7_CLASS_ARRAY);
  }
  TRY(match(v7, '['));

  // Scan array literal, append elements one by one
  while (*v7->pc != ']') {
    // Push new element on stack
    TRY(parse_expression(v7));
    if (!v7->no_exec) {
      TRY(v7_append(v7, v7_top(v7)[-2], v7_top(v7)[-1]));
      TRY(inc_stack(v7, -1));
    }
    test_and_skip_char(v7, ',');
  }

  TRY(match(v7, ']'));
  return V7_OK;
}

static enum v7_err parse_object_literal(struct v7 *v7) {
  // Push empty object on stack
  TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
  TRY(match(v7, '{'));

  // Assign key/values to the object, until closing "}" is found
  while (*v7->pc != '}') {
    // Push key on stack
    if (*v7->pc == '\'' || *v7->pc == '"') {
      TRY(parse_string_literal(v7));
    } else {
      struct v7_val *v;
      TRY(parse_identifier(v7));
      v = v7_mkv(v7, V7_TYPE_STR, v7->tok, v7->tok_len, 1);
      CHECK(v != NULL, V7_OUT_OF_MEMORY);
      TRY(v7_push(v7, v));
    }

    // Push value on stack
    TRY(match(v7, ':'));
    TRY(parse_expression(v7));

    // Stack should now have object, key, value. Assign, and remove key/value
    if (!v7->no_exec) {
      struct v7_val **v = v7_top(v7) - 3;
      CHECK(v[0]->type == V7_TYPE_OBJ, V7_INTERNAL_ERROR);
      TRY(v7_set(v7, v[0], v[1], v[2]));
      TRY(inc_stack(v7, -2));
    }
    test_and_skip_char(v7, ',');
  }
  TRY(match(v7, '}'));
  return V7_OK;
}

enum v7_err v7_del(struct v7 *v7, struct v7_val *obj, struct v7_val *key) {
  struct v7_prop **p;
  CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_MISMATCH);
  for (p = &obj->props; *p != NULL; p = &p[0]->next) {
    if (cmp(key, p[0]->key) == 0) {
      struct v7_prop *next = p[0]->next;
      free_prop(v7, p[0]);
      p[0] = next;
      break;
    }
  }
  return V7_OK;
}

static enum v7_err parse_delete(struct v7 *v7) {
  struct v7_val key;
  TRY(parse_expression(v7));
  key = str_to_val(v7->tok, v7->tok_len);  // Must go after parse_expression
  TRY(v7_del(v7, v7->cur_obj, &key));
  return V7_OK;
}

static enum v7_err parse_regex(struct v7 *v7) {
  char regex[MAX_STRING_LITERAL_LENGTH];
  size_t i;

  CHECK(*v7->pc == '/', V7_SYNTAX_ERROR);
  for (i = 0, v7->pc++; i < sizeof(regex) - 1 && *v7->pc != '/' &&
    *v7->pc != '\0'; i++, v7->pc++) {
    if (*v7->pc == '\\' && v7->pc[1] == '/') v7->pc++;
    regex[i] = *v7->pc;
  }
  regex[i] = '\0';
  TRY(match(v7, '/'));
  if (!v7->no_exec) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    v7_set_class(v7_top(v7)[-1], V7_CLASS_REGEXP);
    v7_top(v7)[-1]->v.regex = v7_strdup(regex, strlen(regex));
  }

  return V7_OK;
}

static enum v7_err parse_variable(struct v7 *v7) {
  struct v7_val key = str_to_val(v7->tok, v7->tok_len), *v = NULL;
  if (!v7->no_exec) {
    v = find(v7, &key);
    if (v == NULL) {
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
    } else {
      TRY(v7_push(v7, v));
    }
  }
  return V7_OK;
}

static enum v7_err parse_precedence_0(struct v7 *v7) {
  if (*v7->pc == '(') {
    TRY(match(v7, '('));
    TRY(parse_expression(v7));
    TRY(match(v7, ')'));
  } else if (*v7->pc == '\'' || *v7->pc == '"') {
    TRY(parse_string_literal(v7));
  } else if (*v7->pc == '{') {
    TRY(parse_object_literal(v7));
  } else if (*v7->pc == '[') {
    TRY(parse_array_literal(v7));
  } else if (*v7->pc == '/') {
    TRY(parse_regex(v7));
  } else if (is_valid_start_of_identifier(v7->pc[0])) {
    TRY(parse_identifier(v7));
    if (test_token(v7, "this", 4)) {
      TRY(v7_push(v7, v7->this_obj));
    } else if (test_token(v7, "null", 4)) {
      TRY(v7_make_and_push(v7, V7_TYPE_NULL));
    } else if (test_token(v7, "undefined", 9)) {
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
    } else if (test_token(v7, "true", 4)) {
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = 1;
    } else if (test_token(v7, "false", 5)) {
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = 0;
    } else if (test_token(v7, "function", 8)) {
      TRY(parse_function_definition(v7, NULL, 0));
    } else if (test_token(v7, "delete", 6)) {
      TRY(parse_delete(v7));
    } else if (test_token(v7, "NaN", 3)) {
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = NAN;
    } else if (test_token(v7, "Infinity", 8)) {
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = INFINITY;
    } else {
      TRY(parse_variable(v7));
    }
  } else {
    TRY(parse_num(v7));
  }

  return V7_OK;
}


static enum v7_err parse_prop_accessor(struct v7 *v7, int op) {
  struct v7_val *v = NULL, *ns = NULL;

  if (!v7->no_exec) {
    ns = v7_top(v7)[-1];
    v7_make_and_push(v7, V7_TYPE_UNDEF);
    v = v7_top(v7)[-1];
  }
  v7->cur_obj = v7->this_obj = ns;
  CHECK(v7->no_exec || ns != NULL, V7_SYNTAX_ERROR);

  if (op == '.') {
    TRY(parse_identifier(v7));
    if (!v7->no_exec) {
      struct v7_val key = str_to_val(v7->tok, v7->tok_len);
      ns = get2(ns, &key);
      if (ns != NULL && (ns->flags & V7_PROP_FUNC)) {
        ns->v.prop_func(v7->cur_obj, v);
        ns = v;
      }
    }
  } else {
    TRY(parse_expression(v7));
    TRY(match(v7, ']'));
    if (!v7->no_exec) {
      ns = get2(ns, v7_top(v7)[-1]);
      if (ns != NULL && (ns->flags & V7_PROP_FUNC)) {
        ns->v.prop_func(v7->cur_obj, v);
        ns = v;
      }
      TRY(inc_stack(v7, -1));
    }
  }

  if (!v7->no_exec) {
    TRY(v7_push(v7, ns == NULL ? v : ns));
  }

  return V7_OK;
}

static enum v7_err parse_precedence_1(struct v7 *v7, int has_new) {
  struct v7_val *old_this = v7->this_obj;

  TRY(parse_precedence_0(v7));
  while (*v7->pc == '.' || *v7->pc == '[') {
    int op = v7->pc[0];
    TRY(match(v7, op));
    TRY(parse_prop_accessor(v7, op));

    while (*v7->pc == '(') {
      TRY(parse_function_call(v7, v7->cur_obj, has_new));
    }
  }
  v7->this_obj = old_this;

  return V7_OK;
}

static enum v7_err parse_precedence_2(struct v7 *v7) {
  int has_new = 0;
  struct v7_val *old_this_obj = v7->this_obj, *cur_this = v7->this_obj;

  if (lookahead(v7, "new", 3)) {
    has_new++;
    if (!v7->no_exec) {
      v7_make_and_push(v7, V7_TYPE_OBJ);
      cur_this = v7->this_obj = v7_top(v7)[-1];
    }
  }
  TRY(parse_precedence_1(v7, has_new));
  while (*v7->pc == '(') {
    TRY(parse_function_call(v7, v7->this_obj, has_new));
  }

  if (has_new && !v7->no_exec) {
    TRY(v7_push(v7, cur_this));
  }

  v7->this_obj = old_this_obj;

  return V7_OK;
}

static enum v7_err parse_precedence_3(struct v7 *v7) {
  TRY(parse_precedence_2(v7));
  if ((v7->pc[0] == '+' && v7->pc[1] == '+') ||
      (v7->pc[0] == '-' && v7->pc[1] == '-')) {
    int increment = (v7->pc[0] == '+') ? 1 : -1;
    v7->pc += 2;
    skip_whitespaces_and_comments(v7);
    if (!v7->no_exec) {
      struct v7_val *v = v7_top(v7)[-1];
      CHECK(v->type == V7_TYPE_NUM, V7_TYPE_MISMATCH);
      v->v.num += increment;
    }
  }
  return V7_OK;
}

static enum v7_err parse_precedence4(struct v7 *v7) {
  int has_neg = 0, has_typeof = 0;

  if (v7->pc[0] == '!') {
    TRY(match(v7, v7->pc[0]));
    has_neg++;
  }
  has_typeof = lookahead(v7, "typeof", 6);

  TRY(parse_precedence_3(v7));
  if (has_neg && !v7->no_exec) {
    int is_true = v7_is_true(v7_top(v7)[-1]);
    TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
    v7_top(v7)[-1]->v.num = is_true ? 0.0 : 1.0;
  }
  if (has_typeof && !v7->no_exec) {
    const struct v7_val *v = v7_top(v7)[-1];
    static const char *names[] = {
      "undefined", "null", "boolean", "string", "number", "object"
    };
    const char *s = names[v->type];
    if (v7_is_class(v, V7_CLASS_ARRAY)) s = "array";
    if (v7_is_class(v, V7_CLASS_FUNCTION)) s = "function";
    TRY(v7_push(v7, v7_mkv(v7, V7_TYPE_STR, s, strlen(s), 0)));
  }

  return V7_OK;
}

static enum v7_err parse_term(struct v7 *v7) {
  TRY(parse_precedence4(v7));
  while ((*v7->pc == '*' || *v7->pc == '/' || *v7->pc == '%') &&
         v7->pc[1] != '=') {
    int sp1 = v7->sp, ch = *v7->pc;
    TRY(match(v7, ch));
    TRY(parse_precedence4(v7));
    if (!v7->no_exec) {
      TRY(do_arithmetic_op(v7, ch, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static int is_relational_op(const char *s) {
  switch (s[0]) {
    case '>': return s[1] == '=' ? OP_GREATER_EQUAL : OP_GREATER_THEN;
    case '<': return s[1] == '=' ? OP_LESS_EQUAL : OP_LESS_THEN;
    default: return OP_INVALID;
  }
}

static int is_equality_op(const char *s) {
  if (s[0] == '=' && s[1] == '=') {
    return s[2] == '=' ? OP_EQUAL_EQUAL : OP_EQUAL;
  } else if (s[0] == '!' && s[1] == '=') {
    return s[2] == '=' ? OP_NOT_EQUAL_EQUAL : OP_NOT_EQUAL;
  }
  return OP_INVALID;
}

static enum v7_err do_logical_op(struct v7 *v7, int op, int sp1, int sp2) {
  struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7->stack[sp2 - 1];
  int res = 0;

  if (v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM) {
    switch (op) {
      case OP_GREATER_THEN:   res = v1->v.num >  v2->v.num; break;
      case OP_GREATER_EQUAL:  res = v1->v.num >= v2->v.num; break;
      case OP_LESS_THEN:      res = v1->v.num <  v2->v.num; break;
      case OP_LESS_EQUAL:     res = v1->v.num <= v2->v.num; break;
      case OP_EQUAL: // FALLTHROUGH
      case OP_EQUAL_EQUAL:    res = cmp(v1, v2) == 0; break;
      case OP_NOT_EQUAL: // FALLTHROUGH
      case OP_NOT_EQUAL_EQUAL:  res = cmp(v1, v2) != 0; break;
    }
  } else if (op == OP_EQUAL || op == OP_EQUAL_EQUAL) {
    res = cmp(v1, v2) == 0;
  } else if (op == OP_NOT_EQUAL || op == OP_NOT_EQUAL_EQUAL) {
    res = cmp(v1, v2) != 0;
  }
  TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
  v7_top(v7)[-1]->v.num = res ? 1.0 : 0.0;
  return V7_OK;
}

static enum v7_err parse_assign(struct v7 *v7, struct v7_val *obj, int op) {
  const char *tok = v7->tok;
  unsigned long tok_len = v7->tok_len;

  v7->pc += s_op_lengths[op];
  skip_whitespaces_and_comments(v7);
  TRY(parse_expression(v7));

  // Stack layout at this point (assuming stack grows down):
  //
  //          | object's value (rvalue)    |    top[-2]
  //          +----------------------------+
  //          | expression value (lvalue)  |    top[-1]
  //          +----------------------------+
  // top -->  |       nothing yet          |
  if (!v7->no_exec) {
    struct v7_val **top = v7_top(v7), *a = top[-2], *b = top[-1];
    switch (op) {
      case OP_ASSIGN:
        CHECK(v7->sp >= 2, V7_INTERNAL_ERROR);
        TRY(v7_setv(v7, obj, V7_TYPE_STR, V7_TYPE_OBJ, tok, tok_len, 1, b));
        return V7_OK;
      case OP_PLUS_ASSIGN: TRY(arith(a, b, a, '+')); break;
      case OP_MINUS_ASSIGN: TRY(arith(a, b, a, '-')); break;
      case OP_MUL_ASSIGN: TRY(arith(a, b, a, '*')); break;
      case OP_DIV_ASSIGN: TRY(arith(a, b, a, '/')); break;
      case OP_REM_ASSIGN: TRY(arith(a, b, a, '%')); break;
      case OP_XOR_ASSIGN: TRY(arith(a, b, a, '^')); break;
      default: return V7_NOT_IMPLEMENTED;
    }
    TRY(inc_stack(v7, -1));
  }

  return V7_OK;
}

static enum v7_err parse_add_sub(struct v7 *v7) {
  TRY(parse_term(v7));
  while ((*v7->pc == '-' || *v7->pc == '+') && v7->pc[1] != '=') {
    int sp1 = v7->sp, ch = *v7->pc;
    TRY(match(v7, ch));
    TRY(parse_term(v7));
    if (!v7->no_exec) {
      TRY(do_arithmetic_op(v7, ch, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err parse_relational(struct v7 *v7) {
  int op;
  TRY(parse_add_sub(v7));
  while ((op = is_relational_op(v7->pc)) > OP_INVALID) {
    int sp1 = v7->sp;
    v7->pc += s_op_lengths[op];
    skip_whitespaces_and_comments(v7);
    TRY(parse_add_sub(v7));
    if (!v7->no_exec) {
      TRY(do_logical_op(v7, op, sp1, v7->sp));
    }
  }
  if (lookahead(v7, "instanceof", 10)) {
    TRY(parse_identifier(v7));
    if (!v7->no_exec) {
      struct v7_val key = str_to_val(v7->tok, v7->tok_len);
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = instanceof(v7_top(v7)[-2], find(v7, &key));
    }
  }
  return V7_OK;
}

static enum v7_err parse_equality(struct v7 *v7) {
  int op;
  TRY(parse_relational(v7));
  if ((op = is_equality_op(v7->pc)) > OP_INVALID) {
    int sp1 = v7->sp;
    v7->pc += s_op_lengths[op];
    skip_whitespaces_and_comments(v7);
    TRY(parse_relational(v7));
    if (!v7->no_exec) {
      TRY(do_logical_op(v7, op, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_and(struct v7 *v7) {
  TRY(parse_equality(v7));
  if (*v7->pc == '&' && v7->pc[1] != '&' && v7->pc[1] != '=') {
    int sp1 = v7->sp;
    TRY(match(v7, '&'));
    TRY(parse_equality(v7));
    if (!v7->no_exec) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = v1->v.num, b = v2->v.num;
      CHECK(v1->type == V7_TYPE_NUM && v1->type == V7_TYPE_NUM, V7_TYPE_MISMATCH);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a & b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_xor(struct v7 *v7) {
  TRY(parse_bitwise_and(v7));
  if (*v7->pc == '^' && v7->pc[1] != '=') {
    int sp1 = v7->sp;
    TRY(match(v7, '^'));
    TRY(parse_bitwise_and(v7));
    if (!v7->no_exec) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = v1->v.num, b = v2->v.num;
      CHECK(v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM, V7_TYPE_MISMATCH);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a ^ b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_or(struct v7 *v7) {
  TRY(parse_bitwise_xor(v7));
  if (*v7->pc == '|' && v7->pc[1] != '=' && v7->pc[1] != '|') {
    int sp1 = v7->sp;
    TRY(match(v7, '|'));
    TRY(parse_bitwise_xor(v7));
    if (!v7->no_exec) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = v1->v.num, b = v2->v.num;
      CHECK(v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM, V7_TYPE_MISMATCH);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a | b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_logical_and(struct v7 *v7) {
  TRY(parse_bitwise_or(v7));
  while (*v7->pc == '&' && v7->pc[1] == '&') {
    int sp1 = v7->sp;
    match(v7, '&');
    match(v7, '&');
    TRY(parse_bitwise_or(v7));
    if (!v7->no_exec) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      int is_true = v7_is_true(v1) && v7_is_true(v2);
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = is_true ? 1.0 : 0.0;
    }
  }
  return V7_OK;
}

static enum v7_err parse_logical_or(struct v7 *v7) {
  TRY(parse_logical_and(v7));
  if (*v7->pc == '|' && v7->pc[1] == '|') {
    int sp1 = v7->sp;
    match(v7, '|');
    match(v7, '|');
    TRY(parse_logical_and(v7));
    if (!v7->no_exec) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      int is_true = v7_is_true(v1) || v7_is_true(v2);
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = is_true ? 1.0 : 0.0;
    }
  }
  return V7_OK;
}

static int is_assign_op(const char *s) {
  if (s[0] == '=') {
    return OP_ASSIGN;
  } else if (s[1] == '=') {
    switch (s[0]) {
      case '+': return OP_PLUS_ASSIGN;
      case '-': return OP_MINUS_ASSIGN;
      case '*': return OP_MUL_ASSIGN;
      case '/': return OP_DIV_ASSIGN;
      case '%': return OP_REM_ASSIGN;
      case '&': return OP_AND_ASSIGN;
      case '^': return OP_XOR_ASSIGN;
      case '|': return OP_OR_ASSIGN;
      default: return OP_INVALID;
    }
  } else if (s[0] == '<' && s[1] == '<' && s[2] == '=') {
    return OP_LSHIFT_ASSIGN;
  } else if (s[0] == '>' && s[1] == '>' && s[2] == '=') {
    return OP_RSHIFT_ASSIGN;
  } else if (s[0] == '>' && s[1] == '>' && s[2] == '>' && s[3] == '=') {
    return OP_RRSHIFT_ASSIGN;
  } else {
    return OP_INVALID;
  }
}

static enum v7_err parse_expression(struct v7 *v7) {
#ifdef V7_DEBUG
  const char *stmt_str = v7->pc;
#endif
  int op, old_sp = v7->sp;

  v7->cur_obj = cur_scope(v7);
  TRY(parse_logical_or(v7));

  // Parse assignment
  if ((op = is_assign_op(v7->pc))) {
    TRY(parse_assign(v7, v7->cur_obj, op));
  }

  // Parse ternary operator
  if (*v7->pc == '?') {
    int old_no_exec = v7->no_exec;
    int condition_true = 1;

    if (!v7->no_exec) {
      CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
      condition_true = v7_is_true(v7_top(v7)[-1]);
      TRY(inc_stack(v7, -1));   // Remove condition result
    }

    TRY(match(v7, '?'));
    v7->no_exec = old_no_exec || !condition_true;
    TRY(parse_expression(v7));
    TRY(match(v7, ':'));
    v7->no_exec = old_no_exec || condition_true;
    TRY(parse_expression(v7));
    v7->no_exec = old_no_exec;
  }

  // Collapse stack, leave only one value on top
  if (!v7->no_exec) {
    struct v7_val *result = v7_top(v7)[-1];
    inc_ref_count(result);
    TRY(inc_stack(v7, old_sp - v7->sp));
    TRY(v7_push(v7, result));
    assert(result->ref_count > 1);
    v7_freeval(v7, result);
  }

  return V7_OK;
}

static enum v7_err parse_declaration(struct v7 *v7) {
  int sp = v7_sp(v7);

  do {
    inc_stack(v7, sp - v7_sp(v7));  // Clean up the stack after prev decl
    TRY(parse_identifier(v7));
    if (v7->no_exec) {
      v7_setv(v7, cur_scope(v7), V7_TYPE_STR, V7_TYPE_UNDEF, v7->tok, v7->tok_len, 1);
    }
    if (*v7->pc == '=') {
      if (!v7->no_exec) v7_make_and_push(v7, V7_TYPE_UNDEF);
      TRY(parse_assign(v7, cur_scope(v7), OP_ASSIGN));
    }
  } while (test_and_skip_char(v7, ','));

  return V7_OK;
}

static enum v7_err parse_if_statement(struct v7 *v7, int *has_return) {
  int old_no_exec = v7->no_exec;  // Remember execution flag

  TRY(match(v7, '('));
  TRY(parse_expression(v7));      // Evaluate condition, pushed on stack
  TRY(match(v7, ')'));
  if (!old_no_exec) {
    // If condition is false, do not execute "if" body
    CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
    v7->no_exec = !v7_is_true(v7_top(v7)[-1]);
    TRY(inc_stack(v7, -1));   // Cleanup condition result from the stack
  }
  TRY(parse_compound_statement(v7, has_return));

  if (strncmp(v7->pc, "else", 4) == 0) {
    v7->pc += 4;
    skip_whitespaces_and_comments(v7);
    v7->no_exec = old_no_exec || !v7->no_exec;
    TRY(parse_compound_statement(v7, has_return));
  }

  v7->no_exec = old_no_exec;  // Restore old execution flag
  return V7_OK;
}

static enum v7_err parse_for_in_statement(struct v7 *v7, int has_var,
                                          int *has_return) {
  const char *tok = v7->tok, *stmt;
  int tok_len = v7->tok_len, line_stmt;

  TRY(parse_expression(v7));
  TRY(match(v7, ')'));
  stmt = v7->pc;
  line_stmt = v7->line_no;

  // Execute loop body
  if (v7->no_exec) {
    TRY(parse_compound_statement(v7, has_return));
  } else {
    int old_sp = v7->sp;
    struct v7_val *obj = v7_top(v7)[-1];
    struct v7_val *scope = has_var ? cur_scope(v7) : &v7->root_scope;
    struct v7_prop *prop;

    CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_MISMATCH);
    for (prop = obj->props; prop != NULL; prop = prop->next) {
      TRY(v7_setv(v7, scope, V7_TYPE_STR, V7_TYPE_OBJ,
                  tok, tok_len, 1, prop->key));
      v7->pc = stmt;
      v7->line_no = line_stmt;
      TRY(parse_compound_statement(v7, has_return));  // Loop body
      TRY(inc_stack(v7, old_sp - v7->sp));  // Clean up stack
    }
  }

  return V7_OK;
}

static enum v7_err parse_for_statement(struct v7 *v7, int *has_return) {
  int line_expr1, line_expr2, line_expr3, line_stmt, line_end,
    is_true, old_no_exec = v7->no_exec, has_var = 0;
  const char *expr1, *expr2, *expr3, *stmt, *end;

  TRY(match(v7, '('));
  expr1 = v7->pc;
  line_expr1 = v7->line_no;

  // See if this is an enumeration loop
  if (lookahead(v7, "var", 3)) {
    has_var = 1;
  }
  if (parse_identifier(v7) == V7_OK && lookahead(v7, "in", 2)) {
    return parse_for_in_statement(v7, has_var, has_return);
  } else {
    v7->pc = expr1;
    v7->line_no = line_expr1;
  }

  if (lookahead(v7, "var", 3)) {
    parse_declaration(v7);
  } else {
    TRY(parse_expression(v7));    // expr1
  }
  TRY(match(v7, ';'));

  // Pass through the loop, don't execute it, just remember locations
  v7->no_exec = 1;
  expr2 = v7->pc;
  line_expr2 = v7->line_no;
  TRY(parse_expression(v7));    // expr2 (condition)
  TRY(match(v7, ';'));

  expr3 = v7->pc;
  line_expr3 = v7->line_no;
  TRY(parse_expression(v7));    // expr3  (post-iteration)
  TRY(match(v7, ')'));

  stmt = v7->pc;
  line_stmt = v7->line_no;
  TRY(parse_compound_statement(v7, has_return));
  end = v7->pc;
  line_end = v7->line_no;

  v7->no_exec = old_no_exec;

  // Execute loop
  if (!v7->no_exec) {
    int old_sp = v7->sp;
    for (;;) {
      v7->pc = expr2;
      v7->line_no = line_expr2;
      TRY(parse_expression(v7));    // Evaluate condition
      is_true = !v7_is_true(v7_top(v7)[-1]);
      TRY(inc_stack(v7, -1));
      if (is_true) break;

      v7->pc = stmt;
      v7->line_no = line_stmt;
      TRY(parse_compound_statement(v7, has_return));  // Loop body

      v7->pc = expr3;
      v7->line_no = line_expr3;
      TRY(parse_expression(v7));    // expr3  (post-iteration)

      TRY(inc_stack(v7, old_sp - v7->sp));  // Clean up stack
    }
  }

  // Jump to the code after the loop
  v7->line_no = line_end;
  v7->pc = end;

  return V7_OK;
}

static enum v7_err parse_statement(struct v7 *v7, int *has_return) {
  if (is_valid_start_of_identifier(v7->pc[0])) {
    TRY(parse_identifier(v7));    // Load identifier into v7->tok, v7->tok_len
    if (test_token(v7, "var", 3)) {
      TRY(parse_declaration(v7));
    } else if (test_token(v7, "return", 6)) {
      if (!v7->no_exec) {
        *has_return = 1;
      }
      if (*v7->pc != ';' && *v7->pc != '}') {
        TRY(parse_expression(v7));
      }
    } else if (test_token(v7, "if", 2)) {
      TRY(parse_if_statement(v7, has_return));
    } else if (test_token(v7, "for", 3)) {
      TRY(parse_for_statement(v7, has_return));
    } else {
      v7->pc = v7->tok;
      TRY(parse_expression(v7));
    }
  } else {
    TRY(parse_expression(v7));
  }

  // Skip optional colons and semicolons
  while (*v7->pc == ',') match(v7, *v7->pc);
  while (*v7->pc == ';') match(v7, *v7->pc);
  return V7_OK;
}

static enum v7_err do_exec(struct v7 *v7, const char *source_code, int sp) {
  int has_ret = 0;

  v7->source_code = v7->pc = source_code;
  skip_whitespaces_and_comments(v7);

  // Prior calls to v7_exec() may have left current_scope modified, reset now
  // TODO(lsm): free scope chain
  v7->this_obj = &v7->root_scope;

  while (*v7->pc != '\0') {
    TRY(inc_stack(v7, sp - v7->sp));      // Reset stack on each statement
    TRY(parse_statement(v7, &has_ret));   // Last expr result on stack
  }
  assert(v7->root_scope.proto == &s_global);

  return V7_OK;
}

enum v7_err v7_exec(struct v7 *v7, const char *source_code) {
  return do_exec(v7, source_code, 0);
}

enum v7_err v7_exec_file(struct v7 *v7, const char *path) {
  FILE *fp;
  char *p, *old_pc = (char *) v7->pc;
  long file_size, old_line_no = v7->line_no;
  enum v7_err status = V7_INTERNAL_ERROR;

  if ((fp = fopen(path, "r")) == NULL) {
  } else if (fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0) {
    fclose(fp);
  } else if ((p = (char *) calloc(1, (size_t) file_size + 1)) == NULL) {
    fclose(fp);
  } else {
    rewind(fp);
    fread(p, 1, (size_t) file_size, fp);
    fclose(fp);
    v7->line_no = 1;
    status = do_exec(v7, p, v7->sp);
    free(p);
    v7->pc = old_pc;
    if (status == V7_OK) v7->line_no = (int) old_line_no;
  }

  return status;
}

const char *v7_strerror(enum v7_err e) {
  static const char *strings[] = {
    "no error", "syntax error", "out of memory", "internal error",
    "stack overflow", "stack underflow", "undefined variable", "type mismatch",
    "called non-function", "not implemented"
  };
  assert(ARRAY_SIZE(strings) == V7_NUM_ERRORS);
  return e >= (int) ARRAY_SIZE(strings) ? "?" : strings[e];
}

#ifdef V7_EXE
int main(int argc, char *argv[]) {
  struct v7 *v7 = v7_create();
  int i, error_code;

  // Execute inline code
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
      if ((error_code = v7_exec(v7, argv[i + 1])) != V7_OK) {
        fprintf(stderr, "Error executing [%s]: %s\n", argv[i + 1],
                v7_strerror(error_code));
      }
      i++;
    }
  }

  // Execute files
  for (; i < argc; i++) {
    if ((error_code = v7_exec_file(v7, argv[i])) != V7_OK) {
      fprintf(stderr, "%s line %d: %s\n", argv[i], v7->line_no,
              v7_strerror(error_code));
    }
  }

  v7_destroy(&v7);
  return EXIT_SUCCESS;
}
#endif
