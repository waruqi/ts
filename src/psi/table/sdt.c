// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "sdt.h"
#include "../descriptor/0x48.h"

/* //////////////////////////////////////////////////////////////////
 * macros
 */

/* //////////////////////////////////////////////////////////////////
 * inner implemention
 */
// add a service at the end of the sdt.
static ts_table_sdt_service_t* ts_table_sdt_add_service(	ts_table_sdt_t* sdt,
													tb_uint_t service_id
												, 	tb_uint_t eit_schedule_flag
												,	tb_uint_t eit_present_following_flag
												,	tb_uint_t running_status
												,	tb_uint_t free_ca_mode)
{
	tb_assert(sdt);
	if (!sdt) return NULL;

	// create a new service
	ts_table_sdt_service_t* service = (ts_table_sdt_service_t*)malloc(sizeof(ts_table_sdt_service_t));
	if (service)
	{
		// init the service
		service->service_id					= service_id;
		service->eit_schedule_flag			= eit_schedule_flag;
		service->eit_present_following_flag	= eit_present_following_flag;
		service->running_status				= running_status;
		service->free_ca_mode				= free_ca_mode;
		service->descriptor_list			= NULL;
		service->next						= NULL;

		// append it to the end of service list
		if (!sdt->service_list) sdt->service_list = service;
		else
		{
			ts_table_sdt_service_t* last_service = sdt->service_list;
			while (last_service->next) last_service = last_service->next;
			last_service->next = service;
		}
	}

	return service;
}

