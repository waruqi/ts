#ifndef TS_PSI_TABLE_PAT_H
#define TS_PSI_TABLE_PAT_H

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

// pat program
// if (program_number) pid is nit
// else pid is pmt
typedef struct __ts_table_pat_program_t
{
	ts_uint_t							program_number;		//!< program_number
	ts_uint_t							pid;				//!< pid of NIT/PMT

	struct __ts_table_pat_program_t* 	next;				//!< next element of the list

}ts_table_pat_program_t;

// pat
typedef struct __ts_table_pat_t
{
	// table header
	ts_table_header_t					table_header;	//!< \note table_id_extension is transport_stream_id

	// specific data
	ts_table_pat_program_t*				program_list;
	
}ts_table_pat_t;

// pat decoder
typedef struct __ts_table_pat_decoder_t
{
	// non-specific decoder data
	ts_table_decoder_t					decoder_header;

}ts_table_pat_decoder_t;

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */

// decoder operations
ts_table_pat_decoder_t*		ts_table_pat_create_decoder(ts_table_callback_t callback, void* cb_data);
void						ts_table_pat_destroy_decoder(ts_table_pat_decoder_t* tb_pat_decoder);

// dump pat info
void						ts_table_pat_dump(ts_packet_t* ts_packet, ts_table_pat_t* pat);

#	ifdef __cplusplus
}
#	endif

#endif

