/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "descriptor.h"

/* //////////////////////////////////////////////////////////////////
 * inner implemention
 */
// create a descriptor
ts_descriptor_t* ts_descriptor_create_descriptor(tb_byte_t descriptor_tag, tb_byte_t descriptor_length, tb_byte_t* data)
{
	// allocate descriptor
	ts_descriptor_t* descriptor = (ts_descriptor_t*)malloc(sizeof(ts_descriptor_t));
	if (descriptor)
	{
		descriptor->data = (tb_byte_t*)malloc(descriptor_length * sizeof(tb_byte_t));
		if (descriptor->data)
		{
			// init header
			descriptor->descriptor_tag		= descriptor_tag;
			descriptor->descriptor_length	= descriptor_length;

			// init data
			if (data) memcpy(descriptor->data, data, descriptor_length);

			// init others
			descriptor->decoded_dr = NULL;
			descriptor->next = NULL;
		}
		else
		{
			// free it if failed
			free(descriptor);
			descriptor = NULL;
		}
	}

	return descriptor;
}
// destroy descriptors
void ts_descriptor_destroy_descriptors(ts_descriptor_t* descriptor_list)
{
	ts_descriptor_t* descriptor = descriptor_list;
	while (descriptor)
	{
		ts_descriptor_t* next = descriptor->next;
		if (descriptor->data) free(descriptor->data);
		if (descriptor->decoded_dr) free(descriptor->decoded_dr);

		free(descriptor);
		descriptor = next;
	}
}


