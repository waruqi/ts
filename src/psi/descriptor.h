#ifndef TS_PSI_DESCRIPTOR_H
#define TS_PSI_DESCRIPTOR_H

#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"

/* ////////////////////////////////////////////////////////////////////////
 * types
 */

/* descriptior type
 *
 * descriptor_tag
 * descriptor_length
 * ...					<= data(descriptor_length)
 *
 * 
 */
typedef struct __ts_descriptor_t
{
	// descriptor header
	tb_uint_t						descriptor_tag;
	tb_uint_t						descriptor_length;

	// data
	tb_byte_t*						data;

	// next
	struct __ts_descriptor_t*		next;
	void*							decoded_dr;		//!< for freeing specific decoded descriptor

}ts_descriptor_t;

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */
ts_descriptor_t*		ts_descriptor_create_descriptor(tb_byte_t descriptor_tag, tb_byte_t descriptor_length, tb_byte_t* data);
void					ts_descriptor_destroy_descriptors(ts_descriptor_t* descriptor_list);

#	ifdef __cplusplus
}
#	endif

#endif

