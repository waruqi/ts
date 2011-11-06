#ifndef TS_PSI_DESCRIPTOR_0x4d_H
#define TS_PSI_DESCRIPTOR_0x4d_H

#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"

/* ////////////////////////////////////////////////////////////////////////
 * macros
 */

// short event descriptior tag type
#define TS_DESCRIPTOR_TAG_SHORT_EVENT_DESCRIPTOR	(0x4d)

// name maximum size
#define TS_DESCRIPTOR_EVENT_NAME_MAX_SIZE			(256)
#define TS_DESCRIPTOR_EVENT_TEXT_MAX_SIZE			(256)

/* ////////////////////////////////////////////////////////////////////////
 * types
 */

// short event descriptior type
typedef struct __ts_short_event_descriptor_t
{
	tb_char_t		iso_639_code[4];	//!< e.g. code:"fre" + '\0'
	tb_size_t		event_name_length;
	tb_char_t		event_name[TS_DESCRIPTOR_EVENT_NAME_MAX_SIZE];
	tb_size_t		text_length;
	tb_char_t		text[TS_DESCRIPTOR_EVENT_TEXT_MAX_SIZE];

}ts_short_event_descriptor_t;

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */
ts_short_event_descriptor_t*		ts_short_event_descriptor_decode(ts_descriptor_t* dr);

#	ifdef __cplusplus
}
#	endif

#endif

