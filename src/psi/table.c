// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "table.h"

/* //////////////////////////////////////////////////////////////////
 * implemention
 */
// guess table id type
static ts_table_id_t ts_table_guess_table_id(tb_uint_t pid, tb_uint_t table_id)
{
	ts_table_entry_t* entries = g_tb_table_entries;
	while (entries->table_id_t != en_ts_table_end)
	{
		if (entries->from <= table_id && entries->to >= table_id) break;
		entries++;
	}

	// check
	ts_assert((pid == TS_PID_PAT) == (entries->table_id_t == en_ts_table_pat));

	return entries->table_id_t;
}
static tb_bool_t ts_table_decode_specific_table(ts_packet_t* ts_packet, ts_table_decoder_t* tb_decoder, ts_table_header_t* table)
{
	ts_assert(ts_packet && tb_decoder && table);
	if (!ts_packet || !tb_decoder || !table) return TB_FALSE;

	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_table_header_t*	table_header 	= table;

	// guess table type
	ts_table_id_t table_id_t = ts_table_guess_table_id(ts_header->pid, table_header->table_id);

	// decode specific table
	switch (table_id_t)
	{
		case en_ts_table_pat:	// decode pat
		{
			//ts_table_pat_dump(ts_packet, table);
		}
		break;
		case en_ts_table_pmt:	// decode pmt
		{
			//ts_table_pmt_dump(ts_packet, table);
		}
		break;
		default: break;
	}

	// specific table callback
	if (tb_decoder->callback) return tb_decoder->callback(tb_decoder, ts_packet, table, tb_decoder->cb_data);
	else return TB_TRUE;
}
// merge all pat sections
static void ts_table_merge_sections(ts_packet_t* ts_packet, ts_table_decoder_t* tb_decoder)
{
	ts_assert(ts_packet && tb_decoder);
	if (!ts_packet || !tb_decoder) return ;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);
	ts_section_t*		section 		= tb_decoder->sc_decoder.current_section;

	tb_bool_t is_append = TB_TRUE;
	tb_bool_t is_reinit = TB_FALSE;
	tb_int_t i;

	// check this section
	if (tb_decoder->table_check) is_append = tb_decoder->table_check(tb_decoder, section);

	// create current table if is NULL
	if (!tb_decoder->current_table && tb_decoder->table_create)
		tb_decoder->current_table = (ts_table_header_t*)tb_decoder->table_create(tb_decoder, NULL);

	// now if is_append is true then we have a valid pat section
	if (TB_TRUE == is_append)
	{
		// ts discontinuity check
		if (TB_TRUE == tb_decoder->sc_decoder.is_discontinuous)
		{
			is_reinit = TB_TRUE;
			tb_decoder->sc_decoder.is_discontinuous = TB_FALSE;
		}
		else
		{
			// perform a few sanity checks
			if (tb_decoder->building_table)
			{
				// check table_id_extension
				if (tb_decoder->building_table->table_id_extension != section->table_id_extension)
				{
					ts_trace("table_id_extension differs whereas no TS discontinuity has occured");
					is_reinit = TB_TRUE;
				}
				// check version_number
				else if (tb_decoder->building_table->version_number != section->version_number)
				{
					ts_trace("version_number differs whereas no discontinuity has occured");
					is_reinit = TB_TRUE;
				}
				// check last_section_number
				else if (tb_decoder->last_section_number != section->last_section_number)
				{
					ts_trace("'last_section_number differs whereas no discontinuity has occured");
					is_reinit = TB_TRUE;
				}
			}
			else
			{
				// if the pat with this version has been processed, then ignore it
				if ((TB_TRUE == tb_decoder->is_current_valid)
				&& (tb_decoder->current_table->version_number == section->version_number))
				{
					// signal a new pat if the previous one wasn't active
					if ((!tb_decoder->current_table->current_next_indicator)
					&& (section->current_next_indicator))
					{
						// decode specific table
						tb_decoder->current_table->current_next_indicator = 1;
						if (TB_FALSE == ts_table_decode_specific_table(ts_packet, tb_decoder, tb_decoder->current_table)) return ;
					}

					// don't decode since this version is already decoded
					is_append = TB_FALSE;
				}
			}
		}
	}

	// reinit the decoder if wanted
	if (TB_TRUE == is_reinit)
	{
		// force redecoding
		tb_decoder->is_current_valid = TB_FALSE;

		// free structures
		if (tb_decoder->building_table)
		{
			free(tb_decoder->building_table);
			tb_decoder->building_table = NULL;
		}

		// clear the section array
		for (i = 0; i < TS_TABLE_SECTIONS_MAX_COUNT; i++)
		{
			if (tb_decoder->sections[i])
			{
				ts_section_destroy_section(tb_decoder->sections[i]);
				tb_decoder->sections[i] = NULL;
			}
		}
	}

	// append the section to the list if wanted
	if (TB_TRUE == is_append)
	{
		tb_bool_t is_complete = TB_FALSE;

		// initialize the structures if it's the first section received
		if (!tb_decoder->building_table)
		{
			// create pat and init it
			if (tb_decoder->table_create) tb_decoder->building_table = (ts_table_header_t*)tb_decoder->table_create(tb_decoder, section);

			// gets last_section_number
			tb_decoder->last_section_number = section->last_section_number;
		}

		// fill the section array
		if (tb_decoder->sections[section->section_number])
		{
			ts_trace("overwrite section number %d", section->section_number);
			ts_section_destroy_section(tb_decoder->sections[section->section_number]);
		}
		tb_decoder->sections[section->section_number] = section;

		// check if we have all the sections
		for (i = 0; i <= tb_decoder->last_section_number; i++)
		{
			if (!tb_decoder->sections[i]) break;
			if (i == tb_decoder->last_section_number) is_complete = TB_TRUE;
		}

		// process a complete pat
		if (is_complete == TB_TRUE)
		{
			// save the current information
			*(tb_decoder->current_table) = *(tb_decoder->building_table);
			tb_decoder->is_current_valid = TB_TRUE;

			// chain the sections for process all sections
			if (tb_decoder->last_section_number)
			{
				for (i = 0; i <= tb_decoder->last_section_number - 1; i++)
					tb_decoder->sections[i]->next = tb_decoder->sections[i + 1];
			}

			// decode the whole table
			if (tb_decoder->table_decode) tb_decoder->table_decode(tb_decoder, tb_decoder->building_table, tb_decoder->sections[0]);

			// delete the all sections
			ts_section_destroy_section(tb_decoder->sections[0]);

			// reinitialize the sections
			for (i = 0; i <= tb_decoder->last_section_number; i++)
				tb_decoder->sections[i] = NULL;

			// decode specific table
			if (TB_FALSE == ts_table_decode_specific_table(ts_packet, tb_decoder, tb_decoder->building_table)) return;

			// destroy pat
			if (tb_decoder->table_destroy) tb_decoder->table_destroy(tb_decoder, (void*)tb_decoder->building_table);
			tb_decoder->building_table = NULL;
		}
	}
	else ts_section_destroy_section(section);
}

