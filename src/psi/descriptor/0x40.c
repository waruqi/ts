/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "0x40.h"

/* //////////////////////////////////////////////////////////////////
 * macros
 */

/* //////////////////////////////////////////////////////////////////
 * implemention
 */

// decode network name descriptor
ts_network_name_descriptor_t* ts_network_name_descriptor_decode(ts_descriptor_t* dr)
{
	ts_assert(dr);
	if (!dr) return NULL;

	// check the tag
	if (dr->descriptor_tag != TS_DESCRIPTOR_TAG_NETWORK_NAME_DESCRIPTOR)
	{
		ts_trace("network name descriptor: bad tag (%#x)", dr->descriptor_tag);
		return NULL;
	}

	// don't decode twice
	if (dr->decoded_dr) return dr->decoded_dr;

	// check the length
	if (dr->descriptor_length <= 0)
	{
		ts_trace("network name descriptor: bad length (%d)", dr->descriptor_length);
		return NULL;
	}

	// check data
	if (!dr->data)
	{
		ts_trace("network name descriptor: invalid data");
		return NULL;
	}

	// allocate memory
	ts_network_name_descriptor_t* p = (ts_network_name_descriptor_t*)malloc(sizeof(ts_network_name_descriptor_t));
	if (!p)
	{
		ts_trace("network name descriptor: out of memory");
		return NULL;
	}

	// save pointer for freeing memory
	dr->decoded_dr = (void*)p;

	// decode data
	tb_size_t n = dr->descriptor_length;
	if (n > TS_DESCRIPTOR_NETWORK_NAME_MAX_SIZE - 1) n = TS_DESCRIPTOR_NETWORK_NAME_MAX_SIZE - 1;
	memcpy(p->name, dr->data, n);
	p->name[n] = '\0';
	
	return p;
}

