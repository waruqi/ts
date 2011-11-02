// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "eit.h"
#include "../descriptor/0x4d.h"

/* //////////////////////////////////////////////////////////////////
 * macros
 */

/* //////////////////////////////////////////////////////////////////
 * inner implemention
 */
// add a event at the end of the eit.
static ts_table_eit_event_t* ts_table_eit_add_event(	ts_table_eit_t* eit
											,	ts_uint_t event_id
											,	ts_uint_t start_time_mjd
											,	ts_uint_t start_time_utc
											,	ts_uint_t duration
											,	ts_uint_t running_status
											,	ts_uint_t free_ca_mode)
{
	TS_ASSERT(eit);
	if (!eit) return NULL;

	// create a new service
	ts_table_eit_event_t* event = (ts_table_eit_event_t*)malloc(sizeof(ts_table_eit_event_t));
	if (event)
	{
		// init the event
		event->event_id						= event_id;
		event->start_time_mjd				= start_time_mjd;
		event->start_time_utc				= start_time_utc;
		event->duration						= duration;
		event->running_status				= running_status;
		event->free_ca_mode					= free_ca_mode;
		event->descriptor_list				= NULL;
		event->next							= NULL;

		// append it to the end of event list
		if (!eit->event_list) eit->event_list = event;
		else
		{
			ts_table_eit_event_t* last_event = eit->event_list;
			while (last_event->next) last_event = last_event->next;
			last_event->next = event;
		}
	}

	return event;
}

