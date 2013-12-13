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

#ifndef EJS_HEADER_INCLUDED
#define  EJS_HEADER_INCLUDED

#define EJS_VERSION "1.0"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct ejs *ejs_create(void);
void ejs_destroy(struct ejs **);
int ejs_exec(struct ejs *, const char *str, int str_len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // EJS_HEADER_INCLUDED