// add a descriptor in the sdt service description.
static ts_descriptor_t* ts_table_sdt_service_add_descriptor	(	ts_table_sdt_service_t* service
																,	tb_uint_t descriptor_tag
																,	tb_uint_t descriptor_length
																,	tb_byte_t* data)
{
	ts_descriptor_t* descriptor = ts_descriptor_create_descriptor(descriptor_tag, descriptor_length, data);
	if (descriptor)
	{
		if (!service->descriptor_list) service->descriptor_list = descriptor;
		else
		{
			// apped descriptor
			ts_descriptor_t* last_descriptor = service->descriptor_list;
			while (last_descriptor->next) last_descriptor = last_descriptor->next;
			last_descriptor->next = descriptor;
		}
	}

  return descriptor;
}
// create and init a new sdt
static void* ts_table_sdt_create(void* decoder, ts_section_t* sections)
{
	ts_table_sdt_t* sdt = (ts_table_sdt_t*)malloc(sizeof(ts_table_sdt_t));
	ts_table_header_t* header = (ts_table_header_t*)sdt;
	tb_assert(sdt);

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

		sdt->original_network_id			= tb_bits_get_ubits32(section->payload_start, 0, 16);
	}

	// need analyze
	sdt->service_list				= NULL;

	return (void*)sdt;
}
// empty the all programs
static void ts_table_sdt_empty(void* decoder, void* table)
{
	tb_assert(table);
	if (!table) return ;
	ts_table_sdt_t* sdt = (ts_table_sdt_t*)table;

	ts_table_sdt_service_t* service = sdt->service_list;
	while (service)
	{
		ts_table_sdt_service_t* tmp = service->next;
		ts_descriptor_destroy_descriptors(service->descriptor_list);
		free(service);
		service = tmp;
	}

	sdt->service_list = NULL;
}
// destroy sdt
static void ts_table_sdt_destroy(void* decoder, void* table)
{
	tb_assert(table);
	if (!table) return ;
	ts_table_sdt_t* sdt = (ts_table_sdt_t*)table;

	ts_table_sdt_empty(decoder, sdt);
	free(sdt);
}
// decode sdt
static void ts_table_sdt_decode(void* decoder, void* table, ts_section_t* sections)
{
	tb_assert(table && sections);
	if (!table || !sections) return ;
	ts_table_sdt_t* sdt = (ts_table_sdt_t*)table;
	ts_section_t* section = sections;

	tb_byte_t* p = NULL;
	tb_byte_t* end = NULL;
	while (section)
	{
		for(p = section->payload_start + 3; p + 4 < section->payload_end;)
		{
			tb_uint_t service_id					= tb_bits_get_ubits32(p, 0, 16);
			//tb_uint_t reversed					= tb_bits_get_ubits32(p, 16, 6);
			tb_uint_t eit_schedule_flag				= tb_bits_get_ubits32(p, 22, 1);
			tb_uint_t eit_present_following_flag	= tb_bits_get_ubits32(p, 23, 1);
			tb_uint_t running_status				= tb_bits_get_ubits32(p, 24, 3);
			tb_uint_t free_ca_mode					= tb_bits_get_ubits32(p, 27, 1);
			tb_uint_t descriptors_loop_length		= tb_bits_get_ubits32(p, 28, 12);

			ts_table_sdt_service_t* service = ts_table_sdt_add_service	(	sdt
																,	service_id
																,	eit_schedule_flag
																,	eit_present_following_flag
																,	running_status
																,	free_ca_mode);

			// service descriptors
			p += 5;
			end = p + descriptors_loop_length;
			if (end > section->payload_end) break;

			while (p + 2 <= end)
			{
				tb_uint_t descriptor_tag	= p[0];
				tb_uint_t descriptor_length	= p[1];

				if (descriptor_length + 2 <= end - p) ts_table_sdt_service_add_descriptor(service, descriptor_tag, descriptor_length, p + 2);
				p += 2 + descriptor_length;
			}
		}

		// process the next section in the sdt
		section = section->next;
	}
}
// check section
static tb_bool_t ts_table_sdt_check(void* decoder, ts_section_t* section)
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
// create sdt decoder
ts_table_sdt_decoder_t* ts_table_sdt_create_decoder(ts_table_callback_t callback, void* cb_data)
{
	// allocate decoder
	ts_table_sdt_decoder_t* tb_sdt_decoder = (ts_table_sdt_decoder_t*)malloc(sizeof(ts_table_sdt_decoder_t));

	// init table decoder
	ts_table_decoder_t* tb_decoder = (ts_table_decoder_t*)tb_sdt_decoder;
	ts_table_decoder_init(tb_decoder, TS_PID_SDT);
	ts_table_register_callback(tb_decoder, callback, cb_data);

	// init callbacks
	tb_decoder->table_create	= &ts_table_sdt_create;
	tb_decoder->table_decode	= &ts_table_sdt_decode;
	tb_decoder->table_destroy	= &ts_table_sdt_destroy;
	tb_decoder->table_check		= &ts_table_sdt_check;

	// init decoder type
	tb_decoder->table_id_t		= en_ts_table_sdt;

	return tb_sdt_decoder;
}
void ts_table_sdt_destroy_decoder(ts_table_sdt_decoder_t* tb_sdt_decoder)
{
	if (tb_sdt_decoder)
	{
		ts_table_decoder_exit((ts_table_decoder_t*)tb_sdt_decoder);
		free(tb_sdt_decoder);
	}
}
// dump sdt info
void ts_table_sdt_dump(ts_packet_t* ts_packet, ts_table_sdt_t* sdt)
{
	tb_assert(sdt && ts_packet);
	if (!sdt || !ts_packet) return ;

	ts_table_header_t* header = (ts_table_header_t*)sdt;
	tb_print("----------------------------------------"										);
	tb_print("sdt(pid:%x)",						ts_packet->header.pid						);
	tb_print("----------------------------------------"										);
	tb_print("table_id:%x",						header->table_id							);
	tb_print("section_syntax_indicator:%u",		header->section_syntax_indicator			);
	tb_print("section_length:%u",				header->section_length						);
	tb_print("transport_stream_id:%u",			header->table_id_extension					);
	tb_print("version_number:%u",				header->version_number						);
	tb_print("current_next_indicator:%u",		header->current_next_indicator				);
	tb_print("section_number:%u",				header->section_number						);
	tb_print("last_section_number:%u",			header->last_section_number					);
	tb_print("crc:%x",							header->crc									);
	tb_print("original_network_id:%x\n",		sdt->original_network_id					);

	ts_table_sdt_service_t* service = sdt->service_list;
	while (service)
	{
		tb_print("\tservice_id:%u",					service->service_id					);
		tb_print("\teit_schedule_flag:%u",			service->eit_schedule_flag			);
		tb_print("\teit_present_following_flag:%u",	service->eit_present_following_flag	);
		tb_print("\trunning_status:%u",				service->running_status				);
		tb_print("\tfree_ca_mode:%u\n",				service->free_ca_mode				);

		ts_descriptor_t* descriptor = service->descriptor_list;
		while (descriptor)
		{
			tb_print("\t\tdescriptor_tag:%x",		descriptor->descriptor_tag			);
			tb_print("\t\tdescriptor_length:%u\n",	descriptor->descriptor_length		);

			if (descriptor->descriptor_tag == TS_DESCRIPTOR_TAG_SERVICE_DESCRIPTOR)
			{
				ts_service_descriptor_t* p = ts_service_descriptor_decode(descriptor);
				tb_print("\t\tservice_provider_name:%s",	p->service_provider_name			);
				tb_print("\t\tservice_name:%s\n",			p->service_name						);
			}

			descriptor = descriptor->next;
		}

		service = service->next;
	}
	tb_print("========================================\n"										);
}

// extern "C" {
#	ifdef __cplusplus
}
#	endif
