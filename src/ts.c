// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "ts.h"

/* ////////////////////////////////////////////////////////////////////////
 * implementions
 */
ts_bool_t ts_impl_decode_header(ts_packet_t* ts_packet, ts_decoder_t* ts_decoder)
{
	TS_ASSERT(ts_packet && ts_decoder);
	if (!ts_packet || !ts_decoder) return TS_FALSE;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	ts_byte_t* p = ts_payload->data;
	ts_header->sync_byte					= ts_get_bits(p, 0, 0, 8);
	ts_header->transport_error_indicator	= ts_get_bits(p, 0, 8, 1);
	ts_header->payload_unit_start_indicator	= ts_get_bits(p, 0, 9, 1);
	ts_header->transport_priority			= ts_get_bits(p, 0, 10, 1);
	ts_header->pid							= ts_get_bits(p, 0, 11, 13);
	ts_header->transport_scrambling_control	= ts_get_bits(p, 0, 24, 2);
	ts_header->adaptation_field_control		= ts_get_bits(p, 0, 26, 2);
	ts_header->continuity_counter			= ts_get_bits(p, 0, 28, 4);

	// update the decoded position
	ts_payload->data += TS_HEADER_SIZE;
	ts_payload->size -= TS_HEADER_SIZE;

	return TS_TRUE;
}
ts_bool_t ts_impl_decode_adaptation(ts_packet_t* ts_packet, ts_decoder_t* ts_decoder)
{
	TS_ASSERT(ts_packet && ts_decoder);
	if (!ts_packet || !ts_decoder) return TS_FALSE;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	ts_byte_t* p = ts_payload->data;

	// skip adaptation
	if (TS_TRUE == ts_decoder->skip_adaptation)
	{
		ts_adaptation->adaptation_field_length == 0;
		goto end;
	}

	// decode adaptation_field_length
	ts_adaptation->adaptation_field_length					= ts_get_bits(p, 0, 0, 8);
	if (ts_adaptation->adaptation_field_length == 0) goto end;
	p++;

	// decode indicators
	ts_adaptation->discontinuity_indicator					= ts_get_bits(p, 0, 0, 1);
	ts_adaptation->random_access_indicator					= ts_get_bits(p, 0, 1, 1);
	ts_adaptation->elementary_stream_priority_indicator		= ts_get_bits(p, 0, 2, 1);

	// decode flags
	ts_adaptation->pcr_flag									= ts_get_bits(p, 0, 3, 1);
	ts_adaptation->opcr_flag								= ts_get_bits(p, 0, 4, 1);
	ts_adaptation->splicing_point_flag						= ts_get_bits(p, 0, 5, 1);
	ts_adaptation->transport_private_data_flag				= ts_get_bits(p, 0, 6, 1);
	ts_adaptation->adaptation_field_extension_flag			= ts_get_bits(p, 0, 7, 1);
	p++;

	// decode pcr
	if (ts_adaptation->pcr_flag)
	{
		ts_uint64_t pcr_h			= ts_get_bits(p, 0, 0, 1);
		ts_uint64_t pcr_l			= ts_get_bits(p, 0, 1, 32);
		// reversed					= ts_get_bits(p, 0, 33, 6);
		ts_adaptation->pcr_ext		= ts_get_bits(p, 0, 39, 9);
		ts_adaptation->pcr_base		= (pcr_h << 32) + pcr_l;

		p += 6;
	}

	// decode opcr
	if (ts_adaptation->pcr_flag)
	{
		ts_uint64_t opcr_h			= ts_get_bits(p, 0, 0, 1);
		ts_uint64_t opcr_l			= ts_get_bits(p, 0, 1, 32);
		// reversed					= ts_get_bits(p, 0, 33, 6);
		ts_adaptation->opcr_ext		= ts_get_bits(p, 0, 39, 9);
		ts_adaptation->opcr_base	= (opcr_h << 32) + opcr_l;

		p += 6;
	}

	// decode splicing point
	if (ts_adaptation->splicing_point_flag)
	{
		ts_adaptation->splice_countdown	= ts_get_bits(p, 0, 0, 8);
		p++;
	}

	// decode private_data
	if (ts_adaptation->transport_private_data_flag)
	{
		ts_adaptation->transport_private_data_length	= ts_get_bits(p, 0, 0, 8);
		ts_adaptation->private_data_byte				= NULL;		//!< \note have not been decoded
		p += ts_adaptation->transport_private_data_length + 1;
	}

	// decode adaptation field extension
	if (ts_adaptation->adaptation_field_extension_flag)
	{
		ts_byte_t* pe = p;

		// decode extension length
		ts_adaptation->adaptation_field_extension_length	= ts_get_bits(pe, 0, 0, 8);
		pe++;

		if (ts_adaptation->adaptation_field_extension_length > 0)
		{
			// decode flags
			ts_adaptation->ltw_flag					= ts_get_bits(pe, 0, 0, 1);
			ts_adaptation->piecewise_rate_flag		= ts_get_bits(pe, 0, 1, 1);
			ts_adaptation->seamless_splice_flag		= ts_get_bits(pe, 0, 2, 1);
			//reversed 								= ts_get_bits(pe, 0, 3, 5);
			pe++;

			// decode ltw
			if (ts_adaptation->ltw_flag)
			{
				ts_adaptation->ltw_valid_flag		= ts_get_bits(pe, 0, 0, 1);
				ts_adaptation->ltw_offset			= ts_get_bits(pe, 0, 1, 15);
				pe += 2;
			}

			// decode piecewise
			if (ts_adaptation->piecewise_rate_flag)
			{
				//reversed							= ts_get_bits(pe, 0, 0, 2);
				ts_adaptation->piecewise_rate		= ts_get_bits(pe, 0, 2, 22);
				pe += 3;
			}

			// decode seamless splice
			if (ts_adaptation->seamless_splice_flag)
			{
				ts_adaptation->splice_type			= ts_get_bits(pe, 0, 0, 4);

				{	// this is basically the same as for PES
					ts_uint64_t xTS_32_30		= ts_get_bits(pe, 0, 4, 3);
					// marker_bit 				= ts_get_bits(pe, 0, 7, 1);
					ts_uint64_t xTS_29_15		= ts_get_bits(pe, 0, 8, 15);
					// marker_bit 				= ts_get_bits(pe, 0, 23, 1);
					ts_uint64_t xTS_14_0		= ts_get_bits(pe, 0, 24, 15);
					// marker_bit 				= ts_get_bits(pe, 0, 39, 1);

					// 90kHz, using ts_dump_pcr_base(sdts_next_au)
					ts_adaptation->dts_next_au = (xTS_32_30 << 30) + (xTS_29_15 << 15) + xTS_14_0;

					pe += 5;
				}
			}

			p += ts_adaptation->adaptation_field_extension_length + 1;
		}
		else
		{
			p++;
		}
	}

end:
	// update the decoded position
	ts_payload->data += ts_adaptation->adaptation_field_length + 1;
	ts_payload->size -= ts_adaptation->adaptation_field_length + 1;

	return TS_TRUE;
}
/* ////////////////////////////////////////////////////////////////////////
 * dump info
 */

