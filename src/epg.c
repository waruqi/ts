// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "epg.h"

/* ////////////////////////////////////////////////////////////////////////
 * program list operations
 */
// add a program
static ts_epg_program_t* ts_epg_program_create(ts_epg_decoder_t* epg_decoder)
{
	TS_ASSERT(epg_decoder);
	if (!epg_decoder) return NULL;

	// create a new program
	ts_epg_program_t* program = (ts_epg_program_t*)malloc(sizeof(ts_epg_program_t));
	if (program)
	{
		// init the program
		program->next				= NULL;

		// append it to the end of program list
		if (!epg_decoder->program_list) epg_decoder->program_list = program;
		else
		{
			ts_epg_program_t* last_program = epg_decoder->program_list;
			while (last_program->next) last_program = last_program->next;
			last_program->next = program;
		}
	}

	return program;
}
// destroy the all programs
static void ts_epg_program_destroy_all(ts_epg_decoder_t* epg_decoder)
{
	TS_ASSERT(epg_decoder);
	if (!epg_decoder) return ;

	ts_epg_program_t* program = epg_decoder->program_list;
	while (program)
	{
		ts_epg_program_t* tmp = program->next;
		free(program);
		program = tmp;
	}
	epg_decoder->program_list = NULL;
}

// find the program using service_id
static ts_epg_program_t* ts_epg_program_find(ts_epg_decoder_t* epg_decoder, ts_uint_t service_id)
{
	TS_ASSERT(epg_decoder);
	if (!epg_decoder) return NULL;

	ts_epg_program_t* program = epg_decoder->program_list;
	while (program)
	{
		if (program->program_number == service_id) return program;
		program = program->next;
	}
	return NULL;
}
/* ////////////////////////////////////////////////////////////////////////
 * table decoder list operations
 */
static ts_epg_tb_decoder_entry_t* ts_epg_tb_decoder_list_add(ts_epg_decoder_t* epg_decoder, ts_table_decoder_t* tb_decoder)
{
	TS_ASSERT(epg_decoder);
	if (!epg_decoder) return NULL;

	// create a new decoder entry
	ts_epg_tb_decoder_entry_t* entry = (ts_epg_tb_decoder_entry_t*)malloc(sizeof(ts_epg_tb_decoder_entry_t));
	if (entry)
	{
		// init the decoder entry and append it to the header of decoder list
		entry->tb_decoder				= tb_decoder;
		//entry->next						= epg_decoder->tb_decoder_list;
		//epg_decoder->tb_decoder_list	= entry;

		entry->next = NULL;
		if (!epg_decoder->tb_decoder_list) epg_decoder->tb_decoder_list = entry;
		else
		{
			entry->next						= epg_decoder->tb_decoder_list;
			epg_decoder->tb_decoder_list	= entry;
		}

		// append it to the header of decoder list
		/*entry->next = NULL;
		if (!epg_decoder->tb_decoder_list) epg_decoder->tb_decoder_list = entry;
		else
		{
			ts_epg_tb_decoder_entry_t* last_entry = epg_decoder->tb_decoder_list;
			while (last_entry->next) last_entry = last_entry->next;
			last_entry->next = entry;
		}*/
	}
	return entry;
}
static void ts_epg_tb_decoder_list_destroy(ts_epg_decoder_t* epg_decoder)
{
	TS_ASSERT(epg_decoder);
	if (!epg_decoder) return ;

	ts_epg_tb_decoder_entry_t* entry = epg_decoder->tb_decoder_list;
	while (entry)
	{
		ts_epg_tb_decoder_entry_t* tmp = entry->next;

		// destroy specific table decoder
		if (entry->tb_decoder)
		{
			switch (entry->tb_decoder->table_id_t)
			{
				case en_ts_table_pat:
				  ts_table_pat_destroy_decoder((ts_table_pat_decoder_t*)entry->tb_decoder);
				  break;
				case en_ts_table_pmt:
				  ts_table_pmt_destroy_decoder((ts_table_pmt_decoder_t*)entry->tb_decoder);
				  break;
				case en_ts_table_sdt:
				  ts_table_sdt_destroy_decoder((ts_table_sdt_decoder_t*)entry->tb_decoder);
				  break;
				case en_ts_table_eit:
				  ts_table_eit_destroy_decoder((ts_table_eit_decoder_t*)entry->tb_decoder);
				  break;
			}
		}

		// free it
		free(entry);
		entry = tmp;
	}
}

