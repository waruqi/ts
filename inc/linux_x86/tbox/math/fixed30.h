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
 * \file		fixed30.h
 *
 */
#ifndef TB_MATH_FIXED30_H
#define TB_MATH_FIXED30_H

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "int32.h"

#ifdef TB_CONFIG_TYPE_FLOAT
# 	include "float.h"
#endif

#if defined(TB_CONFIG_ARCH_x86)
# 	include "opt/fixed16_x86.h"
#elif defined(TB_CONFIG_ARCH_ARM)
# 	include "opt/fixed16_arm.h"
#elif defined(TB_CONFIG_ARCH_SH4)
# 	include "opt/fixed16_sh4.h"
#endif

/* ////////////////////////////////////////////////////////////////////////
 * macros
 */

// constant
#define TB_FIXED30_ONE 						(1 << 30)
#define TB_FIXED30_HALF 					(1 << 29)
#define TB_FIXED30_MAX 						(TB_MAXS32)
#define TB_FIXED30_MIN 						(-TB_FIXED30_MAX)
#define TB_FIXED30_NAN 						((tb_int_t)0x80000000)
#define TB_FIXED30_INF 						(TB_MAXS32)
#define TB_FIXED30_SQRT2 					(0x5a827999)

// conversion
#ifdef TB_CONFIG_TYPE_FLOAT
# 	ifndef tb_fixed30_to_float
# 		define tb_fixed30_to_float(x) 		(((x) * 0.00000000093132257f))
# 	endif
# 	ifndef tb_float_to_fixed30
# 		ifdef TB_DEBUG
# 			define tb_float_to_fixed30(x) 	tb_float_to_fixed30_check(x)
# 		else
# 			define tb_float_to_fixed30(x) 	((tb_fixed30_t)((x) * TB_FIXED30_ONE))
# 		endif
# 	endif
#endif

#ifdef TB_DEBUG
# 	define tb_fixed16_to_fixed30(x) 		tb_fixed16_to_fixed30_check(x)
#else
# 	define tb_fixed16_to_fixed30(x) 		((x) << 14)
#endif
#define tb_fixed30_to_fixed16(x) 			((x) >> 14)

// abs
#define tb_fixed30_abs(x) 					tb_abs(x)

// avg
#define tb_fixed30_avg(x, y) 				(((x) + (y)) >> 1)

// mul
#ifndef tb_fixed30_mul
# 	if defined(TB_CONFIG_TYPE_INT64)
# 		define tb_fixed30_mul(x, y) 		tb_fixed30_mul_int64(x, y)
# 	elif defined(TB_CONFIG_TYPE_FLOAT)
# 		define tb_fixed30_mul(x, y) 		tb_fixed30_mul_float(x, y)
# 	else
# 		define tb_fixed30_mul(x, y) 		tb_fixed30_mul_int32(x, y)
# 	endif
#endif

// div
#ifndef tb_fixed30_div
# 	if defined(TB_CONFIG_TYPE_INT64)
# 		define tb_fixed30_div(x, y) 		tb_fixed30_div_int64(x, y)
# 	elif defined(TB_CONFIG_TYPE_FLOAT)
# 		define tb_fixed30_div(x, y) 		tb_fixed30_div_float(x, y)
# 	else
# 		define tb_fixed30_div(x, y) 		tb_int32_div(x, y, 30)
# 	endif
#endif

// sqre
#ifndef tb_fixed30_sqre
# 	if defined(TB_CONFIG_TYPE_INT64)
# 		define tb_fixed30_sqre(x) 			tb_fixed30_sqre_int64(x)
# 	elif defined(TB_CONFIG_TYPE_FLOAT)
# 		define tb_fixed30_sqre(x) 			tb_fixed30_sqre_float(x)
# 	else
# 		define tb_fixed30_sqre(x) 			tb_fixed30_sqre_int32(x)
# 	endif
#endif

// sqrt
#ifndef tb_fixed30_sqrt
# 	define tb_fixed30_sqrt(x) 				tb_fixed30_sqrt_int32(x)
#endif

/* ////////////////////////////////////////////////////////////////////////
 * inlines
 */