// pcr_base: 90kHz pcr_ext: 27MHz
void ts_dump_pcr(ts_uint64_t pcr_base, ts_uint_t pcr_ext)
{
	ts_uint64_t ull = pcr_base * 300 + pcr_ext;

	ts_uint_t h ,m, s;
	ts_long_t u;
	ts_uint64_t p = ull / 27;
	ts_uint64_t fa = 1000000;

	// -- following lines basically taken from "dvbtextsubs Dave Chapman"
	h = (p / (fa * 60 * 60));
	m = (p / (fa * 60)) - (h * 60);
	s = (p / fa) - (h * 3600) - (m * 60);
	u = p - (h * fa * 60 * 60) - (m * fa * 60) - (s * fa);

	TS_PRINT("pcr: %d:%02d:%02d.%06ld - %llu (0x%08llx)", h, m, s, u, ull, ull);
}
// pcr_base: 90kHz
void ts_dump_pcr_base(ts_uint64_t pcr_base)
{
	ts_uint64_t ull = pcr_base;

   	ts_uint_t h, m, s, u;
	ts_uint64_t p = ull / 9;

	// -- following lines basically taken from "dvbtextsubs Dave Chapman"
	h = (p / (10000L * 60 * 60));
	m = (p / (10000L * 60)) - (h * 60);
	s = (p / 10000L) - (h * 3600) - (m * 60);
	u = p - (h * 10000L * 60 * 60) - (m * 10000L * 60) - (s * 10000L);

	TS_PRINT("pcr: %d:%02d:%02d.%04d - %llu (0x%08llx)", h, m, s, u, ull, ull);
}
void ts_dump_header(ts_packet_t* ts_packet)
{
	TS_ASSERT(ts_packet);
	if (!ts_packet) return ;
	ts_header_t* ts_header = &(ts_packet->header);

	TS_PRINT("----------------------------------------"											);
	TS_PRINT("transport header:"																);
	TS_PRINT("----------------------------------------"											);
	TS_PRINT("sync_byte: %#x",							ts_header->sync_byte					);
	TS_PRINT("transport_error_indicator: %u",			ts_header->transport_error_indicator	);
	TS_PRINT("payload_unit_start_indicator: %u",		ts_header->payload_unit_start_indicator	);
	TS_PRINT("transport_priority: %u",					ts_header->transport_priority			);
	TS_PRINT("pid: %#x",								ts_header->pid							);
	TS_PRINT("transport_scrambling_control: %u",		ts_header->transport_scrambling_control	);
	TS_PRINT("adaptation_field_control: %u",			ts_header->adaptation_field_control		);
	TS_PRINT("continuity_counter: %u",					ts_header->continuity_counter			);
	TS_PRINT("========================================\n"										);
}
void ts_dump_adaptation(ts_packet_t* ts_packet)
{
	TS_ASSERT(ts_packet);
	if (!ts_packet) return ;
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);

	TS_PRINT("----------------------------------------"																);
	TS_PRINT("adaptationfield header:"																				);
	TS_PRINT("----------------------------------------"																);
	TS_PRINT("adaptation_field_length: %u",					ts_adaptation->adaptation_field_length					);
	TS_PRINT("discontinuity_indicator: %u",					ts_adaptation->discontinuity_indicator					);
	TS_PRINT("random_access_indicator: %u",					ts_adaptation->random_access_indicator					);
	TS_PRINT("elementary_stream_priority_indicator: %u", 	ts_adaptation->elementary_stream_priority_indicator		);
	TS_PRINT("pcr_flag: %u",								ts_adaptation->pcr_flag									);
	TS_PRINT("opcr_flag: %u",								ts_adaptation->opcr_flag								);
	TS_PRINT("splicing_point_flag: %u",						ts_adaptation->splicing_point_flag						);
	TS_PRINT("transport_private_data_flag: %u",				ts_adaptation->transport_private_data_flag				);
	TS_PRINT("extension_flag: %u",							ts_adaptation->adaptation_field_extension_flag			);
	TS_PRINT("========================================\n"															);

	// dump others
	// ...
}
/* ////////////////////////////////////////////////////////////////////////
 * helpers
 */
