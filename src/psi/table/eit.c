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
											,	tb_uint_t event_id
											,	tb_uint_t start_time_mjd
											,	tb_uint_t start_time_utc
											,	tb_uint_t duration
											,	tb_uint_t running_status
											,	tb_uint_t free_ca_mode)
{
	ts_assert(eit);
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
															,	tb_uint_t descriptor_tag
															,	tb_uint_t descriptor_length
															,	tb_byte_t* data)
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
	ts_assert(eit);

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

		eit->transport_stream_id			= tb_bits_get_ubits32(section->payload_start, 0, 16);
		eit->original_network_id			= tb_bits_get_ubits32(section->payload_start, 16, 16);
		eit->segment_last_section_number	= tb_bits_get_ubits32(section->payload_start, 32, 8);
		eit->last_table_id					= tb_bits_get_ubits32(section->payload_start, 40, 8);
	}

	// need analyze
	eit->event_list				= NULL;

	return (void*)eit;
}
// empty the all programs
static void ts_table_eit_empty(void* decoder, void* table)
{
	ts_assert(table);
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
	ts_assert(table);
	if (!table) return ;
	ts_table_eit_t* eit = (ts_table_eit_t*)table;

	ts_table_eit_empty(decoder, eit);
	free(eit);
}
// decode eit
static void ts_table_eit_decode(void* decoder, void* table, ts_section_t* sections)
{
	ts_assert(table && sections);
	if (!table || !sections) return ;
	ts_table_eit_t* eit = (ts_table_eit_t*)table;
	ts_section_t* section = sections;

	tb_byte_t* p = NULL;
	tb_byte_t* end = NULL;
	while (section)
	{
		for(p = section->payload_start + 6; p < section->payload_end - 12;)
		{
			tb_uint_t event_id						= tb_bits_get_ubits32(p, 0, 16);
			tb_uint_t start_time_mjd				= tb_bits_get_ubits32(p, 16, 16);
			tb_uint_t start_time_utc				= tb_bits_get_ubits32(p, 32, 24);
			tb_uint_t duration						= tb_bits_get_ubits32(p, 56, 24);
			tb_uint_t running_status				= tb_bits_get_ubits32(p, 80, 3);
			tb_uint_t free_ca_mode					= tb_bits_get_ubits32(p, 83, 1);
			tb_uint_t descriptors_loop_length		= tb_bits_get_ubits32(p, 84, 12);

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
				tb_uint_t descriptor_tag		= p[0];
				tb_uint_t descriptor_length	= p[1];

				if (descriptor_length + 2 <= end - p) ts_table_eit_event_add_descriptor(event, descriptor_tag, descriptor_length, p + 2);
				p += 2 + descriptor_length;
			}
		}

		// process the next section in the eit
		section = section->next;
	}
}
// check section
static tb_bool_t ts_table_eit_check(void* decoder, ts_section_t* section)
{
	ts_assert(decoder && section);
	if (!decoder || !section) return TB_FALSE;
	if (!section) return TB_FALSE;

	// check section_syntax_indicator: must be 1
	if (!section->section_syntax_indicator)
	{
		// invalid section_syntax_indicator
		ts_trace("invalid section (section_syntax_indicator == 0)");
		return TB_FALSE;
	}

	return TB_TRUE;
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
	ts_assert(eit && ts_packet);
	if (!eit || !ts_packet) return ;

	ts_table_header_t* header = (ts_table_header_t*)eit;
	ts_print("----------------------------------------"										);
	ts_print("eit(pid:%x)",						ts_packet->header.pid						);
	ts_print("----------------------------------------"										);
	ts_print("table_id:%x",						header->table_id							);
	ts_print("section_syntax_indicator:%u",		header->section_syntax_indicator			);
	ts_print("section_length:%u",				header->section_length						);
	ts_print("service_id:%u",					header->table_id_extension					);
	ts_print("version_number:%u",				header->version_number						);
	ts_print("current_next_indicator:%u",		header->current_next_indicator				);
	ts_print("section_number:%u",				header->section_number						);
	ts_print("last_section_number:%u",			header->last_section_number					);
	ts_print("crc:%x",							header->crc									);
	ts_print("original_network_id:%x",			eit->original_network_id					);
	ts_print("segment_last_section_number:%x",	eit->segment_last_section_number			);
	ts_print("last_table_id:%x",				eit->last_table_id							);

	ts_table_eit_event_t* event = eit->event_list;
	while (event)
	{
		tb_uint_t mjd = event->start_time_mjd;
		tb_uint_t b_utc = event->start_time_utc;
		tb_uint_t dur = event->duration;
		tb_uint_t e_utc = b_utc + dur;

		ts_print("\tevent_id:%u",								event->event_id					);
		ts_print("\tstart_time_mjd:%x mjd:%02u-%02u-%02u",		mjd, TS_MJD_Y(mjd), TS_MJD_M(mjd), TS_MJD_D(mjd));
		ts_print("\tstart_time_utc:%x utc:%02x:%02x:%02x",		b_utc, TS_UTC_H(b_utc), TS_UTC_M(b_utc), TS_UTC_S(b_utc));
		ts_print("\tend_time_utc:%x utc:%02x:%02x:%02x",		e_utc, TS_UTC_H(e_utc), TS_UTC_M(e_utc), TS_UTC_S(e_utc));
		ts_print("\tduration:%u utc: %02x:%02x:%02x",			dur, TS_UTC_H(dur), TS_UTC_M(dur), TS_UTC_S(dur));
		ts_print("\trunning_status:%u",							event->running_status			);
		ts_print("\tfree_ca_mode:%u\n",							event->free_ca_mode				);

		ts_descriptor_t* descriptor = event->descriptor_list;
		while (descriptor)
		{
			ts_print("\t\tdescriptor_tag:%x",			descriptor->descriptor_tag			);
			ts_print("\t\tdescriptor_length:%u\n",		descriptor->descriptor_length		);

			if (descriptor->descriptor_tag == TS_DESCRIPTOR_TAG_SHORT_EVENT_DESCRIPTOR)
			{
				ts_short_event_descriptor_t* p = ts_short_event_descriptor_decode(descriptor);
				ts_print("\t\tiso_639_code:%s",				p->iso_639_code			);
				ts_print("\t\tevent_name:%s",				p->event_name			);
				ts_print("\t\ttext:%s\n",					p->text					);
			}

			descriptor = descriptor->next;
		}


		event = event->next;
	}
	ts_print("========================================\n"										);
}

// extern "C" {
#	ifdef __cplusplus
}
#	endif