// remove a new decoder from table decoder list header
static void ts_epg_tb_decoder_list_remove(ts_epg_decoder_t* epg_decoder, ts_table_decoder_t* tb_decoder)
{
	TS_ASSERT(epg_decoder && tb_decoder);
	if (!epg_decoder || !tb_decoder) return ;

	// create new entry and add to header
	ts_epg_tb_decoder_entry_t* entry = epg_decoder->tb_decoder_list;
	if (entry)
	{
		// remove it from header
		if (entry->tb_decoder == tb_decoder) 
		{
			epg_decoder->tb_decoder_list = entry->next;
			entry->next = NULL;
		}
		else	// remove it from body
		{
			while (entry->next)
			{
				if (entry->next->tb_decoder == tb_decoder)
				{
					entry->next = entry->next->next;
					break;
				}
				entry = entry->next;
			}
		}
	}
}
/* ////////////////////////////////////////////////////////////////////////
 * callbacks implementions
 */

// callback declarations
static ts_bool_t ts_table_pat_callback(void* tb_decoder, ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data);
static ts_bool_t ts_table_pmt_callback(void* tb_decoder, ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data);
static ts_bool_t ts_table_sdt_callback(void* tb_decoder, ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data);
static ts_bool_t ts_table_eit_callback(void* tb_decoder, ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data);
static ts_bool_t ts_table_nit_callback(void* tb_decoder, ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data);

