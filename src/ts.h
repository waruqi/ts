#ifndef TS_H
#define TS_H

// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "psi/psi.h"
#include "epg.h"

/* ////////////////////////////////////////////////////////////////////////
 * interfaces
 */

// packet operations
void				ts_init_packet(ts_packet_t* ts_packet);
tb_bool_t			ts_read_packet(tb_int_t fd, ts_packet_t* ts_packet);
tb_bool_t			ts_decode_packet(ts_packet_t* ts_packet, ts_decoder_t* ts_decoder);

// decoder operations
ts_decoder_t*		ts_create_decoder();
void				ts_destroy_decoder(ts_decoder_t* ts_decoder);

// dump info
void				ts_dump_header(ts_packet_t* ts_packet);
void				ts_dump_adaptation(ts_packet_t* ts_packet);
void				ts_dump_pcr(tb_uint64_t pcr_base, tb_uint_t pcr_ext);
void				ts_dump_pcr_base(tb_uint64_t pcr_base);

// helpers
tb_bool_t			ts_exists_adaptation(ts_packet_t* ts_packet);
tb_bool_t			ts_exists_payload(ts_packet_t* ts_packet);

// extern "C" {
#	ifdef __cplusplus
}
#	endif
#endif
