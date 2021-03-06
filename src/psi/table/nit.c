// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "nit.h"
#include "../descriptor/0x40.h"
#include "../descriptor/0x41.h"

/* //////////////////////////////////////////////////////////////////
 * macros
 */

/* //////////////////////////////////////////////////////////////////
 * inner implemention
 */
// add a descriptor in the nit.
static ts_descriptor_t* ts_table_nit_add_descriptor(ts_table_nit_t* nit, tb_uint_t descriptor_tag, tb_uint_t descriptor_length, tb_byte_t* data)
{
	// create a descriptor
	ts_descriptor_t* descriptor = ts_descriptor_create_descriptor(descriptor_tag, descriptor_length, data);
	if (descriptor)
	{
		if (!nit->descriptor_list) nit->descriptor_list = descriptor;
		else
		{
			// apped descriptor
			ts_descriptor_t* last_descriptor = nit->descriptor_list;
			while (last_descriptor->next) last_descriptor = last_descriptor->next;
			last_descriptor->next = descriptor;
		}
	}
	return descriptor;
}

// add ts to nit
static ts_table_nit_ts_t* ts_table_nit_add_ts(ts_table_nit_t* nit, tb_uint_t transport_stream_id, tb_uint_t original_network_id)
{
	// allocate a ts
	ts_table_nit_ts_t* ts = (ts_table_nit_ts_t*)malloc(sizeof(ts_table_nit_ts_t));
	if (ts)
	{
		// init header
		ts->transport_stream_id		= transport_stream_id;
		ts->original_network_id		= original_network_id;

		// init other
		ts->descriptor_list	= NULL;
		ts->next			= NULL;

		// append ts
		if (!nit->ts_list) nit->ts_list = ts;
		else
		{
			ts_table_nit_ts_t* last_ts = nit->ts_list;
			while (last_ts->next) last_ts = last_ts->next;
			last_ts->next = ts;
		}
	}

	return ts;
}
// add a descriptor in the nit ts
ts_descriptor_t* ts_table_nit_ts_add_descriptor(ts_table_nit_ts_t* ts, tb_uint_t descriptor_tag, tb_uint_t descriptor_length, tb_byte_t* data)
{
	// create a descriptor
	ts_descriptor_t* descriptor = ts_descriptor_create_descriptor(descriptor_tag, descriptor_length, data);
	if (descriptor)
	{
		if (!ts->descriptor_list) ts->descriptor_list = descriptor;
		else
		{
			// apped descriptor
			ts_descriptor_t* last_descriptor = ts->descriptor_list;
			while (last_descriptor->next) last_descriptor = last_descriptor->next;
			last_descriptor->next = descriptor;
		}
	}
	return descriptor;
}
// create and init a new nit
static void* ts_table_nit_create(void* decoder, ts_section_t* sections)
{
	ts_table_nit_t* nit = (ts_table_nit_t*)malloc(sizeof(ts_table_nit_t));
	ts_table_header_t* header = (ts_table_header_t*)nit;
	tb_assert(nit);

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
	nit->descriptor_list					= NULL;
	nit->ts_list							= NULL;

	return (void*)nit;
}
// empty the all programs
static void ts_table_nit_empty(void* decoder, void* table)
{
	tb_assert(table);
	if (!table) return ;
	ts_table_nit_t* nit = (ts_table_nit_t*)table;
	ts_table_nit_ts_t* ts = nit->ts_list;

	// destroy nit descriptor list
	ts_descriptor_destroy_descriptors(nit->descriptor_list);

	// destroy ts list
	while(ts)
	{
		ts_table_nit_ts_t* tmp = ts->next;

		// destroy nit ts descriptor list
		ts_descriptor_destroy_descriptors(ts->descriptor_list);
		free(ts);
		ts = tmp;
	}

	nit->descriptor_list = NULL;
	nit->ts_list = NULL;
}
// destroy nit
static void ts_table_nit_destroy(void* decoder, void* table)
{
	tb_assert(table);
	if (!table) return ;
	ts_table_nit_t* nit = (ts_table_nit_t*)table;

	ts_table_nit_empty(decoder, nit);
	free(nit);
}
// decode nit
static void ts_table_nit_decode(void* decoder, void* table, ts_section_t* sections)
{
	tb_assert(table && sections);
	if (!table || !sections) return ;
	ts_table_nit_t* nit = (ts_table_nit_t*)table;
	ts_section_t* section = sections;

	tb_byte_t* p = NULL;
	tb_byte_t* end = NULL;
	while (section)
	{
		// point to descriptor_list
		p = section->payload_start + 2;

		// end: p + network_descriptor_length
		end = p + tb_bits_get_ubits32(section->payload_start, 4, 12);

		// add all descriptors to nit
		while (p + 2 <= end)
		{
			// decode header
			tb_uint_t descriptor_tag		= p[0];
			tb_uint_t descriptor_length	= p[1];

			// add it
			if (descriptor_length + 2 <= end - p) ts_table_nit_add_descriptor(nit, descriptor_tag, descriptor_length, p + 2);
			p += 2 + descriptor_length;
		}

		// transport_stream list
		tb_uint_t transport_stream_loop_length	= tb_bits_get_ubits32(end, 4, 12);
		p = end + 2;
		end = p + transport_stream_loop_length;
		for (; p + 2 <= end;)
		{
			// decode header
			tb_uint_t transport_stream_id			= tb_bits_get_ubits32(p, 0, 16);
			tb_uint_t original_network_id			= tb_bits_get_ubits32(p, 16, 16);
			tb_uint_t transport_descriptor_length	= tb_bits_get_ubits32(p, 36, 12);

			// add transport_stream to nit
			ts_table_nit_ts_t* ts = ts_table_nit_add_ts(nit, transport_stream_id, original_network_id);

			// - transport_stream descriptors
			p += 6;
			end = p + transport_descriptor_length;
			if (end > section->payload_end) end = section->payload_end;
			while (p + 2 <= end)
			{
				tb_uint_t descriptor_tag		= p[0];
				tb_uint_t descriptor_length	= p[1];

				if (descriptor_length + 2 <= end - p) ts_table_nit_ts_add_descriptor(ts, descriptor_tag, descriptor_length, p + 2);
				p += 2 + descriptor_length;
			}
		}

		// process the next section in the nit
		section = section->next;
	}
}
// check section
static tb_bool_t ts_table_nit_check(void* decoder, ts_section_t* section)
{
	tb_assert(decoder && section);
	if (!decoder || !section) return TB_FALSE;
	if (!section) return TB_FALSE;

	// check section_syntax_indicator: must be 1
	if (!section->section_syntax_indicator)
	{
		// invalid section_syntax_indicator
		tb_trace("invalid section (section_syntax_indicator == 0)");
		return TB_FALSE;
	}

	return TB_TRUE;
}