// pat callback
static ts_bool_t ts_table_pat_callback(void* tb_decoder, ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data)
{
	ts_table_header_t*	tb_header		= table;
	ts_table_pat_t*			pat				= (ts_table_pat_t*)table;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	// epg_decoder
	ts_epg_decoder_t*		epg_decoder		= (ts_epg_decoder_t*)cb_data;

	// dump pat
	ts_table_pat_dump(ts_packet, pat);

	// add pmt decoder
	epg_decoder->program_list_count	= 0;
	ts_table_pat_program_t* program = pat->program_list;
	while (program)
	{
		if (program->program_number != 0)	// pmt pid
		{
			// create and add a pmt decoder
			ts_table_pmt_decoder_t* tb_pmt_decoder = ts_table_pmt_create_decoder(program->pid, program->program_number, &ts_table_pmt_callback, cb_data);
			ts_epg_tb_decoder_list_add(epg_decoder, (ts_table_decoder_t*)tb_pmt_decoder);

			// update counter
			epg_decoder->program_list_count++;
		}

		program = program->next;
	}

#if 1
	// remove pat decoder from table decoder list
	ts_epg_tb_decoder_list_remove(epg_decoder, (ts_table_decoder_t*)tb_decoder);

	//destroy pat decoder
	ts_table_pat_destroy_decoder(tb_decoder);

	// not continue to decode next pat if return TS_FALSE
	return TS_FALSE;	
#else
	return TS_TRUE;	// continue
#endif
}
// pmt callback
static ts_bool_t ts_table_pmt_callback(void* tb_decoder, ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data)
{
	ts_table_header_t*	tb_header		= table;
	ts_table_pmt_t*			pmt				= (ts_table_pmt_t*)table;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	// epg_decoder
	ts_epg_decoder_t*		epg_decoder		= (ts_epg_decoder_t*)cb_data;

	// dump pmt
	ts_table_pmt_dump(ts_packet, pmt);

	// create and add a new program 
	ts_epg_program_t* program	= ts_epg_program_create(epg_decoder);
	program->pid_pmt 			= ts_header->pid;
	program->pid_pcr			= pmt->pcr_pid;
	program->pid_videos_n 		= 0;
	program->pid_audios_n		= 0;
	program->program_number 	= tb_header->table_id_extension;
	program->service_id			= 0;

	// add video & audio pids
	ts_table_pmt_es_t* es = pmt->es_list;
	while(es)
	{
		if (es->stream_type == 0x01 || es->stream_type == 0x02) program->pid_videos[program->pid_videos_n++] = es->elementary_pid;
		else if (es->stream_type == 0x03 || es->stream_type == 0x04) program->pid_audios[program->pid_audios_n++] = es->elementary_pid;

		es = es->next;
	}

	// if all pmts have been decoded then decode sdt
	if (--epg_decoder->program_list_count == 0)
	{
		// add a sdt decoder for decoding sdt
		ts_table_sdt_decoder_t* tb_sdt_decoder = ts_table_sdt_create_decoder(&ts_table_sdt_callback, (void*)epg_decoder);
		ts_epg_tb_decoder_list_add(epg_decoder, (ts_table_decoder_t*)tb_sdt_decoder);
	}

	// remove pmt decoder from table decoder list
	ts_epg_tb_decoder_list_remove(epg_decoder, (ts_table_decoder_t*)tb_decoder);

	//destroy pmt decoder
	ts_table_pmt_destroy_decoder(tb_decoder);

	// not continue to decode next pmt if return TS_FALSE
	return TS_FALSE;
}
// sdt callback
static ts_bool_t ts_table_sdt_callback(void* tb_decoder, ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data)
{
	ts_table_header_t*	tb_header		= table;
	ts_table_sdt_t*			sdt				= (ts_table_sdt_t*)table;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	// epg_decoder
	ts_epg_decoder_t*		epg_decoder		= (ts_epg_decoder_t*)cb_data;

	// dump sdt
	ts_table_sdt_dump(ts_packet, sdt);

	// add service info to program if service_id == program_number
	ts_table_sdt_service_t* service = sdt->service_list;
	while (service)
	{
		// find program
		ts_epg_program_t* program	= ts_epg_program_find(epg_decoder, service->service_id);
		
		// fill service info
		program->service_id			= service->service_id;

		// fill program name
		ts_descriptor_t* descriptor = service->descriptor_list;
		while (descriptor)
		{
			if (descriptor->descriptor_tag == TS_DESCRIPTOR_TAG_SERVICE_DESCRIPTOR)
			{
				ts_service_descriptor_t* p = ts_service_descriptor_decode(descriptor);
				strcpy(program->program_provider_name, p->service_provider_name);
				strcpy(program->program_name, p->service_name);

				break;
			}
			descriptor = descriptor->next;
		}

		// next
		service = service->next;
	}

#if 0
	// remove sdt decoder from table decoder list
	ts_epg_tb_decoder_list_remove(epg_decoder, (ts_table_decoder_t*)tb_decoder);
	//destroy sdt decoder
	ts_table_sdt_destroy_decoder(tb_decoder);
#else
	// stop decoding and destroy decoder list
	ts_epg_tb_decoder_list_destroy(epg_decoder);
#endif
	
	// finish callback
	ts_epg_program_t* program = epg_decoder->program_list;
	while (program)
	{
		ts_uint_t i = 0;
		TS_PRINT("--------------------------------------");
		TS_PRINT("program_number:%u service_id:%u", program->program_number, program->service_id);
		TS_PRINT("--------------------------------------");
		TS_PRINT("pid_pmt:%x",						program->pid_pmt);
		TS_PRINT("pid_pcr:%x",						program->pid_pcr);
		for (i = 0; i < program->pid_videos_n; ++i)
			TS_PRINT("\tpid_video:%x",					program->pid_videos[i]);
		for (i = 0; i < program->pid_audios_n; ++i)
			TS_PRINT("\tpid_audio:%x",					program->pid_audios[i]);
		TS_PRINT("program_name:%s", 				program->program_name);
		TS_PRINT("program_provider_name:%s\n",		program->program_provider_name);
		
		program = program->next;
	}

	// not continue to decode next sdt if return TS_FALSE
	return TS_FALSE;
}

