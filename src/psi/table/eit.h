#ifndef TS_PSI_TABLE_EIT_H
#define TS_PSI_TABLE_EIT_H

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

// eit service description structure
typedef struct __ts_table_eit_event_t
{
	tb_uint_t						event_id;
	tb_uint_t						start_time_mjd;	//!< start_time: yyyy-mm-dd
	tb_uint_t						start_time_utc;	//!< start_time: hh:mm:ss
	tb_uint_t						duration;
	tb_uint_t						running_status;
	tb_uint_t						free_ca_mode;
	ts_descriptor_t*				descriptor_list;

	struct __ts_table_eit_event_t* 	next;

}ts_table_eit_event_t;

// eit
typedef struct __ts_table_eit_t
{
	// table header
	ts_table_header_t		table_header;	//!< \note table_id_extension is service_id

	// specific data
	tb_uint_t				transport_stream_id;
	tb_uint_t				original_network_id;
	tb_uint_t				segment_last_section_number;
	tb_uint_t				last_table_id;
	ts_table_eit_event_t*	event_list;

}ts_table_eit_t;

// eit decoder
typedef struct __ts_table_eit_decoder_t
{
	// non-specific decoder data
	ts_table_decoder_t		decoder_header;

}ts_table_eit_decoder_t;

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */

// decoder operations
ts_table_eit_decoder_t*		ts_table_eit_create_decoder(ts_table_callback_t callback, void* cb_data);
void						ts_table_eit_destroy_decoder(ts_table_eit_decoder_t* tb_eit_decoder);

// dump eit info
void						ts_table_eit_dump(ts_packet_t* ts_packet, ts_table_eit_t* eit);

#	ifdef __cplusplus
}
#	endif

#endif

