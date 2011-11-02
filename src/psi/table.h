#ifndef TS_PSI_TABLE_H
#define TS_PSI_TABLE_H

// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "section.h"

/* ////////////////////////////////////////////////////////////////////////
 * macros
 */
// section max size
#define TS_TABLE_MAX_SIZE					(4 * 1024)

// the maximum count of sections in a table
#define TS_TABLE_SECTIONS_MAX_COUNT			(256)
/* ////////////////////////////////////////////////////////////////////////
 * types
 */
// table type
typedef enum __ts_table_id_t
{
	en_ts_table_pat
,	en_ts_table_cat
,	en_ts_table_pmt
,	en_ts_table_tsdt
,	en_ts_table_14496_sct
,	en_ts_table_14496_oct
,	en_ts_table_mdt
,	en_ts_table_ipmp_cit
,	en_ts_table_dsmcc
,	en_ts_table_dsmcc_datagram
,	en_ts_table_nit
,	en_ts_table_sdt
,	en_ts_table_bat
,	en_ts_table_dsmcc_unt
,	en_ts_table_dsmcc_int
,	en_ts_table_eit
,	en_ts_table_tdt
,	en_ts_table_rst
,	en_ts_table_st
,	en_ts_table_tot
,	en_ts_table_mhp_ait
,	en_ts_table_tva_cs
,	en_ts_table_mpe_fec
,	en_ts_table_tva_rnt
,	en_ts_table_dit
,	en_ts_table_sit
,	en_ts_table_emm_ecm
,	en_ts_table_private

// end flag
,	en_ts_table_end

}ts_table_id_t;

// table header type
typedef struct __ts_table_header_t
{
	ts_uint_t				table_id;
	ts_uint_t				section_syntax_indicator;
	ts_uint_t				section_length;
	ts_uint_t				table_id_extension;		//!< transport_stream_id(pat, sdt), program_number(pmt), service_id(eit), network_id(nit), bouquet_id(bat)
	ts_uint_t				version_number;
	ts_uint_t				current_next_indicator;
	ts_uint_t				section_number;
	ts_uint_t				last_section_number;

	ts_uint32_t				crc;

}ts_table_header_t;

// table callback type
typedef ts_bool_t			(*ts_table_callback_t)(void* tb_decoder, ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data);

/*!\brief table decoder
 *
 * table:
 * section0
 * section1
 * ...
 * sectionN //!< N < 256
 */
typedef struct __ts_table_decoder_t
{
	// section decoder
	ts_section_decoder_t	sc_decoder;		//!< \note must be at first position for getting tb_decoder from it

	// pid filter
	ts_uint_t				pid_filter;

	// table decoder type
	ts_table_id_t			table_id_t;

	// callback
	ts_table_callback_t		callback;
	void*					cb_data;

	// for decoding table with N sections
	ts_table_header_t*		current_table;
	ts_table_header_t*		building_table;
	ts_bool_t				is_current_valid;
	ts_uint_t				last_section_number;
	ts_section_t*			sections[TS_TABLE_SECTIONS_MAX_COUNT];

	// speciic decoder functions
	void* 					(*table_create)(void* decoder, ts_section_t* sections);
	void					(*table_destroy)(void* decoder, void* table);
	void					(*table_decode)(void* decoder, void* table, ts_section_t* sections);
	ts_bool_t 				(*table_check)(void* decoder, ts_section_t* section);

}ts_table_decoder_t;

// table entry type
typedef struct __ts_table_entry_t
{
	ts_uint_t			from;		// from id 1
	ts_uint_t			to;			// to   id 3
	ts_table_id_t		table_id_t;	// table type

}ts_table_entry_t;

/* ////////////////////////////////////////////////////////////////////////
 * global variables
 */
// for guess table type
static ts_table_entry_t g_tb_table_entries[] =
{
	{  0x00, 0x00,  en_ts_table_pat	}
,	{  0x01, 0x01,  en_ts_table_cat	}
,	{  0x02, 0x02,  en_ts_table_pmt	}
,	{  0x03, 0x03,  en_ts_table_tsdt }
,	{  0x04, 0x04,  en_ts_table_14496_sct }
,	{  0x05, 0x05,  en_ts_table_14496_oct }
,	{  0x06, 0x06,  en_ts_table_mdt }			// metadata section
,	{  0x07, 0x07,  en_ts_table_ipmp_cit }		// ipmp_control_information_section (defined in iso/iec13818-11)
	// res.
,	{  0x3a, 0x3d,  en_ts_table_dsmcc	}
,	{  0x3e, 0x3e,  en_ts_table_dsmcc_datagram }
,	{  0x40, 0x41,  en_ts_table_nit	}
,	{  0x42, 0x42,  en_ts_table_sdt	}
	// res.
,	{  0x46, 0x46,  en_ts_table_sdt	}
	// res.
,	{  0x4a, 0x4a,  en_ts_table_bat	}
,	{  0x4b, 0x4b,  en_ts_table_dsmcc_unt }
,	{  0x4c, 0x4c,  en_ts_table_dsmcc_int }
,	{  0x4e, 0x6e,  en_ts_table_eit	}			//  4 different types
,	{  0x70, 0x70,  en_ts_table_tdt }
,	{  0x71, 0x71,  en_ts_table_rst }
,	{  0x72, 0x72,  en_ts_table_st  }
,	{  0x73, 0x73,  en_ts_table_tot }
,	{  0x74, 0x74,  en_ts_table_mhp_ait }

,	{  0x75, 0x75,  en_ts_table_tva_cs }		// ts 102 323

	// 0x76 related content section (ts 102 323 [36])
	// 0x77 content identifier section (ts 102 323 [36])  (cit)

,	{  0x78, 0x78,  en_ts_table_mpe_fec }		// en 301 192 v1.4.1
,	{  0x79, 0x79,  en_ts_table_tva_rnt }		// ts 102 323

	// res.
,	{  0x7e, 0x7e,  en_ts_table_dit }
,	{  0x7f, 0x7f,  en_ts_table_sit }
,	{  0x80, 0x8f,  en_ts_table_emm_ecm }		// $$$ conditional access message section

,	{  0x90, 0xfe,  en_ts_table_private }		// opps!? dsm-cc or other stuff?!
,	{  0, 0, 		en_ts_table_end }
};

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */

void					ts_table_decoder_init(ts_table_decoder_t* tb_decoder, ts_uint_t pid_filter);
void					ts_table_decoder_exit(ts_table_decoder_t* tb_decoder);

ts_bool_t 				ts_table_decode(ts_packet_t* ts_packet, ts_table_decoder_t* tb_decoder);
void 					ts_table_register_callback(ts_table_decoder_t* tb_decoder, ts_table_callback_t callback, void* cb_data);

// extern "C" {
#	ifdef __cplusplus
}
#	endif
#endif