// eit callback
static ts_bool_t ts_table_eit_callback(void* tb_decoder, ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data)
{
	ts_table_header_t*	tb_header		= table;
	ts_table_eit_t*			eit				= (ts_table_eit_t*)table;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	// epg_decoder
	ts_epg_decoder_t*		epg_decoder		= (ts_epg_decoder_t*)cb_data;

	// dump eit
	ts_table_eit_dump(ts_packet, eit);

	// remove eit decoder from table decoder list
	ts_epg_tb_decoder_list_remove(epg_decoder, (ts_table_decoder_t*)tb_decoder);

	//destroy eit decoder
	ts_table_eit_destroy_decoder(tb_decoder);

	// not continue to decode next eit if return TS_FALSE
	return TS_FALSE;
}
// nit callback
static ts_bool_t ts_table_nit_callback(void* tb_decoder, ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data)
{
	ts_table_header_t*	tb_header		= table;
	ts_table_nit_t*			nit				= (ts_table_nit_t*)table;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	// epg_decoder
	ts_epg_decoder_t*		epg_decoder		= (ts_epg_decoder_t*)cb_data;

	// dump eit
	ts_table_nit_dump(ts_packet, nit);

	// remove eit decoder from table decoder list
	ts_epg_tb_decoder_list_remove(epg_decoder, (ts_table_decoder_t*)tb_decoder);

	//destroy nit decoder
	ts_table_nit_destroy_decoder(tb_decoder);

	// not continue to decode next nit if return TS_FALSE
	return TS_FALSE;
}

/* ////////////////////////////////////////////////////////////////////////
 * interfaces implementions
 */
ts_epg_decoder_t* ts_epg_create_decoder()
{
	// allocate decoders
	ts_epg_decoder_t* epg_decoder = (ts_epg_decoder_t*)malloc(sizeof(ts_epg_decoder_t));
	TS_ASSERT(epg_decoder);

	// init epg decoder
	epg_decoder->ts_decoder			= ts_create_decoder();
	epg_decoder->program_list		= NULL;
	epg_decoder->program_list_count	= 0;
	epg_decoder->tb_decoder_list	= NULL;
	// create decoder list and add a pat decoder at first
	ts_epg_tb_decoder_list_add(epg_decoder, (ts_table_decoder_t*)ts_table_pat_create_decoder(&ts_table_pat_callback, (void*)epg_decoder));

	ts_table_eit_decoder_t* tb_eit_decoder	= ts_table_eit_create_decoder(&ts_table_eit_callback, (void*)epg_decoder);
	ts_epg_tb_decoder_list_add(epg_decoder, (ts_table_decoder_t*)tb_eit_decoder);

	ts_table_nit_decoder_t* tb_nit_decoder	= ts_table_nit_create_decoder(&ts_table_nit_callback, (void*)epg_decoder);
	ts_epg_tb_decoder_list_add(epg_decoder, (ts_table_decoder_t*)tb_nit_decoder);

	return epg_decoder;
}

void ts_epg_destroy_decoder(ts_epg_decoder_t* epg_decoder)
{
	if (epg_decoder)
	{
		if (epg_decoder->ts_decoder) ts_destroy_decoder(epg_decoder->ts_decoder);
		if (epg_decoder->tb_decoder_list) ts_epg_tb_decoder_list_destroy(epg_decoder);

		free(epg_decoder);
	}
}

ts_bool_t ts_epg_decode(ts_packet_t* ts_packet, ts_epg_decoder_t* epg_decoder)
{
	// not exists decoders
	if (!epg_decoder || !epg_decoder->ts_decoder) return TS_FALSE;

	// decode packet
	epg_decoder->ts_decoder->skip_adaptation = TS_TRUE;
	if (TS_FALSE == ts_decode_packet(ts_packet, epg_decoder->ts_decoder)) return TS_FALSE;

	// empty packet filter
	if (ts_packet->header.pid == TS_PID_NULL) return TS_TRUE;

	// decode tables
	ts_epg_tb_decoder_entry_t* entry = epg_decoder->tb_decoder_list;
	while (entry)
	{
		// decode this ts packet
		if (entry->tb_decoder && TS_TRUE == ts_table_decode(ts_packet, entry->tb_decoder))
			return TS_TRUE;
		entry = entry->next;
	}

	// no decoder processes it
	return TS_FALSE;
}


// extern "C" {
#	ifdef __cplusplus
}
#	endif
