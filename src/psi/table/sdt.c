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
													ts_uint_t service_id
												, 	ts_uint_t eit_schedule_flag
												,	ts_uint_t eit_present_following_flag
												,	ts_uint_t running_status
												,	ts_uint_t free_ca_mode)
{
	TS_ASSERT(sdt);
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
																,	ts_uint_t descriptor_tag
																,	ts_uint_t descriptor_length
																,	ts_byte_t* data)
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
	TS_ASSERT(sdt);

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

		sdt->original_network_id			= ts_get_bits(section->payload_start, 0, 0, 16);
	}

	// need analyze
	sdt->service_list				= NULL;

	return (void*)sdt;
}
// empty the all programs
static void ts_table_sdt_empty(void* decoder, void* table)
{
	TS_ASSERT(table);
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
	TS_ASSERT(table);
	if (!table) return ;
	ts_table_sdt_t* sdt = (ts_table_sdt_t*)table;

	ts_table_sdt_empty(decoder, sdt);
	free(sdt);
}
// decode sdt
static void ts_table_sdt_decode(void* decoder, void* table, ts_section_t* sections)
{
	TS_ASSERT(table && sections);
	if (!table || !sections) return ;
	ts_table_sdt_t* sdt = (ts_table_sdt_t*)table;
	ts_section_t* section = sections;

	ts_byte_t* p = NULL;
	ts_byte_t* end = NULL;
	while (section)
	{
		for(p = section->payload_start + 3; p + 4 < section->payload_end;)
		{
			ts_uint_t service_id					= ts_get_bits(p, 0, 0, 16);
			//ts_uint_t reversed					= ts_get_bits(p, 0, 16, 6);
			ts_uint_t eit_schedule_flag				= ts_get_bits(p, 0, 22, 1);
			ts_uint_t eit_present_following_flag	= ts_get_bits(p, 0, 23, 1);
			ts_uint_t running_status				= ts_get_bits(p, 0, 24, 3);
			ts_uint_t free_ca_mode					= ts_get_bits(p, 0, 27, 1);
			ts_uint_t descriptors_loop_length		= ts_get_bits(p, 0, 28, 12);

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
				ts_uint_t descriptor_tag	= p[0];
				ts_uint_t descriptor_length	= p[1];

				if (descriptor_length + 2 <= end - p) ts_table_sdt_service_add_descriptor(service, descriptor_tag, descriptor_length, p + 2);
				p += 2 + descriptor_length;
			}
		}

		// process the next section in the sdt
		section = section->next;
	}
}
// check section
static ts_bool_t ts_table_sdt_check(void* decoder, ts_section_t* section)
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
	TS_ASSERT(sdt && ts_packet);
	if (!sdt || !ts_packet) return ;

	ts_table_header_t* header = (ts_table_header_t*)sdt;
	TS_PRINT("----------------------------------------"										);
	TS_PRINT("sdt(pid:%x)",						ts_packet->header.pid						);
	TS_PRINT("----------------------------------------"										);
	TS_PRINT("table_id:%x",						header->table_id							);
	TS_PRINT("section_syntax_indicator:%u",		header->section_syntax_indicator			);
	TS_PRINT("section_length:%u",				header->section_length						);
	TS_PRINT("transport_stream_id:%u",			header->table_id_extension					);
	TS_PRINT("version_number:%u",				header->version_number						);
	TS_PRINT("current_next_indicator:%u",		header->current_next_indicator				);
	TS_PRINT("section_number:%u",				header->section_number						);
	TS_PRINT("last_section_number:%u",			header->last_section_number					);
	TS_PRINT("crc:%x",							header->crc									);
	TS_PRINT("original_network_id:%x\n",		sdt->original_network_id					);

	ts_table_sdt_service_t* service = sdt->service_list;
	while (service)
	{
		TS_PRINT("\tservice_id:%u",					service->service_id					);
		TS_PRINT("\teit_schedule_flag:%u",			service->eit_schedule_flag			);
		TS_PRINT("\teit_present_following_flag:%u",	service->eit_present_following_flag	);
		TS_PRINT("\trunning_status:%u",				service->running_status				);
		TS_PRINT("\tfree_ca_mode:%u\n",				service->free_ca_mode				);

		ts_descriptor_t* descriptor = service->descriptor_list;
		while (descriptor)
		{
			TS_PRINT("\t\tdescriptor_tag:%x",		descriptor->descriptor_tag			);
			TS_PRINT("\t\tdescriptor_length:%u\n",	descriptor->descriptor_length		);

			if (descriptor->descriptor_tag == TS_DESCRIPTOR_TAG_SERVICE_DESCRIPTOR)
			{
				ts_service_descriptor_t* p = ts_service_descriptor_decode(descriptor);
				TS_PRINT("\t\tservice_provider_name:%s",	p->service_provider_name			);
				TS_PRINT("\t\tservice_name:%s\n",			p->service_name						);
			}

			descriptor = descriptor->next;
		}

		service = service->next;
	}
	TS_PRINT("========================================\n"										);
}

// extern "C" {
#	ifdef __cplusplus
}
#	endif
