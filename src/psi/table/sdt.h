#ifndef TS_PSI_TABLE_SDT_H
#define TS_PSI_TABLE_SDT_H

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

// sdt service description structure
typedef struct __ts_table_sdt_service_t
{
	tb_uint_t							service_id;
	tb_uint_t							eit_schedule_flag;
	tb_uint_t							eit_present_following_flag;
	tb_uint_t							running_status;
	tb_uint_t							free_ca_mode;
	ts_descriptor_t*					descriptor_list;

	struct __ts_table_sdt_service_t* 	next;

}ts_table_sdt_service_t;

// sdt
typedef struct __ts_table_sdt_t
{
	// table header
	ts_table_header_t					table_header;	//!< \note table_id_extension is transport_stream_id

	// specific data
	tb_uint_t							original_network_id;
	ts_table_sdt_service_t*				service_list;

}ts_table_sdt_t;

// sdt decoder
typedef struct __ts_table_sdt_decoder_t
{
	// non-specific decoder data
	ts_table_decoder_t					decoder_header;

}ts_table_sdt_decoder_t;

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */

// decoder operations
ts_table_sdt_decoder_t*		ts_table_sdt_create_decoder(ts_table_callback_t callback, void* cb_data);
void						ts_table_sdt_destroy_decoder(ts_table_sdt_decoder_t* tb_sdt_decoder);

// dump sdt info
void						ts_table_sdt_dump(ts_packet_t* ts_packet, ts_table_sdt_t* sdt);

#	ifdef __cplusplus
}
#	endif

#endif

