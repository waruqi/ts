/*!The Tiny Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2011, ruki All rights reserved.
 *
 * \author		ruki
 * \file		stdarg.h
 *
 */
#ifndef TB_LIBC_STDARG_H
#define TB_LIBC_STDARG_H

// c plus plus
#ifdef __cplusplus
extern "C" {
#endif

/* /////////////////////////////////////////////////////////
 * includes
 */
#include "../prefix.h"
//#include <stdarg.h>

/* /////////////////////////////////////////////////////////
 * macros
 */
#if 0
#define tb_va_start(v, l) 	va_start(v, l)
#define tb_va_end(v) 		va_end(v)
#define tb_va_arg(v, l) 	va_arg(v, l)
#else // gcc
#define tb_va_start(v, l)	__builtin_va_start(v, l)
#define tb_va_end(v)		__builtin_va_end(v)
#define tb_va_arg(v, l)		__builtin_va_arg(v, l)
#endif

// varg
#define TB_VA_FMT(s, n, fmt, r) \
do \
{ \
	tb_int_t __tb_ret = 0; \
	tb_va_list_t __tb_varg_list; \
    tb_va_start(__tb_varg_list, fmt); \
    __tb_ret = tb_vsnprintf(s, (n), fmt, __tb_varg_list); \
    tb_va_end(__tb_varg_list); \
	if (__tb_ret >= 0) s[__tb_ret] = '\0'; \
	if (r) *r = __tb_ret > 0? __tb_ret : 0; \
 \
} while (0) 


/* /////////////////////////////////////////////////////////
 * types
 */

//typedef va_list 	tb_va_list_t;
typedef __builtin_va_list 	tb_va_list_t;


// c plus plus
#ifdef __cplusplus
}
#endif

#endif
