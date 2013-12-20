// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
// Copyright (c) 2013 Cesanta Software Limited
// All rights reserved
//
// This library is dual-licensed: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation. For the terms of this
// license, see <http://www.gnu.org/licenses/>.
//
// You are free to use this library under the terms of the GNU General
// Public License, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// Alternatively, you can license this library under a commercial
// license, as set out in <http://cesanta.com/products.html>.

#ifndef V7_HEADER_INCLUDED
#define  V7_HEADER_INCLUDED

#define V7_VERSION "1.0"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct v7 *v7_create(void);
void v7_destroy(struct v7 **);

const char *v7_exec(struct v7 *, const char *source_code);

int v7_set_func(struct v7 *, const char *name, void (*func)(struct v7 *));
int v7_set_num(struct v7 *, const char *name, double num);
int v7_set_str(struct v7 *, const char *name, const char *str, int str_len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // V7_HEADER_INCLUDED