/* //////////////////////////////////////////////////////////////////
 * interface implemention
 */
// create nit decoder
ts_table_nit_decoder_t* ts_table_nit_create_decoder(ts_table_callback_t callback, void* cb_data)
{
	// allocate decoder
	ts_table_nit_decoder_t* tb_nit_decoder = (ts_table_nit_decoder_t*)malloc(sizeof(ts_table_nit_decoder_t));

	// init table decoder
	ts_table_decoder_t* tb_decoder = (ts_table_decoder_t*)tb_nit_decoder;
	ts_table_decoder_init(tb_decoder, TS_PID_NIT);
	ts_table_register_callback(tb_decoder, callback, cb_data);

	// init callbacks
	tb_decoder->table_create	= &ts_table_nit_create;
	tb_decoder->table_decode	= &ts_table_nit_decode;
	tb_decoder->table_destroy	= &ts_table_nit_destroy;
	tb_decoder->table_check		= &ts_table_nit_check;

	// init decoder type
	tb_decoder->table_id_t		= en_ts_table_nit;

	return tb_nit_decoder;
}
void ts_table_nit_destroy_decoder(ts_table_nit_decoder_t* tb_nit_decoder)
{
	if (tb_nit_decoder)
	{
		ts_table_decoder_exit((ts_table_decoder_t*)tb_nit_decoder);
		free(tb_nit_decoder);
	}
}
// dump nit info
void ts_table_nit_dump(ts_packet_t* ts_packet, ts_table_nit_t* nit)
{
	tb_assert(nit && ts_packet);
	if (!nit || !ts_packet) return ;

	ts_table_header_t* header = (ts_table_header_t*)nit;
	tb_print("----------------------------------------"										);
	tb_print("nit(pid:%x)",						ts_packet->header.pid						);
	tb_print("----------------------------------------"										);
	tb_print("table_id:%x",						header->table_id							);
	tb_print("section_syntax_indicator:%u",		header->section_syntax_indicator			);
	tb_print("section_length:%u",				header->section_length						);
	tb_print("network_id:%x",					header->table_id_extension					);
	tb_print("version_number:%u",				header->version_number						);
	tb_print("current_next_indicator:%u",		header->current_next_indicator				);
	tb_print("section_number:%u",				header->section_number						);
	tb_print("last_section_number:%u",			header->last_section_number					);
	tb_print("crc:%x",							header->crc									);

	ts_descriptor_t* descriptor = nit->descriptor_list;
	while (descriptor)
	{
		tb_print("\tdescriptor_tag:%x",			descriptor->descriptor_tag					);
		tb_print("\tdescriptor_length:%x\n",	descriptor->descriptor_length				);

		if (descriptor->descriptor_tag == TS_DESCRIPTOR_TAG_NETWORK_NAME_DESCRIPTOR)
		{
			ts_network_name_descriptor_t* network_name = ts_network_name_descriptor_decode(descriptor);
			tb_print("\tnetwork_name: %s\n",	network_name->name							);
		}

		descriptor = descriptor->next;
	}

	// destroy ts list
	ts_table_nit_ts_t* ts = nit->ts_list;
	while(ts)
	{
		tb_print("\tes transport_stream_id:%x",		ts->transport_stream_id				);
		tb_print("\tes original_network_id:%x\n",	ts->original_network_id				);

		descriptor = ts->descriptor_list;
		while (descriptor)
		{
			tb_print("\t\tdescriptor_tag:%x",		descriptor->descriptor_tag				);
			tb_print("\t\tdescriptor_length:%u\n",	descriptor->descriptor_length			);

			if (descriptor->descriptor_tag == TS_DESCRIPTOR_TAG_SERVICE_LIST_DESCRIPTOR)
			{
				ts_service_list_descriptor_t* service_list = ts_service_list_descriptor_decode(descriptor);
				ts_service_list_descriptor_t* service = service_list;
				while (service)
				{
					tb_print("\t\tservice_id:%u",		service->service_id					);
					tb_print("\t\tservice_type:%x\n",	service->service_type				);
					service = service->next;
				}
				ts_service_list_descriptor_destroy(service_list);
			}
			descriptor = descriptor->next;
		}

		ts = ts->next;
	}

	tb_print("========================================\n"										);
}

// extern "C" {
#	ifdef __cplusplus
}
#	endif
