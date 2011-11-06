/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "0x4d.h"

/* //////////////////////////////////////////////////////////////////
 * macros
 */

/* //////////////////////////////////////////////////////////////////
 * implemention
 */

// decode short event a descriptor
ts_short_event_descriptor_t* ts_short_event_descriptor_decode(ts_descriptor_t* dr)
{
	ts_assert(dr);
	if (!dr) return NULL;
	ts_short_event_descriptor_t* p = NULL;

	// check the tag
	if (dr->descriptor_tag != TS_DESCRIPTOR_TAG_SHORT_EVENT_DESCRIPTOR)
	{
		ts_trace("short_event_descriptor: bad tag (%#x)", dr->descriptor_tag);
		return NULL;
	}

	// don't decode twice
	if (dr->decoded_dr) return dr->decoded_dr;

	// check the length
	if (dr->descriptor_length < 5)
	{
		ts_trace("short_event_descriptor: bad length (%d)", dr->descriptor_length);
		return NULL;
	}

	// check data
	if (!dr->data)
	{
		ts_trace("short_event_descriptor: invalid data");
		return NULL;
	}

	// allocate memory
	p = (ts_short_event_descriptor_t*)malloc(sizeof(ts_short_event_descriptor_t));
	if (!p)
	{
		ts_trace("short_event_descriptor: out of memory");
		return NULL;
	}
	
	// save pointer for freeing memory
	dr->decoded_dr = (void*)p;

	// decode iso_639_code
	memcpy(p->iso_639_code, dr->data, 3);
	p->iso_639_code[3] = '\0';

	// init data
	p->event_name_length	= 0;
	p->text_length			= 0;
	p->event_name[0]		= '\0';
	p->text[0]				= '\0';

	// decode length and check it
	p->event_name_length = dr->data[3];
	if (p->event_name_length + 4 > dr->descriptor_length) return p;
	if (p->event_name_length >= TS_DESCRIPTOR_EVENT_NAME_MAX_SIZE) p->event_name_length = TS_DESCRIPTOR_EVENT_NAME_MAX_SIZE - 1;

	// decode event name
	if (p->event_name_length) memcpy(p->event_name, dr->data + 4, p->event_name_length);
	p->event_name[p->event_name_length] = '\0';

	// decode length and check it
	if (p->event_name_length + 5 > dr->descriptor_length) return p;
	p->text_length = dr->data[4 + p->event_name_length];
	if (p->text_length + p->event_name_length + 5 > dr->descriptor_length) return p;
	if (p->text_length >= TS_DESCRIPTOR_EVENT_TEXT_MAX_SIZE) p->text_length = TS_DESCRIPTOR_EVENT_TEXT_MAX_SIZE - 1;

	// decode event text
	if (p->text_length) memcpy(p->event_name, dr->data + 4, p->text_length);
	p->text[p->text_length] = '\0';
	
	return p;
}

