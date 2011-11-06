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
 * \file		ctype.h
 *
 */
#ifndef TB_LIBC_CTYPE_H
#define TB_LIBC_CTYPE_H

// c plus plus
#ifdef __cplusplus
extern "C" {
#endif

/* /////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"

/* /////////////////////////////////////////////////////////
 * macros
 */

// is
#define tb_isspace(x) 				((x) == 0x20 || ((x) > 0x8 && (x) < 0xe))
#define tb_isalpha(x) 				(((x) > 0x40 && (x) < 0x5b) || ((x) > 0x60 && (x) < 0x7b))
#define tb_isupper(x) 				((x) > 0x40 && (x) < 0x5b)
#define tb_islower(x) 				((x) > 0x60 && (x) < 0x7b)
#define tb_isascii(x) 				((x) >= 0x0 && (x) < 0x80)
#define tb_isdigit(x) 				((x) > 0x2f && (x) < 0x3a)
#define tb_isdigit2(x) 				((x) == '0' || (x) == '1')
#define tb_isdigit8(x) 				(((x) > 0x2f && (x) < 0x38))
#define tb_isdigit10(x) 			(tb_isdigit(x))
#define tb_isdigit16(x) 			(((x) > 0x2f && (x) < 0x3a) || ((x) > 0x40 && (x) < 0x47) || ((x) > 0x60 && (x) < 0x67))

// to lower & upper
#define tb_tolower(x) 				(tb_isupper(x)? (x) + 0x20 : (x))
#define tb_toupper(x) 				(tb_islower(x)? (x) - 0x20 : (x))



// c plus plus
#ifdef __cplusplus
}
#endif

#endif
