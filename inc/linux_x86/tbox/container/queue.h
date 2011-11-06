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
 * \file		queue.h
 *
 */
#ifndef TB_CONTAINER_QUEUE_H
#define TB_CONTAINER_QUEUE_H

// c plus plus
#ifdef __cplusplus
extern "C" {
#endif

/* /////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"

/* /////////////////////////////////////////////////////////
 * types
 */

// the item func
typedef tb_void_t 	(*tb_queue_item_free_func_t)(tb_void_t* item, tb_void_t* priv);	

// the queue item func type
typedef struct __tb_queue_item_func_t
{
	// the item func
	tb_queue_item_free_func_t 	free;

	// the priv data
	tb_void_t* 					priv;

}tb_queue_item_func_t;

/* the queue type
 *
 * queue: |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||------|
 *       head                                                           last    tail
 *
 * queue: ||||||||||||||-----|--------------------------||||||||||||||||||||||||||
 *                   last  tail                       head                
 *
 * head: => the first item
 * last: => the last item
 * tail: => behind the last item, no item
 *
 * performance: 
 *
 * push: 	fast
 * pop: 	fast
 *
 * iterator:
 * next: 	fast
 * prev: 	fast
 *
 * \note the index of the same item is mutable
 *
 */
typedef struct __tb_queue_t
{
	// the data
	tb_byte_t* 				data;

	// the info
	tb_size_t 				step;
	tb_size_t 				head;
	tb_size_t 				tail;
	tb_size_t 				maxn;

	// the func
	tb_queue_item_func_t 	func;

}tb_queue_t;

/* /////////////////////////////////////////////////////////
 * interfaces
 */

// init & exit
tb_queue_t* 		tb_queue_init(tb_size_t step, tb_size_t maxn, tb_queue_item_func_t const* func);
tb_void_t 			tb_queue_exit(tb_queue_t* queue);

// accessors
tb_void_t* 			tb_queue_at_head(tb_queue_t* queue);
tb_void_t* 			tb_queue_at_last(tb_queue_t* queue);

tb_void_t const* 	tb_queue_const_at_head(tb_queue_t const* queue);
tb_void_t const* 	tb_queue_const_at_last(tb_queue_t const* queue);

// modifiors
tb_void_t 			tb_queue_clear(tb_queue_t* queue);

tb_void_t 	 		tb_queue_put(tb_queue_t* queue, tb_void_t const* item);
tb_void_t 	 		tb_queue_pop(tb_queue_t* queue, tb_void_t* item);

/* iterator
 * 
 * tb_size_t itor = tb_queue_itor_head(queue);
 * tb_size_t tail = tb_queue_itor_tail(queue);
 * for (; itor != tail; itor = tb_queue_itor_next(queue, itor))
 * {
 * 		tb_void_t const* item = tb_queue_itor_const_at(queue, itor);
 * 		if (item)
 * 		{
 * 			// ...
 * 		}
 * }
 *
 */
tb_void_t* 			tb_queue_itor_at(tb_queue_t* queue, tb_size_t itor);
tb_void_t const* 	tb_queue_itor_const_at(tb_queue_t const* queue, tb_size_t itor);

tb_size_t 			tb_queue_itor_head(tb_queue_t const* queue);
tb_size_t 			tb_queue_itor_tail(tb_queue_t const* queue);
tb_size_t 			tb_queue_itor_last(tb_queue_t const* queue);
tb_size_t 			tb_queue_itor_next(tb_queue_t const* queue, tb_size_t itor);
tb_size_t 			tb_queue_itor_prev(tb_queue_t const* queue, tb_size_t itor);

// attributes
tb_size_t 			tb_queue_size(tb_queue_t const* queue);
tb_size_t 			tb_queue_maxn(tb_queue_t const* queue);
tb_bool_t 			tb_queue_full(tb_queue_t const* queue);
tb_bool_t 			tb_queue_null(tb_queue_t const* queue);



// c plus plus
#ifdef __cplusplus
}
#endif

#endif

