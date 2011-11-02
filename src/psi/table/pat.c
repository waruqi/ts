// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "pat.h"

/* //////////////////////////////////////////////////////////////////
 * macros
 */

/* //////////////////////////////////////////////////////////////////
 * inner implemention
 */

// add a program at the end of the pat.
static ts_table_pat_program_t* ts_table_pat_add_program(ts_table_pat_t* pat, ts_uint_t program_number, ts_uint_t pid)
{
	TS_ASSERT(pat);
	if (!pat) return ;

	// create a new program
	ts_table_pat_program_t* program = (ts_table_pat_program_t*)malloc(sizeof(ts_table_pat_program_t));
	if (program)
	{
		// init the program
		program->program_number = program_number;
		program->pid			= pid;
		program->next			= NULL;

		// append it to the end of program list
		if (!pat->program_list) pat->program_list = program;
		else
		{
			ts_table_pat_program_t* last_program = pat->program_list;
			while (last_program->next) last_program = last_program->next;
			last_program->next = program;
		}
	}

	return program;
}

// create and init a new pat
static void* ts_table_pat_create(void* decoder, ts_section_t* sections)
{
	ts_table_pat_t* pat = (ts_table_pat_t*)malloc(sizeof(ts_table_pat_t));
	ts_table_header_t* header = (ts_table_header_t*)pat;
	TS_ASSERT(pat);

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
	}

	// need analyze
	pat->program_list				= NULL;

	return (void*)pat;
}
// empty the all programs
static void ts_table_pat_empty(void* decoder, void* table)
{
	TS_ASSERT(table);
	if (!table) return ;
	ts_table_pat_t* pat = (ts_table_pat_t*)table;

	ts_table_pat_program_t* program = pat->program_list;
	while (program)
	{
		ts_table_pat_program_t* tmp = program->next;
		free(program);
		program = tmp;
	}
	pat->program_list = NULL;
}
// destroy pat
static void ts_table_pat_destroy(void* decoder, void* table)
{
	TS_ASSERT(table);
	if (!table) return ;
	ts_table_pat_t* pat = (ts_table_pat_t*)table;

	ts_table_pat_empty(decoder, pat);
	free(pat);
}
// decode pat
static void ts_table_pat_decode(void* decoder, void* table, ts_section_t* sections)
{
	TS_ASSERT(table && sections);
	if (!table || !sections) return ;
	ts_table_pat_t* pat = (ts_table_pat_t*)table;
	ts_section_t* section = sections;

	ts_byte_t* p;
	while (section)
	{
		// process all programs in this section
		for (p = section->payload_start; p < section->payload_end; p += 4)
		{
			ts_uint_t program_number	= ts_get_bits(p, 0, 0, 16);
			ts_uint_t pid				= ts_get_bits(p, 0, 19, 13);

			// add program to the program list of the pat
			ts_table_pat_add_program(pat, program_number, pid);
		}

		// process the next section in the pat
		section = section->next;
	}
}
// check section
static ts_bool_t ts_table_pat_check(void* decoder, ts_section_t* section)
{
	TS_ASSERT(decoder && section);
	if (!decoder || !section) return TS_FALSE;
	if (!section) return TS_FALSE;

	// table_id must be 0x00 if is pat
	if (section->table_id != 0x00)
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

	return TS_TRUE;
}

/* //////////////////////////////////////////////////////////////////
 * interface implemention
 */
// create pat decoder
ts_table_pat_decoder_t* ts_table_pat_create_decoder(ts_table_callback_t callback, void* cb_data)
{
	// allocate decoder
	ts_table_pat_decoder_t* tb_pat_decoder = (ts_table_pat_decoder_t*)malloc(sizeof(ts_table_pat_decoder_t));

	// init table decoder
	ts_table_decoder_t* tb_decoder = (ts_table_decoder_t*)tb_pat_decoder;
	ts_table_decoder_init(tb_decoder, TS_PID_PAT);
	ts_table_register_callback(tb_decoder, callback, cb_data);

	// init callbacks
	tb_decoder->table_create	= &ts_table_pat_create;
	tb_decoder->table_decode	= &ts_table_pat_decode;
	tb_decoder->table_destroy	= &ts_table_pat_destroy;
	tb_decoder->table_check		= &ts_table_pat_check;

	// init decoder type
	tb_decoder->table_id_t		= en_ts_table_pat;

	return tb_pat_decoder;
}
void ts_table_pat_destroy_decoder(ts_table_pat_decoder_t* tb_pat_decoder)
{
	if (tb_pat_decoder)
	{
		ts_table_decoder_exit((ts_table_decoder_t*)tb_pat_decoder);
		free(tb_pat_decoder);
	}
}
// dump pat info
void ts_table_pat_dump(ts_packet_t* ts_packet, ts_table_pat_t* pat)
{
	TS_ASSERT(pat && ts_packet);
	if (!pat || !ts_packet) return ;

	ts_table_header_t* header = (ts_table_header_t*)pat;
	TS_PRINT("----------------------------------------"									);
	TS_PRINT("pat(pid:%x)",					ts_packet->header.pid						);
	TS_PRINT("----------------------------------------"									);
	TS_PRINT("table_id:%x",					header->table_id							);
	TS_PRINT("section_syntax_indicator:%u",	header->section_syntax_indicator			);
	TS_PRINT("section_length:%u",				header->section_length						);
	TS_PRINT("transport_stream_id:%u",			header->table_id_extension					);
	TS_PRINT("version_number:%u",				header->version_number						);
	TS_PRINT("current_next_indicator:%u",		header->current_next_indicator				);
	TS_PRINT("section_number:%u",				header->section_number						);
	TS_PRINT("last_section_number:%u",			header->last_section_number					);
	TS_PRINT("crc:%x",							header->crc									);

	ts_table_pat_program_t* program = pat->program_list;
	while (program)
	{
		TS_PRINT("\tprogram_number:%u",		program->program_number);
		TS_PRINT("\t%s pid:%x",				program->program_number == 0? "nit" : "pmt", program->pid);

		program = program->next;
	}
	TS_PRINT("========================================\n"										);
}

// extern "C" {
#	ifdef __cplusplus
}
#	endif