// section callback
static void ts_section_callback(ts_packet_t* ts_packet, void* sc_decoder)
{
	ts_assert(ts_packet && sc_decoder);
	if (!ts_packet || !sc_decoder) return ;

	// merge all sections and decode specific table
	ts_table_merge_sections(ts_packet, (ts_table_decoder_t*)sc_decoder);
}
/* //////////////////////////////////////////////////////////////////
 * interface implemention
 */
void ts_table_decoder_init(ts_table_decoder_t* tb_decoder, tb_uint_t pid_filter)
{
	ts_assert(tb_decoder);
	if (!tb_decoder) return ;

	// init section decoders
	ts_section_decoder_init(&(tb_decoder->sc_decoder));
	tb_decoder->sc_decoder.callback = &ts_section_callback;
	tb_decoder->sc_decoder.cb_data	= (void*)tb_decoder;

	// init pid filter
	tb_decoder->pid_filter = pid_filter;

	// init callback
	tb_decoder->callback	= NULL;
	tb_decoder->cb_data	= NULL;

	// sets initial state
	tb_decoder->is_current_valid = TB_FALSE;
	tb_decoder->building_table = NULL;
	tb_decoder->current_table = NULL;

	// init sections
	tb_uint_t i;
	for (i = 0; i < TS_TABLE_SECTIONS_MAX_COUNT; i++)
		tb_decoder->sections[i] = NULL;

	// init callbacks
	tb_decoder->table_create = NULL;
	tb_decoder->table_decode = NULL;
	tb_decoder->table_destroy = NULL;
}
void ts_table_decoder_exit(ts_table_decoder_t* tb_decoder)
{
	if (tb_decoder)
	{
		// destroy specific table data
		if (tb_decoder->current_table && tb_decoder->table_destroy)
			tb_decoder->table_destroy(tb_decoder, (void*)tb_decoder->current_table);

		// free structures
		if (tb_decoder->building_table)
		{
			free(tb_decoder->building_table);
			tb_decoder->building_table = NULL;
		}

		// clear the section array
		tb_int_t i;
		for (i = 0; i < TS_TABLE_SECTIONS_MAX_COUNT; i++)
		{
			if (tb_decoder->sections[i])
			{
				ts_section_destroy_section(tb_decoder->sections[i]);
				tb_decoder->sections[i] = NULL;
			}
		}

		// current decoder is invalid
		tb_decoder->is_current_valid = TB_FALSE;
	}
}
// decode table
tb_bool_t ts_table_decode(ts_packet_t* ts_packet, ts_table_decoder_t* tb_decoder)
{
	ts_assert(ts_packet);
	if (!ts_packet) return ;

	// filter pid
	if (tb_decoder->pid_filter != ts_packet->header.pid) return TB_FALSE;

	// decode sections
	ts_section_decode(ts_packet, &(tb_decoder->sc_decoder));

	return TB_TRUE;
}
// register callback
void ts_table_register_callback(ts_table_decoder_t* tb_decoder, ts_table_callback_t callback, void* cb_data)
{
	ts_assert(tb_decoder);
	if (!tb_decoder) return ;

	tb_decoder->callback = callback;
	tb_decoder->cb_data = cb_data;
}
// extern "C" {
#	ifdef __cplusplus
}
#	endif
