// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "pmt.h"

/* //////////////////////////////////////////////////////////////////
 * types
 */

// es stream type
typedef struct __ts_table_pmt_es_stream_t
{
	ts_char_t const*	str;

}ts_table_pmt_es_stream_t;

/* //////////////////////////////////////////////////////////////////
 * global variables
 */

// es stream type list, ISO/IEC 13818-1 : 2000 (E)
static ts_table_pmt_es_stream_t g_ts_table_pmt_es_stream_list[] =
{
	{	"ITU-T | ISO/IEC Reserved"																			}	// 0x00
,	{	"ISO/IEC 11172 Video"																				}	// 0x01
,	{	"ITU-T Rec. H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream"	}	// 0x02
,	{	"ISO/IEC 11172 Audio"																				}	// 0x03
,	{	"ISO/IEC 13818-3 Audio"																				}	// 0x04
,	{	"ITU-T Rec. H.222.0 | ISO/IEC 13818-1 private_sections"												}	// 0x05
,	{	"ITU-T Rec. H.222.0 | ISO/IEC 13818-1 PES packets containing private data"							}	// 0x06
,	{	"ISO/IEC 13522 MHEG"																				}	// 0x07
,	{	"ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Annex A DSM-CC"												}	// 0x08
,	{	"ITU-T Rec. H.222.1"																				}	// 0x09
,	{	"ISO/IEC 13818-6 type A"																			}	// 0x0a
,	{	"ISO/IEC 13818-6 type B"																			}	// 0x0b
,	{	"ISO/IEC 13818-6 type C"																			}	// 0x0c
,	{	"ISO/IEC 13818-6 type D"																			}	// 0x0d
,	{	"ITU-T Rec. H.222.0 | ISO/IEC 13818-1 auxiliary"													}	// 0x0e
,	{	"ISO/IEC 13818-7 Audio with ADTS transport syntax"													}	// 0x0f
,	{	"ISO/IEC 14496-2 Visual"																			}	// 0x10
,	{	"ISO/IEC 14496-3 Audio with the LATM transport syntax as defined in ISO/IEC 14496-3 / AMD 1"		}	// 0x11
,	{	"ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in PES packets"						}	// 0x12
,	{	"ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in ISO/IEC14496_sections."			}	// 0x13
,	{	"ISO/IEC 13818-6 Synchronized Download Protocol"													}	// 0x14
,	{	"ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Reserved"														}	// 0x15-0x7f
,	{	"User Private"																						}	// 0x80-0xff
};
/* //////////////////////////////////////////////////////////////////
 * inner implemention
 */
// guess es stream types
static ts_table_pmt_es_stream_t* ts_table_pmt_guess_es_stream(ts_uint_t stream_type)
{
	if (stream_type < 0x15) return &g_ts_table_pmt_es_stream_list[stream_type];
	else if (stream_type >= 0x15 && stream_type < 0x80) return &g_ts_table_pmt_es_stream_list[0x15];
	else return &g_ts_table_pmt_es_stream_list[0x15 + 1];
}
// add a descriptor in the pmt.
static ts_descriptor_t* ts_table_pmt_add_descriptor(ts_table_pmt_t* pmt, ts_uint_t descriptor_tag, ts_uint_t descriptor_length, ts_byte_t* data)
{
	// create a descriptor
	ts_descriptor_t* descriptor = ts_descriptor_create_descriptor(descriptor_tag, descriptor_length, data);
	if (descriptor)
	{
		if (!pmt->descriptor_list) pmt->descriptor_list = descriptor;
		else
		{
			// apped descriptor
			ts_descriptor_t* last_descriptor = pmt->descriptor_list;
			while (last_descriptor->next) last_descriptor = last_descriptor->next;
			last_descriptor->next = descriptor;
		}
	}
	return descriptor;
}

// add es to pmt
static ts_table_pmt_es_t* ts_table_pmt_add_es(ts_table_pmt_t* pmt, ts_uint_t stream_type, ts_uint_t elementary_pid)
{
	// allocate a es
	ts_table_pmt_es_t* es = (ts_table_pmt_es_t*)malloc(sizeof(ts_table_pmt_es_t));
	if (es)
	{
		// init header
		es->stream_type		= stream_type;
		es->elementary_pid	= elementary_pid;

		// init other
		es->descriptor_list	= NULL;
		es->next			= NULL;

		// append es
		if (!pmt->es_list) pmt->es_list = es;
		else
		{
			ts_table_pmt_es_t* last_es = pmt->es_list;
			while (last_es->next) last_es = last_es->next;
			last_es->next = es;
		}
	}

	return es;
}
// add a descriptor in the pmt es.
ts_descriptor_t* ts_table_pmt_es_add_descriptor(ts_table_pmt_es_t* es, ts_uint_t descriptor_tag, ts_uint_t descriptor_length, ts_byte_t* data)
{
	// create a descriptor
	ts_descriptor_t* descriptor = ts_descriptor_create_descriptor(descriptor_tag, descriptor_length, data);
	if (descriptor)
	{
		if (!es->descriptor_list) es->descriptor_list = descriptor;
		else
		{
			// apped descriptor
			ts_descriptor_t* last_descriptor = es->descriptor_list;
			while (last_descriptor->next) last_descriptor = last_descriptor->next;
			last_descriptor->next = descriptor;
		}
	}
	return descriptor;
}

