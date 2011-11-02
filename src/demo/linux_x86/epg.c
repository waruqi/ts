#include "ts.h"
#include "stdio.h"

int main(int argc, char** argv)
{
	ts_int_t fd = open(argv[1], 0);
	if (fd != -1)
	{
		ts_packet_t				ts_packet;
		ts_epg_decoder_t*		epg_decoder		= ts_epg_create_decoder();

		// read packet
		while (TS_TRUE == ts_read_packet(fd, &ts_packet))
			ts_epg_decode(&ts_packet, epg_decoder);

		ts_epg_destroy_decoder(epg_decoder);

		close(fd);
	}


	return 0;
}

