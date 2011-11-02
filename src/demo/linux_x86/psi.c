#include "ts.h"
#include "stdio.h"

void ts_table_pat_callback(ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data)
{
	ts_table_header_t*	tb_header		= table;
	ts_table_pat_t*			pat				= (ts_table_pat_t*)table;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	ts_table_pat_dump(ts_packet, pat);
}

void ts_table_pmt_callback(ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data)
{
	ts_table_header_t*	tb_header		= table;
	ts_table_pmt_t*			pmt				= (ts_table_pmt_t*)table;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	ts_table_pmt_dump(ts_packet, pmt);
}

void ts_table_sdt_callback(ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data)
{
	ts_table_header_t*	tb_header		= table;
	ts_table_sdt_t*			sdt				= (ts_table_sdt_t*)table;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	ts_table_sdt_dump(ts_packet, sdt);
}
void ts_table_eit_callback(ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data)
{
	ts_table_header_t*	tb_header		= table;
	ts_table_eit_t*			eit				= (ts_table_eit_t*)table;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	ts_table_eit_dump(ts_packet, eit);
}
void ts_table_nit_callback(ts_packet_t* ts_packet, ts_table_header_t* table, void* cb_data)
{
	ts_table_header_t*	tb_header		= table;
	ts_table_nit_t*			nit				= (ts_table_nit_t*)table;

	ts_data_t*			ts_data			= &(ts_packet->data);
	ts_header_t*		ts_header		= &(ts_packet->header);
	ts_adaptation_t*	ts_adaptation	= &(ts_packet->adaptation);
	ts_payload_t*		ts_payload		= &(ts_packet->payload);

	ts_table_nit_dump(ts_packet, nit);
}

int main(int argc, char** argv)
{
	ts_int_t fd = open(argv[1], 0);
	if (fd != -1)
	{
		ts_packet_t					ts_packet;

		//ts_data_t*				ts_data			= &(ts_packet.data);
		ts_header_t*				ts_header		= &(ts_packet.header);
		//ts_adaptation_t*			ts_adaptation	= &(ts_packet.adaptation);
		//ts_payload_t*				ts_payload		= &(ts_packet.payload);

		ts_decoder_t*				ts_decoder		= ts_create_decoder();
		ts_table_pat_decoder_t*		tb_pat_decoder	= ts_table_pat_create_decoder(&ts_table_pat_callback, NULL);
		ts_table_pmt_decoder_t*		tb_pmt_decoder	= ts_table_pmt_create_decoder(0xa0, 9, &ts_table_pmt_callback, NULL);
		ts_table_sdt_decoder_t*		tb_sdt_decoder	= ts_table_sdt_create_decoder(&ts_table_sdt_callback, NULL);
		ts_table_eit_decoder_t*		tb_eit_decoder	= ts_table_eit_create_decoder(&ts_table_eit_callback, NULL);
		ts_table_nit_decoder_t*		tb_nit_decoder	= ts_table_eit_create_decoder(&ts_table_nit_callback, NULL);

		// read packet
		while (TS_TRUE == ts_read_packet(fd, &ts_packet))
		{
			// decode packet
			ts_decoder->skip_adaptation = TS_TRUE;
			if (TS_FALSE == ts_decode_packet(&ts_packet, ts_decoder)) continue;

			if (ts_header->pid == TS_PID_NULL) continue;
			// decode sections
			if (TS_TRUE == ts_table_decode(&ts_packet, (ts_table_decoder_t*)tb_pat_decoder)) continue;
 			if (TS_TRUE == ts_table_decode(&ts_packet, (ts_table_decoder_t*)tb_pmt_decoder)) continue;
 			if (TS_TRUE == ts_table_decode(&ts_packet, (ts_table_decoder_t*)tb_sdt_decoder)) continue;
 			if (TS_TRUE == ts_table_decode(&ts_packet, (ts_table_decoder_t*)tb_eit_decoder)) continue;
 			if (TS_TRUE == ts_table_decode(&ts_packet, (ts_table_decoder_t*)tb_nit_decoder)) continue;
		}

		ts_destroy_decoder(ts_decoder);
		ts_table_pat_destroy_decoder(tb_pat_decoder);
		ts_table_pmt_destroy_decoder(tb_pmt_decoder);
		ts_table_sdt_destroy_decoder(tb_sdt_decoder);
		ts_table_eit_destroy_decoder(tb_eit_decoder);
		ts_table_nit_destroy_decoder(tb_nit_decoder);

		close(fd);
	}

	return 0;
}

