#ifndef TS_PSI_DESCRIPTOR_0x48_H
#define TS_PSI_DESCRIPTOR_0x48_H

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

// service descriptior tag type
#define TS_DESCRIPTOR_TAG_SERVICE_DESCRIPTOR	(0x48)

// name maximum size
#define TS_DESCRIPTOR_SERVICE_NAME_MAX_SIZE		(256)

/* ////////////////////////////////////////////////////////////////////////
 * types
 */

// service descriptior type
typedef struct __ts_service_descriptor_t
{
	tb_uint_t						service_type;
	tb_size_t						service_provider_name_length;
	tb_char_t						service_provider_name[TS_DESCRIPTOR_SERVICE_NAME_MAX_SIZE];
	tb_size_t						service_name_length;
	tb_char_t						service_name[TS_DESCRIPTOR_SERVICE_NAME_MAX_SIZE];

}ts_service_descriptor_t;

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */
ts_service_descriptor_t*		ts_service_descriptor_decode(ts_descriptor_t* dr);

#	ifdef __cplusplus
}
#	endif

#endif