// add a descriptor in the eit event description.
static ts_descriptor_t* ts_table_eit_event_add_descriptor	(	ts_table_eit_event_t* event
															,	ts_uint_t descriptor_tag
															,	ts_uint_t descriptor_length
															,	ts_byte_t* data)
{
	ts_descriptor_t* descriptor = ts_descriptor_create_descriptor(descriptor_tag, descriptor_length, data);
	if (descriptor)
	{
		if (!event->descriptor_list) event->descriptor_list = descriptor;
		else
		{
			// apped descriptor
			ts_descriptor_t* last_descriptor = event->descriptor_list;
			while (last_descriptor->next) last_descriptor = last_descriptor->next;
			last_descriptor->next = descriptor;
		}
	}

  return descriptor;
}
// create and init a new eit
static void* ts_table_eit_create(void* decoder, ts_section_t* sections)
{
	ts_table_eit_t* eit = (ts_table_eit_t*)malloc(sizeof(ts_table_eit_t));
	ts_table_header_t* header = (ts_table_header_t*)eit;
	TS_ASSERT(eit);

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

		eit->transport_stream_id			= ts_get_bits(section->payload_start, 0, 0, 16);
		eit->original_network_id			= ts_get_bits(section->payload_start, 0, 16, 16);
		eit->segment_last_section_number	= ts_get_bits(section->payload_start, 0, 32, 8);
		eit->last_table_id					= ts_get_bits(section->payload_start, 0, 40, 8);
	}

	// need analyze
	eit->event_list				= NULL;

	return (void*)eit;
}
// empty the all programs
static void ts_table_eit_empty(void* decoder, void* table)
{
	TS_ASSERT(table);
	if (!table) return ;
	ts_table_eit_t* eit = (ts_table_eit_t*)table;

	ts_table_eit_event_t* event = eit->event_list;
	while (event)
	{
		ts_table_eit_event_t* tmp = event->next;
		ts_descriptor_destroy_descriptors(event->descriptor_list);
		free(event);
		event = tmp;
	}

	eit->event_list = NULL;
}
// destroy eit
static void ts_table_eit_destroy(void* decoder, void* table)
{
	TS_ASSERT(table);
	if (!table) return ;
	ts_table_eit_t* eit = (ts_table_eit_t*)table;

	ts_table_eit_empty(decoder, eit);
	free(eit);
}
// decode eit
static void ts_table_eit_decode(void* decoder, void* table, ts_section_t* sections)
{
	TS_ASSERT(table && sections);
	if (!table || !sections) return ;
	ts_table_eit_t* eit = (ts_table_eit_t*)table;
	ts_section_t* section = sections;

	ts_byte_t* p = NULL;
	ts_byte_t* end = NULL;
	while (section)
	{
		for(p = section->payload_start + 6; p < section->payload_end - 12;)
		{
			ts_uint_t event_id						= ts_get_bits(p, 0, 0, 16);
			ts_uint_t start_time_mjd				= ts_get_bits(p, 0, 16, 16);
			ts_uint_t start_time_utc				= ts_get_bits(p, 0, 32, 24);
			ts_uint_t duration						= ts_get_bits(p, 0, 56, 24);
			ts_uint_t running_status				= ts_get_bits(p, 0, 80, 3);
			ts_uint_t free_ca_mode					= ts_get_bits(p, 0, 83, 1);
			ts_uint_t descriptors_loop_length		= ts_get_bits(p, 0, 84, 12);

			ts_table_eit_event_t* event = ts_table_eit_add_event	(	eit
															,	event_id
															,	start_time_mjd
															,	start_time_utc
															,	duration
															,	running_status
															,	free_ca_mode);

			// event descriptors
			p += 12;
			end = p + descriptors_loop_length;
			if (end > section->payload_end) break;

			while (p + 2 <= end)
			{
				ts_uint_t descriptor_tag		= p[0];
				ts_uint_t descriptor_length	= p[1];

				if (descriptor_length + 2 <= end - p) ts_table_eit_event_add_descriptor(event, descriptor_tag, descriptor_length, p + 2);
				p += 2 + descriptor_length;
			}
		}

		// process the next section in the eit
		section = section->next;
	}
}
// check section
static ts_bool_t ts_table_eit_check(void* decoder, ts_section_t* section)
{
	TS_ASSERT(decoder && section);
	if (!decoder || !section) return TS_FALSE;
	if (!section) return TS_FALSE;

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
// create eit decoder
ts_table_eit_decoder_t* ts_table_eit_create_decoder(ts_table_callback_t callback, void* cb_data)
{
	// allocate decoder
	ts_table_eit_decoder_t* tb_eit_decoder = (ts_table_eit_decoder_t*)malloc(sizeof(ts_table_eit_decoder_t));

	// init table decoder
	ts_table_decoder_t* tb_decoder = (ts_table_decoder_t*)tb_eit_decoder;
	ts_table_decoder_init(tb_decoder, TS_PID_EIT);
	ts_table_register_callback(tb_decoder, callback, cb_data);

	// init callbacks
	tb_decoder->table_create	= &ts_table_eit_create;
	tb_decoder->table_decode	= &ts_table_eit_decode;
	tb_decoder->table_destroy	= &ts_table_eit_destroy;
	tb_decoder->table_check		= &ts_table_eit_check;

	// init decoder type
	tb_decoder->table_id_t		= en_ts_table_eit;

	return tb_eit_decoder;
}
void ts_table_eit_destroy_decoder(ts_table_eit_decoder_t* tb_eit_decoder)
{
	if (tb_eit_decoder)
	{
		ts_table_decoder_exit((ts_table_decoder_t*)tb_eit_decoder);
		free(tb_eit_decoder);
	}
}
// dump eit info
void ts_table_eit_dump(ts_packet_t* ts_packet, ts_table_eit_t* eit)
{
	TS_ASSERT(eit && ts_packet);
	if (!eit || !ts_packet) return ;

	ts_table_header_t* header = (ts_table_header_t*)eit;
	TS_PRINT("----------------------------------------"										);
	TS_PRINT("eit(pid:%x)",						ts_packet->header.pid						);
	TS_PRINT("----------------------------------------"										);
	TS_PRINT("table_id:%x",						header->table_id							);
	TS_PRINT("section_syntax_indicator:%u",		header->section_syntax_indicator			);
	TS_PRINT("section_length:%u",				header->section_length						);
	TS_PRINT("service_id:%u",					header->table_id_extension					);
	TS_PRINT("version_number:%u",				header->version_number						);
	TS_PRINT("current_next_indicator:%u",		header->current_next_indicator				);
	TS_PRINT("section_number:%u",				header->section_number						);
	TS_PRINT("last_section_number:%u",			header->last_section_number					);
	TS_PRINT("crc:%x",							header->crc									);
	TS_PRINT("original_network_id:%x",			eit->original_network_id					);
	TS_PRINT("segment_last_section_number:%x",	eit->segment_last_section_number			);
	TS_PRINT("last_table_id:%x",				eit->last_table_id							);

	ts_table_eit_event_t* event = eit->event_list;
	while (event)
	{
		ts_uint_t mjd = event->start_time_mjd;
		ts_uint_t b_utc = event->start_time_utc;
		ts_uint_t dur = event->duration;
		ts_uint_t e_utc = b_utc + dur;

		TS_PRINT("\tevent_id:%u",								event->event_id					);
		TS_PRINT("\tstart_time_mjd:%x mjd:%02u-%02u-%02u",		mjd, TS_MJD_Y(mjd), TS_MJD_M(mjd), TS_MJD_D(mjd));
		TS_PRINT("\tstart_time_utc:%x utc:%02x:%02x:%02x",		b_utc, TS_UTC_H(b_utc), TS_UTC_M(b_utc), TS_UTC_S(b_utc));
		TS_PRINT("\tend_time_utc:%x utc:%02x:%02x:%02x",		e_utc, TS_UTC_H(e_utc), TS_UTC_M(e_utc), TS_UTC_S(e_utc));
		TS_PRINT("\tduration:%u utc: %02x:%02x:%02x",			dur, TS_UTC_H(dur), TS_UTC_M(dur), TS_UTC_S(dur));
		TS_PRINT("\trunning_status:%u",							event->running_status			);
		TS_PRINT("\tfree_ca_mode:%u\n",							event->free_ca_mode				);

		ts_descriptor_t* descriptor = event->descriptor_list;
		while (descriptor)
		{
			TS_PRINT("\t\tdescriptor_tag:%x",			descriptor->descriptor_tag			);
			TS_PRINT("\t\tdescriptor_length:%u\n",		descriptor->descriptor_length		);

			if (descriptor->descriptor_tag == TS_DESCRIPTOR_TAG_SHORT_EVENT_DESCRIPTOR)
			{
				ts_short_event_descriptor_t* p = ts_short_event_descriptor_decode(descriptor);
				TS_PRINT("\t\tiso_639_code:%s",				p->iso_639_code			);
				TS_PRINT("\t\tevent_name:%s",				p->event_name			);
				TS_PRINT("\t\ttext:%s\n",					p->text					);
			}

			descriptor = descriptor->next;
		}


		event = event->next;
	}
	TS_PRINT("========================================\n"										);
}

// extern "C" {
#	ifdef __cplusplus
}
#	endif
