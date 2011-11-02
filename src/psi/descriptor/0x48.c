/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "0x48.h"

/* //////////////////////////////////////////////////////////////////
 * macros
 */

/* //////////////////////////////////////////////////////////////////
 * implemention
 */

// decode service a descriptor
ts_service_descriptor_t* ts_service_descriptor_decode(ts_descriptor_t* dr)
{
	TS_ASSERT(dr);
	if (!dr) return NULL;
	ts_service_descriptor_t* p = NULL;

	// check the tag
	if (dr->descriptor_tag != TS_DESCRIPTOR_TAG_SERVICE_DESCRIPTOR)
	{
		TS_DBG("service_descriptor: bad tag (%#x)", dr->descriptor_tag);
		return NULL;
	}

	// don't decode twice
	if (dr->decoded_dr) return dr->decoded_dr;

	// check the length
	if (dr->descriptor_length < 3)
	{
		TS_DBG("service_descriptor: bad length (%d)", dr->descriptor_length);
		return NULL;
	}

	// check data
	if (!dr->data)
	{
		TS_DBG("service_descriptor: invalid data");
		return NULL;
	}

	// allocate memory
	p = (ts_service_descriptor_t*)malloc(sizeof(ts_service_descriptor_t));
	if (!p)
	{
		TS_DBG("service_descriptor: out of memory");
		return NULL;
	}
	
	// save pointer for freeing memory
	dr->decoded_dr = (void*)p;

	// init data
	p->service_type					= dr->data[0];
	p->service_provider_name_length	= 0;
	p->service_name_length			= 0;
	p->service_provider_name[0]		= '\0';
	p->service_name[0]				= '\0';

	// decode length and check it
	p->service_provider_name_length	= dr->data[1];
	if (p->service_provider_name_length + 2 > dr->descriptor_length) return p;
	if (p->service_provider_name_length >= TS_DESCRIPTOR_SERVICE_NAME_MAX_SIZE) p->service_provider_name_length = TS_DESCRIPTOR_SERVICE_NAME_MAX_SIZE - 1;

	// decode service provider name
	if (p->service_provider_name_length) memcpy(p->service_provider_name, dr->data + 2, p->service_provider_name_length);
	p->service_provider_name[p->service_provider_name_length] = '\0';

	// decode length and check it
	if (p->service_provider_name_length + 3 > dr->descriptor_length) return p;
	p->service_name_length = dr->data[2 + p->service_provider_name_length];
	if (p->service_provider_name_length + 3 + p->service_name_length > dr->descriptor_length) return p;
	if (p->service_name_length >= TS_DESCRIPTOR_SERVICE_NAME_MAX_SIZE) p->service_name_length = TS_DESCRIPTOR_SERVICE_NAME_MAX_SIZE - 1;
	
	// decode service_name
	if (p->service_name_length) memcpy(p->service_name, dr->data + 3 + p->service_provider_name_length, p->service_name_length);
	p->service_name[p->service_name_length] = '\0';
	
	return p;
}

