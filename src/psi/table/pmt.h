#ifndef TS_PSI_TABLE_PMT_H
#define TS_PSI_TABLE_PMT_H

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

// this structure is used to store a decoded es description.
// (ISO/IEC 13818-1 section 2.4.4.8).
typedef struct __ts_table_pmt_es_t
{
	tb_uint_t						stream_type;
	tb_uint_t						elementary_pid;
	ts_descriptor_t*				descriptor_list;
	struct __ts_table_pmt_es_t*		next;

}ts_table_pmt_es_t;

// pmt
typedef struct __ts_table_pmt_t
{
	// table header
	ts_table_header_t	table_header;

	// specific data
	tb_uint_t			pcr_pid;
	ts_descriptor_t*	descriptor_list;
	ts_table_pmt_es_t*	es_list;

}ts_table_pmt_t;

// pmt decoder
typedef struct __ts_table_pmt_decoder_t
{
	// non-specific decoder data
	ts_table_decoder_t	decoder_header;	//!< \note table_id_extension is program_number

	// for invalidae
	tb_uint_t			program_number;

}ts_table_pmt_decoder_t;

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */

// decoder operations
ts_table_pmt_decoder_t*	ts_table_pmt_create_decoder(tb_uint_t pid, tb_uint_t program_number, ts_table_callback_t callback, void* cb_data);
void					ts_table_pmt_destroy_decoder(ts_table_pmt_decoder_t* tb_pmt_decoder);

// dump pmt info
void					ts_table_pmt_dump(ts_packet_t* ts_packet, ts_table_pmt_t* pmt);

#	ifdef __cplusplus
}
#	endif

#endif