// create and init a new pmt
static void* ts_table_pmt_create(void* decoder, ts_section_t* sections)
{
	ts_table_pmt_t* pmt = (ts_table_pmt_t*)malloc(sizeof(ts_table_pmt_t));
	ts_table_header_t* header = (ts_table_header_t*)pmt;
	TS_ASSERT(pmt);

	// init header
	ts_section_t* section = sections;
	if (section)
	{
		header->table_id					= section->table_id;
		header->section_syntax_indicator	= section->section_syntax_indicator;
		header->section_length				= section->section_length;
		header->table_id_extension			= section->table_id_extension;
		header->version_number				= section->version_number;
		header->current_next_indicator		= section->current_next_indicator;
		header->section_number				= section->section_number;
		header->last_section_number			= section->last_section_number;
		header->crc							= section->crc;

		pmt->pcr_pid						= ts_get_bits(section->payload_start, 0, 3, 13);

	}

	// need analyze
	pmt->descriptor_list					= NULL;
	pmt->es_list							= NULL;

	return (void*)pmt;
}
// empty the all programs
static void ts_table_pmt_empty(void* decoder, void* table)
{
	TS_ASSERT(table);
	if (!table) return ;
	ts_table_pmt_t* pmt = (ts_table_pmt_t*)table;
	ts_table_pmt_es_t* es = pmt->es_list;

	// destroy pmt descriptor list
	ts_descriptor_destroy_descriptors(pmt->descriptor_list);

	// destroy es list
	while(es)
	{
		ts_table_pmt_es_t* tmp = es->next;

		// destroy pmt es descriptor list
		ts_descriptor_destroy_descriptors(es->descriptor_list);
		free(es);
		es = tmp;
	}

	pmt->descriptor_list = NULL;
	pmt->es_list = NULL;
}
// destroy pmt
static void ts_table_pmt_destroy(void* decoder, void* table)
{
	TS_ASSERT(table);
	if (!table) return ;
	ts_table_pmt_t* pmt = (ts_table_pmt_t*)table;

	ts_table_pmt_empty(decoder, pmt);
	free(pmt);
}
// decode pmt
static void ts_table_pmt_decode(void* decoder, void* table, ts_section_t* sections)
{
	TS_ASSERT(table && sections);
	if (!table || !sections) return ;
	ts_table_pmt_t* pmt = (ts_table_pmt_t*)table;
	ts_section_t* section = sections;

	ts_byte_t*		p	= NULL;
	ts_byte_t*		end = NULL;

	while (section)
	{
		// point to descriptor_list
		p = section->payload_start + 4;

		// end: p + program_info_length
		end = p + ts_get_bits(section->payload_start, 0, 20, 12);

		// add all descriptors to pmt
		while (p + 2 <= end)
		{
			// decode header
			ts_uint_t descriptor_tag		= p[0];
			ts_uint_t descriptor_length	= p[1];

			// add it
			if (descriptor_length + 2 <= end - p) ts_table_pmt_add_descriptor(pmt, descriptor_tag, descriptor_length, p + 2);
			p += 2 + descriptor_length;
		}

		// es list
		for (p = end; p + 5 <= section->payload_end;)
		{
			// decode header
			ts_uint_t stream_type		= p[0];
			ts_uint_t elementary_pid	= ts_get_bits(p, 0, 11, 13);
			ts_uint_t es_info_length	= ts_get_bits(p, 0, 28, 12);

			// add es to pmt
			ts_table_pmt_es_t* es = ts_table_pmt_add_es(pmt, stream_type, elementary_pid);

			// - es descriptors
			p += 5;
			end = p + es_info_length;
			if (end > section->payload_end) end = section->payload_end;
			while (p + 2 <= end)
			{
				ts_uint_t descriptor_tag		= p[0];
				ts_uint_t descriptor_length	= p[1];

				if (descriptor_length + 2 <= end - p) ts_table_pmt_es_add_descriptor(es, descriptor_tag, descriptor_length, p + 2);
				p += 2 + descriptor_length;
			}
		}
		section = section->next;
	}
}
// check section
static ts_bool_t ts_table_pmt_check(void* decoder, ts_section_t* section)
{
	TS_ASSERT(decoder && section);
	if (!decoder || !section) return TS_FALSE;
	ts_table_pmt_decoder_t* tb_pmt_decoder = (ts_table_pmt_decoder_t*)decoder;

	// table_id must be 0x02 if is pmt
	if (section->table_id != 0x02)
	{
		// invalid table_id value
		TS_DBG("invalid section (table_id == 0x%02x)", section->table_id);
		return TS_FALSE;
	}

	// check section_syntax_indicator: must be 1
	if (!section->section_syntax_indicator)
	{
		// invalid section_syntax_indicator
		TS_DBG("invalid section (section_syntax_indicator == 0)");
		return TS_FALSE;
	}

	// pmt section is valid if program_number == table_id_extension(program_number)
	if (tb_pmt_decoder->program_number != section->table_id_extension)
	{
		// invalid section_syntax_indicator
		TS_DBG("invalid section: 'program_number' don't match(pmt:%u section:%u)", tb_pmt_decoder->program_number, section->table_id_extension);
		return TS_FALSE;
	}

	return TS_TRUE;
}
/* //////////////////////////////////////////////////////////////////
 * interface implemention
 */
