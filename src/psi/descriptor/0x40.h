#ifndef TS_PSI_DESCRIPTOR_0x40_H
#define TS_PSI_DESCRIPTOR_0x40_H

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

// network name descriptior tag type
#define TS_DESCRIPTOR_TAG_NETWORK_NAME_DESCRIPTOR	(0x40)

// name maximum size
#define TS_DESCRIPTOR_NETWORK_NAME_MAX_SIZE		(256)
/* ////////////////////////////////////////////////////////////////////////
 * types
 */

// network name descriptior type
typedef struct __ts_network_name_descriptor_t
{
	ts_char_t		name[TS_DESCRIPTOR_NETWORK_NAME_MAX_SIZE];

}ts_network_name_descriptor_t;

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */
ts_network_name_descriptor_t*		ts_network_name_descriptor_decode(ts_descriptor_t* dr);
void								ts_network_name_descriptor_destroy(ts_descriptor_t* dr);

#	ifdef __cplusplus
}
#	endif

#endif