#ifdef TB_DEBUG
# 	ifdef TB_CONFIG_TYPE_FLOAT
static __tb_inline__ tb_fixed30_t tb_float_to_fixed30_check(tb_float_t x)
{
	// check overflow, [-2., 2.]
	TB_ASSERT(x >= -2. && x <= 2.);
	return ((tb_fixed30_t)((x) * TB_FIXED30_ONE));
}
# 	endif
static __tb_inline__ tb_fixed30_t tb_fixed16_to_fixed30_check(tb_fixed16_t x)
{
	// check overflow, [-2, 2]
	TB_ASSERT(x >= (-2 << 16) && x <= (2 << 16) - 1);
	return (x << 14);
}
#endif

#ifdef TB_CONFIG_TYPE_INT64
static __tb_inline__ tb_fixed30_t tb_fixed30_mul_int64(tb_fixed30_t x, tb_fixed30_t y)
{
	return (tb_fixed30_t)((tb_int64_t)x * y >> 30);
}
static __tb_inline__ tb_fixed30_t tb_fixed30_div_int64(tb_fixed30_t x, tb_fixed30_t y)
{
	TB_ASSERT(y);
	return (tb_fixed30_t)((((tb_int64_t)x) << 30) / y);
}
static __tb_inline__ tb_fixed30_t tb_fixed30_sqre_int64(tb_fixed30_t x)
{
	return (tb_fixed30_t)((tb_int64_t)x * x >> 30);
}
#endif

#ifdef TB_CONFIG_TYPE_FLOAT
static __tb_inline__ tb_fixed30_t tb_fixed30_mul_float(tb_fixed30_t x, tb_fixed30_t y)
{
	return tb_float_to_fixed30(tb_float_mul(tb_fixed30_to_float(x), tb_fixed30_to_float(y)));
}
static __tb_inline__ tb_fixed30_t tb_fixed30_div_float(tb_fixed30_t x, tb_fixed30_t y)
{
	TB_ASSERT(y);
	return tb_float_to_fixed30((tb_float_t)x / y);
	//return tb_float_to_fixed30(tb_float_div(tb_fixed30_to_float(x), tb_fixed30_to_float(y)));
}
static __tb_inline__ tb_fixed30_t tb_fixed30_sqre_float(tb_fixed30_t x)
{
	return tb_float_to_fixed30(tb_float_sqre(tb_fixed30_to_float(x)));
}
#endif

static __tb_inline__ tb_fixed30_t tb_fixed30_mul_int32(tb_fixed30_t x, tb_fixed30_t y)
{
	// get sign
	tb_int32_t s = tb_int32_get_sign(x ^ y);
	x = tb_fixed30_abs(x);
	y = tb_fixed30_abs(y);

	tb_uint32_t xh = x >> 16;
	tb_uint32_t xl = x & 0xffff;
	tb_uint32_t yh = y >> 16;
	tb_uint32_t yl = y & 0xffff;

    tb_uint32_t xyh = xh * yh;
    tb_uint32_t xyl = xl * yl;
    tb_uint32_t xyhl = xh * yl + xl * yh;

    tb_uint32_t lo = xyl + (xyhl << 16);
    tb_uint32_t hi = xyh + (xyhl >> 16) + (lo < xyl);

	// check overflow
    TB_ASSERT(!(hi >> 29));

    tb_uint32_t r = (hi << 2) + (lo >> 30);
	return tb_int32_set_sign(r, s);
}

static __tb_inline__ tb_fixed30_t tb_fixed30_sqre_int32(tb_fixed30_t x)
{
	x = tb_fixed30_abs(x);

	tb_uint32_t xh = x >> 16;
	tb_uint32_t xl = x & 0xffff;

    tb_uint32_t xxh = xh * xh;
    tb_uint32_t xxl = xl * xl;
    tb_uint32_t xxhl = (xh * xl) << 1;

    tb_uint32_t lo = xxl + (xxhl << 16);
    tb_uint32_t hi = xxh + (xxhl >> 16) + (lo < xxl);

	// check overflow
    TB_ASSERT(!(hi >> 29));

    return ((hi << 2) + (lo >> 30));
}
static __tb_inline__ tb_fixed30_t tb_fixed30_sqrt_int32(tb_fixed30_t x)
{
	TB_ASSERT(x > 0);
	return (x > 0? (tb_int32_sqrt(x) << 15) : 0);
}


#endif

