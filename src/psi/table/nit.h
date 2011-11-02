#ifndef TS_PSI_TABLE_NIT_H
#define TS_PSI_TABLE_NIT_H

#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"

/* ////////////////////////////////////////////////////////////////////////
 * macros
 */

/* ////////////////////////////////////////////////////////////////////////
 * types
 */

// transport_stream description structure
typedef struct __ts_table_nit_ts_t
{
	ts_uint_t						transport_stream_id;
	ts_uint_t						original_network_id;
	ts_descriptor_t*				descriptor_list;

	struct __ts_table_nit_ts_t* 	next;

}ts_table_nit_ts_t;

// nit
typedef struct __ts_table_nit_t
{
	// table header
	ts_table_header_t			table_header;	//!< \note table_id_extension is network_id

	// specific data
	ts_descriptor_t*			descriptor_list;
	ts_table_nit_ts_t*			ts_list;

}ts_table_nit_t;

// nit decoder
typedef struct __ts_table_nit_decoder_t
{
	// non-specific decoder data
	ts_table_decoder_t			decoder_header;

}ts_table_nit_decoder_t;

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */

// decoder operations
ts_table_nit_decoder_t*		ts_table_nit_create_decoder(ts_table_callback_t callback, void* cb_data);
void						ts_table_nit_destroy_decoder(ts_table_nit_decoder_t* tb_nit_decoder);

// dump nit info
void						ts_table_nit_dump(ts_packet_t* ts_packet, ts_table_nit_t* nit);

#	ifdef __cplusplus
}
#	endif

#endif

