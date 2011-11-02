#ifndef TS_PSI_DESCRIPTOR_0x41_H
#define TS_PSI_DESCRIPTOR_0x41_H

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

// service list descriptior tag type
#define TS_DESCRIPTOR_TAG_SERVICE_LIST_DESCRIPTOR	(0x41)

/* ////////////////////////////////////////////////////////////////////////
 * types
 */

// service list descriptior type
typedef struct __ts_service_list_descriptor_t
{
	ts_uint_t								service_id;
	ts_uint_t								service_type;

	struct __ts_service_list_descriptor_t*	next;

}ts_service_list_descriptor_t;

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */
ts_service_list_descriptor_t*		ts_service_list_descriptor_decode(ts_descriptor_t* dr);
void								ts_service_list_descriptor_destroy(ts_service_list_descriptor_t* dr);


#	ifdef __cplusplus
}
#	endif

#endif

