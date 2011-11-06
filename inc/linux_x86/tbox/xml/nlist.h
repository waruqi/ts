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
 * \file		nlist.h
 *
 */
#ifndef TB_XML_NLIST_H
#define TB_XML_NLIST_H

// c plus plus
#ifdef __cplusplus
extern "C" {
#endif

/* /////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "node.h"

/* /////////////////////////////////////////////////////////
 * types
 */

// the xml nlist type
typedef struct __tb_xml_nlist_t
{
	tb_xml_node_t 	base;
	tb_size_t 		size;

}tb_xml_nlist_t;


/* /////////////////////////////////////////////////////////
 * interfaces
 */

// create & destroy
tb_xml_nlist_t* tb_xml_nlist_create();
tb_void_t 			tb_xml_nlist_destroy(tb_xml_nlist_t* nlist);

// accessors
tb_xml_node_t* 	tb_xml_nlist_at(tb_xml_nlist_t* nlist, tb_int_t index);
tb_xml_node_t* 	tb_xml_nlist_get(tb_xml_nlist_t* nlist, tb_char_t const* name);

// modifiors
tb_void_t 			tb_xml_nlist_add(tb_xml_nlist_t* nlist, tb_xml_node_t* node);
tb_void_t 			tb_xml_nlist_det(tb_xml_nlist_t* nlist, tb_xml_node_t* node);

// c plus plus
#ifdef __cplusplus
}
#endif

#endif
