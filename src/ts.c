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
tb_bool_t ts_impl_decode_header(ts_packet_t* ts_packet, ts_decoder_t* ts_decoder)
{
	tb_assert(ts_packet && ts_decoder);
	if (!ts_packet || !ts_decoder) return TB_FALSE;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	tb_byte_t* p = ts_payload->data;
	ts_header->sync_byte					= tb_bits_get_ubits32(p, 0, 8);
	ts_header->transport_error_indicator	= tb_bits_get_ubits32(p, 8, 1);
	ts_header->payload_unit_start_indicator	= tb_bits_get_ubits32(p, 9, 1);
	ts_header->transport_priority			= tb_bits_get_ubits32(p, 10, 1);
	ts_header->pid							= tb_bits_get_ubits32(p, 11, 13);
	ts_header->transport_scrambling_control	= tb_bits_get_ubits32(p, 24, 2);
	ts_header->adaptation_field_control		= tb_bits_get_ubits32(p, 26, 2);
	ts_header->continuity_counter			= tb_bits_get_ubits32(p, 28, 4);

	// update the decoded position
	ts_payload->data += TS_HEADER_SIZE;
	ts_payload->size -= TS_HEADER_SIZE;

	return TB_TRUE;
}
tb_bool_t ts_impl_decode_adaptation(ts_packet_t* ts_packet, ts_decoder_t* ts_decoder)
{
	tb_assert(ts_packet && ts_decoder);
	if (!ts_packet || !ts_decoder) return TB_FALSE;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	tb_byte_t* p = ts_payload->data;

	// skip adaptation
	if (TB_TRUE == ts_decoder->skip_adaptation)
	{
		ts_adaptation->adaptation_field_length == 0;
		goto end;
	}

	// decode adaptation_field_length
	ts_adaptation->adaptation_field_length					= tb_bits_get_ubits32(p, 0, 8);
	if (ts_adaptation->adaptation_field_length == 0) goto end;
	p++;

	// decode indicators
	ts_adaptation->discontinuity_indicator					= tb_bits_get_ubits32(p, 0, 1);
	ts_adaptation->random_access_indicator					= tb_bits_get_ubits32(p, 1, 1);
	ts_adaptation->elementary_stream_priority_indicator		= tb_bits_get_ubits32(p, 2, 1);

	// decode flags
	ts_adaptation->pcr_flag									= tb_bits_get_ubits32(p, 3, 1);
	ts_adaptation->opcr_flag								= tb_bits_get_ubits32(p, 4, 1);
	ts_adaptation->splicing_point_flag						= tb_bits_get_ubits32(p, 5, 1);
	ts_adaptation->transport_private_data_flag				= tb_bits_get_ubits32(p, 6, 1);
	ts_adaptation->adaptation_field_extension_flag			= tb_bits_get_ubits32(p, 7, 1);
	p++;

	// decode pcr
	if (ts_adaptation->pcr_flag)
	{
		tb_uint32_t pcr_h			= tb_bits_get_ubits32(p, 0, 1);
		tb_uint32_t pcr_l			= tb_bits_get_ubits32(p, 1, 32);
		// reversed					= tb_bits_get_ubits32(p, 33, 6);
		ts_adaptation->pcr_ext		= tb_bits_get_ubits32(p, 39, 9);
		ts_adaptation->pcr_base		= tb_uint64_make(pcr_h, pcr_l);

		p += 6;
	}

	// decode opcr
	if (ts_adaptation->pcr_flag)
	{
		tb_uint32_t opcr_h			= tb_bits_get_ubits32(p, 0, 1);
		tb_uint32_t opcr_l			= tb_bits_get_ubits32(p, 1, 32);
		// reversed					= tb_bits_get_ubits32(p, 33, 6);
		ts_adaptation->opcr_ext		= tb_bits_get_ubits32(p, 39, 9);
		ts_adaptation->opcr_base	= tb_uint64_make(opcr_h, opcr_l);

		p += 6;
	}

	// decode splicing point
	if (ts_adaptation->splicing_point_flag)
	{
		ts_adaptation->splice_countdown	= tb_bits_get_ubits32(p, 0, 8);
		p++;
	}

	// decode private_data
	if (ts_adaptation->transport_private_data_flag)
	{
		ts_adaptation->transport_private_data_length	= tb_bits_get_ubits32(p, 0, 8);
		ts_adaptation->private_data_byte				= NULL;		//!< \note have not been decoded
		p += ts_adaptation->transport_private_data_length + 1;
	}

	// decode adaptation field extension
	if (ts_adaptation->adaptation_field_extension_flag)
	{
		tb_byte_t* pe = p;

		// decode extension length
		ts_adaptation->adaptation_field_extension_length	= tb_bits_get_ubits32(pe, 0, 8);
		pe++;

		if (ts_adaptation->adaptation_field_extension_length > 0)
		{
			// decode flags
			ts_adaptation->ltw_flag					= tb_bits_get_ubits32(pe, 0, 1);
			ts_adaptation->piecewise_rate_flag		= tb_bits_get_ubits32(pe, 1, 1);
			ts_adaptation->seamless_splice_flag		= tb_bits_get_ubits32(pe, 2, 1);
			//reversed 								= tb_bits_get_ubits32(pe, 3, 5);
			pe++;

			// decode ltw
			if (ts_adaptation->ltw_flag)
			{
				ts_adaptation->ltw_valid_flag		= tb_bits_get_ubits32(pe, 0, 1);
				ts_adaptation->ltw_offset			= tb_bits_get_ubits32(pe, 1, 15);
				pe += 2;
			}

			// decode piecewise
			if (ts_adaptation->piecewise_rate_flag)
			{
				//reversed							= tb_bits_get_ubits32(pe, 0, 2);
				ts_adaptation->piecewise_rate		= tb_bits_get_ubits32(pe, 2, 22);
				pe += 3;
			}

			// decode seamless splice
			if (ts_adaptation->seamless_splice_flag)
			{
				ts_adaptation->splice_type			= tb_bits_get_ubits32(pe, 0, 4);

				{	// this is basically the same as for PES
					tb_uint32_t xTS_32_30		= tb_bits_get_ubits32(pe, 4, 3);
					// marker_bit 				= tb_bits_get_ubits32(pe, 7, 1);
					tb_uint32_t xTS_29_15		= tb_bits_get_ubits32(pe, 8, 15);
					// marker_bit 				= tb_bits_get_ubits32(pe, 23, 1);
					tb_uint32_t xTS_14_0		= tb_bits_get_ubits32(pe, 24, 15);
					// marker_bit 				= tb_bits_get_ubits32(pe, 39, 1);

					// 90kHz, using ts_dump_pcr_base(sdts_next_au)
					// ts_adaptation->dts_next_au = (xTS_32_30 << 30) + (xTS_29_15 << 15) + xTS_14_0;
					ts_adaptation->dts_next_au = tb_uint32_to_uint64(xTS_14_0);
					ts_adaptation->dts_next_au = tb_uint64_add(ts_adaptation->dts_next_au, tb_uint64_lsh(tb_uint32_to_uint64(xTS_29_15), 15));
					ts_adaptation->dts_next_au = tb_uint64_add(ts_adaptation->dts_next_au, tb_uint64_lsh(tb_uint32_to_uint64(xTS_32_30), 30));

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

	return TB_TRUE;
}
/* ////////////////////////////////////////////////////////////////////////
 * dump info
 */

// pcr_base: 90kHz pcr_ext: 27MHz
void ts_dump_pcr(tb_uint64_t pcr_base, tb_uint_t pcr_ext)
{
	/*
	 * p = (pcr_base * 300 + pcr_ext) / 27;
	 * h = (p / (1000000 * 60 * 60));
	 * m = (p / (1000000 * 60)) - (h * 60);
	 * s = (p / 1000000) - (h * 3600) - (m * 60);
	 * u = p - (h * 1000000 * 60 * 60) - (m * 1000000 * 60) - (s * 1000000);
	 */
	tb_uint64_t p = tb_uint64_div_uint32(tb_uint64_add_uint32(tb_uint64_mul_uint32(pcr_base, 300), pcr_ext), 27);
	tb_uint64_t b = tb_uint32_to_uint64(1000000 * 60);

	tb_size_t h = tb_uint64_to_uint32(tb_uint64_div(p, tb_uint64_mul_uint32(b, 60)));
	tb_size_t m = tb_uint64_to_uint32(tb_uint64_div(p, b)) - (h * 60);
	tb_size_t s = tb_uint64_to_uint32(tb_uint64_div_uint32(p, 1000000)) - (h * 3600) - (m * 60);
	tb_size_t u = tb_uint64_to_uint32(tb_uint64_sub(tb_uint64_sub(tb_uint64_sub(p, tb_uint64_mul_uint32(b, h * 60)), tb_uint64_mul_uint32(b, m)), tb_uint64_mul_uint32(tb_uint32_to_uint64(s), 1000000)));

	tb_print("pcr: %u:%02u:%02u.%06u", h, m, s, u);
}
// pcr_base: 90kHz
void ts_dump_pcr_base(tb_uint64_t pcr_base)
{
	/*
	 * p = pcr_base / 9;
	 * h = (p / (10000 * 60 * 60));
	 * m = (p / (10000 * 60)) - (h * 60);
	 * s = (p / 10000) - (h * 3600) - (m * 60);
	 * u = p - (h * 10000 * 60 * 60) - (m * 10000 * 60) - (s * 10000);
	 */
	tb_uint64_t p = tb_uint64_div_uint32(pcr_base, 9);
	tb_uint64_t b = tb_uint32_to_uint64(10000 * 60);

	tb_size_t h = tb_uint64_to_uint32(tb_uint64_div(p, tb_uint64_mul_uint32(b, 60)));
	tb_size_t m = tb_uint64_to_uint32(tb_uint64_div(p, b)) - (h * 60);
	tb_size_t s = tb_uint64_to_uint32(tb_uint64_div_uint32(p, 10000)) - (h * 3600) - (m * 60);
	tb_size_t u = tb_uint64_to_uint32(tb_uint64_sub(tb_uint64_sub(tb_uint64_sub(p, tb_uint64_mul_uint32(b, h * 60)), tb_uint64_mul_uint32(b, m)), tb_uint64_mul_uint32(tb_uint32_to_uint64(s), 10000)));

	tb_print("pcr: %u:%02u:%02u.%06u", h, m, s, u);
}
void ts_dump_header(ts_packet_t* ts_packet)
{
	tb_assert(ts_packet);
	if (!ts_packet) return ;
	ts_header_t* ts_header = &(ts_packet->header);

	tb_print("----------------------------------------"											);
	tb_print("transport header:"																);
	tb_print("----------------------------------------"											);
	tb_print("sync_byte: %#x",							ts_header->sync_byte					);
	tb_print("transport_error_indicator: %u",			ts_header->transport_error_indicator	);
	tb_print("payload_unit_start_indicator: %u",		ts_header->payload_unit_start_indicator	);
	tb_print("transport_priority: %u",					ts_header->transport_priority			);
	tb_print("pid: %#x",								ts_header->pid							);
	tb_print("transport_scrambling_control: %u",		ts_header->transport_scrambling_control	);
	tb_print("adaptation_field_control: %u",			ts_header->adaptation_field_control		);
	tb_print("continuity_counter: %u",					ts_header->continuity_counter			);
	tb_print("========================================\n"										);
}
void ts_dump_adaptation(ts_packet_t* ts_packet)
{
	tb_assert(ts_packet);
	if (!ts_packet) return ;
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);

	tb_print("----------------------------------------"																);
	tb_print("adaptationfield header:"																				);
	tb_print("----------------------------------------"																);
	tb_print("adaptation_field_length: %u",					ts_adaptation->adaptation_field_length					);
	tb_print("discontinuity_indicator: %u",					ts_adaptation->discontinuity_indicator					);
	tb_print("random_access_indicator: %u",					ts_adaptation->random_access_indicator					);
	tb_print("elementary_stream_priority_indicator: %u", 	ts_adaptation->elementary_stream_priority_indicator		);
	tb_print("pcr_flag: %u",								ts_adaptation->pcr_flag									);
	tb_print("opcr_flag: %u",								ts_adaptation->opcr_flag								);
	tb_print("splicing_point_flag: %u",						ts_adaptation->splicing_point_flag						);
	tb_print("transport_private_data_flag: %u",				ts_adaptation->transport_private_data_flag				);
	tb_print("extension_flag: %u",							ts_adaptation->adaptation_field_extension_flag			);
	tb_print("========================================\n"															);

	// dump others
	// ...
}
/* ////////////////////////////////////////////////////////////////////////
 * helpers
 */
tb_bool_t ts_exists_adaptation(ts_packet_t* ts_packet)
{
	tb_assert(ts_packet);
	if (!ts_packet) return TB_FALSE;
	return ((ts_packet->header.adaptation_field_control == 0x2) || (ts_packet->header.adaptation_field_control == 0x3))? TB_TRUE : TB_FALSE;
}
tb_bool_t ts_exists_payload(ts_packet_t* ts_packet)
{
	tb_assert(ts_packet);
	if (!ts_packet) return TB_FALSE;
	return ((ts_packet->header.adaptation_field_control == 0x1) || (ts_packet->header.adaptation_field_control == 0x3))? TB_TRUE : TB_FALSE;
}

/* ////////////////////////////////////////////////////////////////////////
 * implemention
 */
ts_decoder_t* ts_create_decoder()
{
	ts_decoder_t* ts_decoder = (ts_decoder_t*)malloc(sizeof(ts_decoder_t));
	tb_assert(ts_decoder);
	ts_decoder->skip_adaptation = TB_FALSE;

	return ts_decoder;
}

void ts_destroy_decoder(ts_decoder_t* ts_decoder)
{
	if (ts_decoder) free(ts_decoder);
}

void ts_init_packet(ts_packet_t* ts_packet)
{
	tb_assert(ts_packet);
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
tb_bool_t ts_read_packet(tb_int_t fd, ts_packet_t* ts_packet)
{
	tb_assert(ts_packet);
	tb_assert(fd >= 0);
	if (fd < 0 || !ts_packet) return TB_FALSE;

	// init packet
	ts_init_packet(ts_packet);

	// ts data
	ts_data_t*	ts_data = &(ts_packet->data);

	// read the whole ts packet
	tb_int_t i = TS_PACKET_SIZE - 1;
	tb_int_t i_rc = 1;

	ts_data->data[0] = 0;
	while((ts_data->data[0] != TS_SYNC_BYTE) && (i_rc > 0))
		i_rc = read(fd, ts_data->data, 1);

	while((i != 0) && (i_rc > 0))
	{
		i_rc = read(fd, ts_data->data + TS_PACKET_SIZE - i, i);
		if (i_rc >= 0) i -= i_rc;
	}

	return (i == 0) ? TB_TRUE : TB_FALSE;
}

tb_bool_t	ts_decode_packet(ts_packet_t* ts_packet, ts_decoder_t* ts_decoder)
{
	tb_assert(ts_packet);
	if (!ts_packet) return TB_FALSE;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	//ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	// decode header
	if (TB_FALSE == ts_impl_decode_header(ts_packet, ts_decoder)
	||	ts_header->sync_byte != TS_SYNC_BYTE)
	{
		tb_trace("cannot decode ts header");
		return TB_FALSE;
	}

	// decode adaptation
	if (TB_TRUE == ts_exists_adaptation(ts_packet)
	&&	TB_FALSE == ts_impl_decode_adaptation(ts_packet, ts_decoder))
	{
		tb_trace("cannot decode ts adaptation field");
			return TB_FALSE;
	}

	// decode payload
	if (TB_FALSE == ts_exists_payload(ts_packet))
	{
		ts_payload->data = NULL;
		ts_payload->size = 0;
		return TB_TRUE;
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

	return TB_TRUE;
}
// extern "C" {
#	ifdef __cplusplus
}
#	endif