// create pmt decoder
ts_table_pmt_decoder_t* ts_table_pmt_create_decoder(ts_uint_t pid, ts_uint_t program_number, ts_table_callback_t callback, void* cb_data)
{
	// allocate decoder
	ts_table_pmt_decoder_t* tb_pmt_decoder = (ts_table_pmt_decoder_t*)malloc(sizeof(ts_table_pmt_decoder_t));

	// init table decoder
	ts_table_decoder_t* tb_decoder = (ts_table_decoder_t*)tb_pmt_decoder;
	ts_table_decoder_init(tb_decoder, pid);
	ts_table_register_callback(tb_decoder, callback, cb_data);

	// init callbacks
	tb_decoder->table_create	= &ts_table_pmt_create;
	tb_decoder->table_decode	= &ts_table_pmt_decode;
	tb_decoder->table_destroy	= &ts_table_pmt_destroy;
	tb_decoder->table_check		= &ts_table_pmt_check;

	// init specific data
	tb_pmt_decoder->program_number	= program_number;

	// init decoder type
	tb_decoder->table_id_t		= en_ts_table_pmt;

	return tb_pmt_decoder;
}
void ts_table_pmt_destroy_decoder(ts_table_pmt_decoder_t* tb_pmt_decoder)
{
	if (tb_pmt_decoder)
	{
		ts_table_decoder_exit((ts_table_decoder_t*)tb_pmt_decoder);
		free(tb_pmt_decoder);
	}
}
// dump pmt info
void ts_table_pmt_dump(ts_packet_t* ts_packet, ts_table_pmt_t* pmt)
{
	TS_ASSERT(pmt && ts_packet);
	if (!pmt || !ts_packet) return ;

	ts_table_header_t* header = (ts_table_header_t*)pmt;
	TS_PRINT("----------------------------------------"									);
	TS_PRINT("pmt(pid:%x)",					ts_packet->header.pid						);
	TS_PRINT("----------------------------------------"									);
	TS_PRINT("table_id:%x",					header->table_id							);
	TS_PRINT("section_syntax_indicator:%u",	header->section_syntax_indicator			);
	TS_PRINT("section_length:%u",				header->section_length						);
	TS_PRINT("program_number:%u",				header->table_id_extension					);
	TS_PRINT("version_number:%u",				header->version_number						);
	TS_PRINT("current_next_indicator:%u",		header->current_next_indicator				);
	TS_PRINT("section_number:%u",				header->section_number						);
	TS_PRINT("last_section_number:%u",			header->last_section_number					);
	TS_PRINT("crc:%x",							header->crc									);
	TS_PRINT("pcr_pid:%x",						pmt->pcr_pid								);

	ts_descriptor_t* descriptor = pmt->descriptor_list;
	while (descriptor)
	{
		TS_PRINT("\tdescriptor_tag:%u",		descriptor->descriptor_tag					);
		TS_PRINT("\tdescriptor_length:%x",		descriptor->descriptor_length				);

		descriptor = descriptor->next;
	}

	// destroy es list
	ts_table_pmt_es_t* es = pmt->es_list;
	while(es)
	{
		TS_PRINT("\tes stream_type:%u => %s",		es->stream_type, ts_table_pmt_guess_es_stream(es->stream_type)->str);
		TS_PRINT("\tes elementary_pid:%x\n",		es->elementary_pid						);

		descriptor = es->descriptor_list;
		while (descriptor)
		{
			TS_PRINT("\t\tdescriptor_tag:%x",		descriptor->descriptor_tag				);
			TS_PRINT("\t\tdescriptor_length:%u\n",	descriptor->descriptor_length			);

			descriptor = descriptor->next;
		}

		es = es->next;
	}

	TS_PRINT("========================================\n"										);
}

// extern "C" {
#	ifdef __cplusplus
}
#	endif
