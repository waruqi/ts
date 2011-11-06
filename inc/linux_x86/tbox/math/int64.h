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
 * \file		int64.h
 *
 */
#ifndef TB_MATH_INT64_H
#define TB_MATH_INT64_H

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "sint64.h"
#include "uint64.h"

/* ////////////////////////////////////////////////////////////////////////
 * macros 
 */
#define TB_INT64_ZERO 				TB_SINT64_ZERO
#define TB_INT64_ONE 				TB_SINT64_ONE

#define tb_int32_to_int64(x) 		tb_sint32_to_sint64(x)
#define tb_int64_to_int32(x) 		tb_sint64_to_sint32(x)
#define tb_int64_make(h, l) 		tb_sint64_make(h, l)

#define tb_int64_abs(x) 			tb_sint64_abs(x)
#define tb_int64_neg(x) 			tb_sint64_neg(x)

#define tb_int64_add(x, y) 			tb_sint64_add(x, y)
#define tb_int64_sub(x, y) 			tb_sint64_sub(x, y)
#define tb_int64_mul(x, y) 			tb_sint64_mul(x, y)
#define tb_int64_div(x, y) 			tb_sint64_div(x, y)
#define tb_int64_mod(x, y) 			tb_sint64_mod(x, y)

#define tb_int64_clz(x) 			tb_sint64_clz(x)
#define tb_int64_not(x) 			tb_sint64_not(x)
#define tb_int64_or(x, y) 			tb_sint64_or(x, y)
#define tb_int64_and(x, y) 			tb_sint64_and(x, y)
#define tb_int64_xor(x, y) 			tb_sint64_xor(x, y)

#define tb_int64_lsh(x, b) 			tb_sint64_lsh(x, b)
#define tb_int64_rsh(x, b) 			tb_sint64_rsh(x, b)

#define tb_int64_add_int32(x, y) 	tb_sint64_add_sint32(x, y)
#define tb_int64_sub_int32(x, y) 	tb_sint64_sub_sint32(x, y)
#define tb_int64_mul_int32(x, y) 	tb_sint64_mul_sint32(x, y)
#define tb_int64_div_int32(x, y) 	tb_sint64_div_sint32(x, y)
#define tb_int64_mod_sint32(x, y) 	tb_sint64_mod_sint32(x, y)

#define tb_int64_or_sint32(x, y) 	tb_sint64_or_sint32(x, y)
#define tb_int64_and_sint32(x, y) 	tb_sint64_and_sint32(x, y)
#define tb_int64_xor_sint32(x, y) 	tb_sint64_xor_sint32(x, y)

#define tb_int64_nz(x) 				tb_sint64_nz(x)
#define tb_int64_ez(x) 				tb_sint64_ez(x)
#define tb_int64_gz(x) 				tb_sint64_gz(x)
#define tb_int64_lz(x) 				tb_sint64_lz(x)
#define tb_int64_et(x, y) 			tb_sint64_et(x, y)
#define tb_int64_nt(x, y) 			tb_sint64_nt(x, y)
#define tb_int64_lt(x, y) 			tb_sint64_lt(x, y)
#define tb_int64_gt(x, y) 			tb_sint64_gt(x, y)

#define tb_int64_et_int32(x, y)		tb_sint64_et_sint32(x, y)
#define tb_int64_nt_int32(x, y)		tb_sint64_nt_sint32(x, y)
#define tb_int64_lt_int32(x, y)		tb_sint64_lt_sint32(x, y)
#define tb_int64_gt_int32(x, y) 	tb_sint64_gt_sint32(x, y)

#endif

