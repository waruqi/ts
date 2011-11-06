/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "0x41.h"

/* //////////////////////////////////////////////////////////////////
 * macros
 */

/* //////////////////////////////////////////////////////////////////
 * implemention
 */

// decode service list descriptor
ts_service_list_descriptor_t* ts_service_list_descriptor_decode(ts_descriptor_t* dr)
{
	tb_assert(dr);
	if (!dr) return NULL;

	// check the tag
	if (dr->descriptor_tag != TS_DESCRIPTOR_TAG_SERVICE_LIST_DESCRIPTOR)
	{
		tb_trace("service list descriptor: bad tag (%#x)", dr->descriptor_tag);
		return NULL;
	}

	// don't decode twice
	//if (dr->decoded_dr) return dr->decoded_dr;

	// check the length
	if (dr->descriptor_length < 3)
	{
		tb_trace("service list descriptor: bad length (%d)", dr->descriptor_length);
		return NULL;
	}

	// check data
	if (!dr->data)
	{
		tb_trace("service list descriptor: invalid data");
		return NULL;
	}

	//! \note need free data by ts_service_list_descriptor_destroy()
	//dr->decoded_dr = (void*)p;
	dr->decoded_dr = NULL;

	// decode data
	tb_byte_t* p = dr->data;
	tb_byte_t* end = dr->data + dr->descriptor_length;
	ts_service_list_descriptor_t* list = NULL;
	for (; p + 3 < end; p += 3)
	{
		// allocate service entry
		ts_service_list_descriptor_t* service = (ts_service_list_descriptor_t*)malloc(sizeof(ts_service_list_descriptor_t));
		if (service)
		{
			service->service_id		= tb_bits_get_ubits32(p, 0, 16);
			service->service_type	= tb_bits_get_ubits32(p, 16, 8);
			service->next			= NULL;
			
			// add service to list
			if (!list) list = service;
			else
			{
				ts_service_list_descriptor_t* last_service = list;
				while (last_service->next) last_service = last_service->next;
				last_service->next = service;
			}
		}
	}
	
	return list;
}

// destroy service list descriptor
void ts_service_list_descriptor_destroy(ts_service_list_descriptor_t* service_list)
{
	ts_service_list_descriptor_t* service = service_list;
	while (service)
	{
		ts_service_list_descriptor_t* next = service->next;
		free(service);
		service = next;
	}
}

