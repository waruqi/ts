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
 * \file		cpu.h
 *
 */
#ifndef TB_PREFIX_CPU_H
#define TB_PREFIX_CPU_H

// c plus plus
#ifdef __cplusplus
extern "C" {
#endif

/* /////////////////////////////////////////////////////////
 * includes
 */
#include "config.h"

/* /////////////////////////////////////////////////////////
 * macros
 */

// check 64-bits
#if defined (__LP64__) \
	|| defined (__64BIT__) \
		|| defined (_LP64) \
		|| (__WORDSIZE == 64)
# 	define TB_CPU_BITSIZE 		(64)
# 	define TB_CPU_BITBYTE 		(8)
# 	define TB_CPU_BITALIGN 		(7)
#else
# 	define TB_CPU_BITSIZE 		(32)
# 	define TB_CPU_BITBYTE 		(4)
# 	define TB_CPU_BITALIGN 		(3)
#endif


// c plus plus
#ifdef __cplusplus
}
#endif

#endif


