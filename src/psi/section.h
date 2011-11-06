#ifndef TS_PSI_SECTION_H
#define TS_PSI_SECTION_H

// c plus plus
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
// section max size
#define TS_SECTION_MAX_SIZE					(4 * 1024)

/* ////////////////////////////////////////////////////////////////////////
 * types
 */
/*!\brief psi section structure.
 *
 * This structure is used to store a PSI section. The common information are
 * decoded (ISO/IEC 13818-1 section 2.4.4.10).
 *
 * section_syntax_indicator == 0,
 * --------------------------------------
 * table_id;
 * section_syntax_indicator;
 * section_length;
 *								<= payload_start
 * table_id_extension;
 * version_number;
 * current_next_indicator;
 * section_number;
 * last_section_number;
 *								<= payload_end
 *
 *
 * section_syntax_indicator != 0,
 * --------------------------------------
 * table_id;
 * section_syntax_indicator;
 * section_length;
 * table_id_extension;
 * version_number;
 * current_next_indicator;
 * section_number;
 * last_section_number;
 *								<= payload_start
 * ...
 *								<= payload_end
 * crc
 */
typedef struct __ts_section_t
{
	// non-specific section data
	tb_uint_t					table_id;
	tb_uint_t					section_syntax_indicator;
	tb_uint_t					section_length;

	// used if section_syntax_indicator is true
	tb_uint_t					table_id_extension;
	tb_uint_t					version_number;
	tb_uint_t					current_next_indicator;
	tb_uint_t					section_number;
	tb_uint_t					last_section_number;

	// non-specific section data
	// the content is table-specific
	tb_byte_t*					data;				//!< complete section
	tb_byte_t*					payload_start;
	tb_byte_t*					payload_end;

	// used if section_syntax_indicator is true
	tb_uint32_t					crc;				//!< CRC_32

	// next section,
	// chain all sections in whole table for the convenience of free sections
	struct __ts_section_t*		next;

}ts_section_t;

// section callback function
typedef void (*ts_section_callback_t)(ts_packet_t* ts_packet, void* sc_decoder);

/*!\brief section decoder
 *
 * section:
 * packet0
 * packet1
 * ...
 * packetN
 */
typedef struct __ts_section_decoder_t
{
	// for decode section
	tb_uint_t					continuity_counter;	// counter counter a section table
	tb_bool_t					is_discontinuous;	// discontinuity
	ts_section_t*				current_section;	// current decoded section
	tb_size_t					processed_size;		// next processed size
	tb_bool_t					complete_header;	// whether header has been processed

	// inner data, not use
	ts_section_callback_t		callback;			// callback
	void*						cb_data;			// callback data

}ts_section_decoder_t;
/* ////////////////////////////////////////////////////////////////////////
 * inner interfaces
 */

// section operations
ts_section_t*			ts_section_create_section(tb_size_t max_size);
void					ts_section_destroy_section(ts_section_t* section);
tb_bool_t				ts_section_valid_section(ts_section_t* section);
void					ts_section_build_section(ts_section_t* section);

// decoder operations
void					ts_section_decoder_init(ts_section_decoder_t* sc_decoder);
void 					ts_section_decode(ts_packet_t* ts_packet, ts_section_decoder_t* sc_decoder);

// extern "C" {
#	ifdef __cplusplus
}
#	endif
#endif

