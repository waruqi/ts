#ifndef TS_EPG_H
#define TS_EPG_H

// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "psi/psi.h"

/* ////////////////////////////////////////////////////////////////////////
 * macros
 */
#define TS_EPG_PROGRAM_VID_MAX_SIZE	(0x20)
#define TS_EPG_PROGRAM_AID_MAX_SIZE	(0x20)

/* ////////////////////////////////////////////////////////////////////////
 * types
 */

// epg program info
typedef struct __ts_epg_program_t
{
	tb_uint_t						pid_pmt;
	tb_uint_t						pid_pcr;
	tb_uint_t						pid_videos[TS_EPG_PROGRAM_VID_MAX_SIZE];
	tb_uint_t						pid_audios[TS_EPG_PROGRAM_AID_MAX_SIZE];
	tb_size_t						pid_videos_n;
	tb_size_t						pid_audios_n;

	tb_uint_t						program_number;
	tb_uint_t						service_id;

	tb_char_t						program_provider_name[TS_DESCRIPTOR_SERVICE_NAME_MAX_SIZE];
	tb_char_t						program_name[TS_DESCRIPTOR_SERVICE_NAME_MAX_SIZE];

	struct __ts_epg_program_t*		next;

}ts_epg_program_t;

// table decoder entry
typedef struct __ts_epg_tb_decoder_entry_t
{
	ts_table_decoder_t*						tb_decoder;
	struct __ts_epg_tb_decoder_entry_t*	next;

}ts_epg_tb_decoder_entry_t;

// epg decoder
typedef struct __ts_epg_decoder_t
{
	ts_decoder_t*					ts_decoder;			//!< ts packet decoder
	ts_epg_tb_decoder_entry_t*		tb_decoder_list;	//!< table decoder list

	// program list info
	ts_epg_program_t*				program_list;
	tb_uint_t						program_list_count;

}ts_epg_decoder_t;


/* ////////////////////////////////////////////////////////////////////////
 * user interfaces
 */
extern ts_epg_decoder_t*		ts_epg_create_decoder();
extern void						ts_epg_destroy_decoder(ts_epg_decoder_t* epg_decoder);
extern tb_bool_t				ts_epg_decode(ts_packet_t* ts_packet, ts_epg_decoder_t* epg_decoder);


// extern "C" {
#	ifdef __cplusplus
}
#	endif
#endif

