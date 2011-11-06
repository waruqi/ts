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
 * \file		utils.h
 *
 */
#ifndef TB_PREFIX_UTILS_H
#define TB_PREFIX_UTILS_H

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

// abs
#define tb_abs(x) 				((x) > 0? (x) : -(x))

// min & max
#define tb_max(x, y) 			(((x) > (y))? (x) : (y))
#define tb_min(x, y) 			(((x) < (y))? (x) : (y))

// the number of entries in the array
#define tb_arrayn(x) 			(sizeof((x)) / sizeof((x)[0]))

// align
#define tb_align2(x) 			(((x) + 1) >> 1 << 1)
#define tb_align4(x) 			(((x) + 3) >> 2 << 2)
#define tb_align8(x) 			(((x) + 7) >> 4 << 4)
#define tb_align(x, b) 			(((x) + ((b) - 1)) & ~((b) - 1))


// c plus plus
#ifdef __cplusplus
}
#endif

#endif