ts_bool_t ts_exists_adaptation(ts_packet_t* ts_packet)
{
	TS_ASSERT(ts_packet);
	if (!ts_packet) return TS_FALSE;
	return ((ts_packet->header.adaptation_field_control == 0x2) || (ts_packet->header.adaptation_field_control == 0x3))? TS_TRUE : TS_FALSE;
}
ts_bool_t ts_exists_payload(ts_packet_t* ts_packet)
{
	TS_ASSERT(ts_packet);
	if (!ts_packet) return TS_FALSE;
	return ((ts_packet->header.adaptation_field_control == 0x1) || (ts_packet->header.adaptation_field_control == 0x3))? TS_TRUE : TS_FALSE;
}

/* ////////////////////////////////////////////////////////////////////////
 * implemention
 */
ts_decoder_t* ts_create_decoder()
{
	ts_decoder_t* ts_decoder = (ts_decoder_t*)malloc(sizeof(ts_decoder_t));
	TS_ASSERT(ts_decoder);
	ts_decoder->skip_adaptation = TS_FALSE;

	return ts_decoder;
}

void ts_destroy_decoder(ts_decoder_t* ts_decoder)
{
	if (ts_decoder) free(ts_decoder);
}

void ts_init_packet(ts_packet_t* ts_packet)
{
	TS_ASSERT(ts_packet);
	if (!ts_packet) return ;
	memset(ts_packet, 0, sizeof(ts_packet_t));

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	// init ts data
	ts_data->size = TS_PACKET_SIZE;

	// init ts payload
	ts_payload->data = ts_data->data;
	ts_payload->size = ts_data->size;

	// init pointer filed
	ts_packet->pointer_field = 0;
}
ts_bool_t ts_read_packet(ts_int_t fd, ts_packet_t* ts_packet)
{
	TS_ASSERT(ts_packet);
	TS_ASSERT(fd >= 0);
	if (fd < 0 || !ts_packet) return TS_FALSE;

	// init packet
	ts_init_packet(ts_packet);

	// ts data
	ts_data_t*	ts_data = &(ts_packet->data);

	// read the whole ts packet
	ts_int_t i = TS_PACKET_SIZE - 1;
	ts_int_t i_rc = 1;

	ts_data->data[0] = 0;
	while((ts_data->data[0] != TS_SYNC_BYTE) && (i_rc > 0))
		i_rc = read(fd, ts_data->data, 1);

	while((i != 0) && (i_rc > 0))
	{
		i_rc = read(fd, ts_data->data + TS_PACKET_SIZE - i, i);
		if (i_rc >= 0) i -= i_rc;
	}

	return (i == 0) ? TS_TRUE : TS_FALSE;
}

ts_bool_t	ts_decode_packet(ts_packet_t* ts_packet, ts_decoder_t* ts_decoder)
{
	TS_ASSERT(ts_packet);
	if (!ts_packet) return TS_FALSE;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	//ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	// decode header
	if (TS_FALSE == ts_impl_decode_header(ts_packet, ts_decoder)
	||	ts_header->sync_byte != TS_SYNC_BYTE)
	{
		TS_DBG("cannot decode ts header");
		return TS_FALSE;
	}

	// decode adaptation
	if (TS_TRUE == ts_exists_adaptation(ts_packet)
	&&	TS_FALSE == ts_impl_decode_adaptation(ts_packet, ts_decoder))
	{
		TS_DBG("cannot decode ts adaptation field");
			return TS_FALSE;
	}

	// decode payload
	if (TS_FALSE == ts_exists_payload(ts_packet))
	{
		ts_payload->data = NULL;
		ts_payload->size = 0;
		return TS_TRUE;
	}

	// decode pointer field
	if (ts_header->payload_unit_start_indicator)
	{
		ts_packet->pointer_field = *(ts_payload->data);
		ts_payload->data++;
		ts_payload->size--;
		ts_payload->data += ts_packet->pointer_field;
		ts_payload->size -= ts_packet->pointer_field;
	}

	return TS_TRUE;
}
// extern "C" {
#	ifdef __cplusplus
}
#	endif
