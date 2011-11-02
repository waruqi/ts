#ifndef TS_PREFIX_H
#define TS_PREFIX_H

// c plus plus
#	ifdef __cplusplus
extern "C" {
#	endif

/* ////////////////////////////////////////////////////////////////////////
 * includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ////////////////////////////////////////////////////////////////////////
 * macros
 */

// is debug?
#if ts_CONFIG_DEBUG
# 	define TS_DEBUG
#endif

// debug
#ifdef TS_DEBUG
#	define TS_DBG(format, arg...)					do { printf("ts: " format "\n" , ## arg); } while (0)
#	define TS_ASSERT(expr)							do { if (!(expr)) printf("ts assert failed at:%d: expr: %s file: %s\n", __LINE__, #expr, __FILE__); } while(0)
#	define TS_MSG_ASSERT(expr, format, arg...)		do { if (!(expr)) printf("ts assert failed at:%d: expr: %s msg: " format " file: %s\n", __LINE__, #expr, ## arg, __FILE__); } while(0)
#else
#	define TS_DBG(format, arg...)
#	define TS_ASSERT(expr)
#	define TS_MSG_ASSERT(expr, format, arg...)
#endif
#define TS_PRINT(format, arg...)				do { printf("" format "\n" , ## arg); } while (0)

// return value
#define	TS_TRUE					((ts_bool_t)(1))
#define	TS_FALSE				((ts_bool_t)(0))

// pid
#define TS_PID(p)				(((ts_uint16_t)(p[1] & 0x1f) << 8) + p[2])
#define TS_PID_PAT				(0x0000)
#define TS_PID_CAT				(0x0001)
#define TS_PID_TSDT				(0x0002)
#define TS_PID_NIT				(0x0010)
#define TS_PID_SDT				(0x0011)
#define TS_PID_BAT				(0x0011)
#define TS_PID_EIT				(0x0012)
#define TS_PID_RST				(0x0012)
#define TS_PID_TDT				(0x0014)
#define TS_PID_TOT				(0x0014)
#define TS_PID_DIT				(0x001e)
#define TS_PID_SIT				(0x001f)
#define TS_PID_NULL				(0x1fff)

// utc time
#define TS_UTC_H(utc)			((ts_uint_t)(((utc) >> 16) & 0xff))
#define TS_UTC_M(utc)			((ts_uint_t)(((utc) >> 8) & 0xff))
#define TS_UTC_S(utc)			((ts_uint_t)((utc) & 0xff))

/*! mjd time
 *
 * algo: ETSI EN 300 468 - ANNEX C
 * \code
	y =  (long) ((mjd  - 15078.2) / 365.25);
	m =  (long) ((mjd - 14956.1 - (long)(y * 365.25) ) / 30.6001);
	d =  (long) (mjd - 14956 - (long)(y * 365.25) - (long)(m * 30.6001));
	k =  (m == 14 || m == 15) ? 1 : 0;
	y = y + k + 1900;
	m = m - 1 - k*12;
	dumpf("%02d-%02d-%02d",y,m,d);
 * \endcode
 */
#define TS_MJD_Y_(mjd)			((ts_uint_t)(((mjd)  - 15078.2) / 365.25))
#define TS_MJD_K_(m)			((ts_uint_t)(((m) == 14 || (m) == 15) ? 1 : 0))
#define TS_MJD_M_(mjd)			((ts_uint_t)(((mjd) - 14956.1 - (ts_uint_t)(TS_MJD_Y_(mjd) * 365.25) ) / 30.6001))

#define TS_MJD_Y(mjd)			((ts_uint_t)(TS_MJD_Y_(mjd) + TS_MJD_K_(TS_MJD_M_(mjd)) + 1900))
#define TS_MJD_M(mjd)			((ts_uint_t)(TS_MJD_M_(mjd) - 1 - TS_MJD_K_(TS_MJD_M_(mjd)) * 12))
#define TS_MJD_D(mjd)			((ts_uint_t)((mjd) - 14956 - (ts_uint_t)(TS_MJD_Y_(mjd) * 365.25) - (ts_uint_t)(TS_MJD_M_(mjd) * 30.6001)))

// packet constants
#define TS_PACKET_SIZE			(188)
#define TS_HEADER_SIZE			(4)
#define TS_SYNC_BYTE			(0x47)

/* ////////////////////////////////////////////////////////////////////////
 * basic types
 */
typedef signed int			ts_int_t;
typedef unsigned int		ts_uint_t;
typedef signed long			ts_long_t;
typedef unsigned long		ts_ulong_t;
typedef ts_int_t			ts_bool_t;
typedef ts_uint_t			ts_size_t;
typedef signed char			ts_int8_t;
typedef unsigned char		ts_uint8_t;
typedef signed short		ts_int16_t;
typedef unsigned short		ts_uint16_t;
typedef ts_int_t			ts_int32_t;
typedef ts_uint_t			ts_uint32_t;
typedef signed long long	ts_int64_t;
typedef unsigned long long	ts_uint64_t;
typedef ts_uint8_t			ts_byte_t;
typedef char				ts_char_t;

/*! brief packet_format
 *
 * \code
 * // transport packet format - TS
 *  struct transport_packet
	{
		// header
		sync_byte 						: 8		;
		transport_error_indicator 		: 1		;
		payload_unit_start_indicator 	: 1		;
		transport_priority 				: 1		;
		pid 							: 13	;
		transport_scrambling_control 	: 2		;
		adaptation_field_control 		: 2		;
		continuity_counter 				: 4		;

		// adaptation field
		if (adaptation_field_control == 0x2
		||	adaptation_field_control == 0x3 )
		{
			struct adaptation_field
			{
				// adaptation field header
				adaptation_field_length					: 8		;

				if (adaptation_field_length > 0)
				{
					discontinuity_indicator					: 1		;
					random_access_indicator					: 1		;
					elementary_stream_priority_indicator	: 1		;
					pcr_flag								: 1		;
					opcr_flag								: 1		;
					splicing_point_flag						: 1		;
					transport_private_data_flag				: 1		;
					adaptation_field_extension_flag			: 1		;

					// program clock reference
					if (pcr_flag == 1)
					{
						pcr_base							: 33	;
						reseved								: 6		;
						pcr_extension						: 9		;

					}

					// original program clock reference
					if (opcr_flag == 1)
					{
						opcr_base							: 33	;
						reseved								: 6		;
						opcr_extension						: 9		;
					}

					// splicing point
					if (splicing_point_flag == 1)
					{
						splice_countdown					: 8		;
					}

					// transport private data
					if (transport_private_data_flag == 1)
					{
						for (i = 0; i < transport_private_data_length; ++i)
							private_data_byte				: 8		;
					}

					// adaptation field extension
					if (adaptation_field_extension_flag == 1)
					{
						adaptation_field_extension_length	: 8		;
						ltw_flag							: 1		;
						piecewise_rate_flag					: 1		;
						seamless_splice_flag				: 1		;
						reserved							: 5		;

						if (ltw_flag == 1)
						{
							ltw_valid_flag					: 1		;
							ltw_offset						: 15	;
						}

						if (piecewise_rate_flag == 1)
						{
							reserved						: 2		;
							piecewise_rate					: 22	;
						}

						if (seamless_splice_flag == 1)
						{
							splice_type						: 4		;
							dts_next_au[32..30]				: 3		;
							marker_bit						: 1		;
							dts_next_au[29..15]				: 15	;
							marker_bit						: 1		;
							dts_next_au[14..0]				: 15	;
							marker_bit						: 1		;
						}

						for (i = 0; i < N; ++i)
							reserved						: 8		;
						}
					}

					for (i = 0; i < N; ++i)
						stuffing_byte						: 8		;
				}
			};
		}

		// payload data
		if (adaptation_field_control == 0x1
		||	adaptation_field_control == 0x3 )
		{
			for (i = 0; i < N; ++i)
				data_byte				: 8		;
		}
	};

	// payload data
	struct payload
	{
		if (pid == 0x0000)	// pat
		{
			struct program_association_section
			{
				table_id						:8		;
				section_syntax_indicator		:1		;
				zero_bit						:1		;
				reserved_2						:2		;
				section_length					:12		;	//< data length after this
				transport_stream_id				:16		;
				reserved						:2		;
				version_number					:5		;
				current_next_indicator			:1		;
				section_number					:8		;
				last_section_number				:8		;

				for (i = 0; i < N; i++)
				{
					program_number				:16		;
					reversed					:3		;
					if (program_number == '0')
						network_pid				:13		;
					else program_map_pid		:13		;
				}
			};
		}
	};
 * \endcode
 *
 */
typedef struct __ts_data_t
{
	// data info
	ts_byte_t		data[TS_PACKET_SIZE];		// the ts packet data
	ts_size_t		size;							// the ts packet size

}ts_data_t;

typedef struct __ts_header_t
{
	ts_uint_t		sync_byte;
	ts_uint_t		transport_error_indicator;
	ts_uint_t		payload_unit_start_indicator;
	ts_uint_t		transport_priority;
	ts_uint_t		pid;
	ts_uint_t		transport_scrambling_control;
	ts_uint_t		adaptation_field_control;
	ts_uint_t		continuity_counter;

}ts_header_t;

typedef struct __ts_adaptation_t
{
	// adaptation field length
	ts_uint_t 		adaptation_field_length;

	// indicators
	ts_uint_t 		discontinuity_indicator;
	ts_uint_t 		random_access_indicator;
	ts_uint_t 		elementary_stream_priority_indicator;

	// flags
	ts_uint_t 		pcr_flag;
	ts_uint_t 		opcr_flag;
	ts_uint_t 		splicing_point_flag;
	ts_uint_t 		transport_private_data_flag;
	ts_uint_t 		adaptation_field_extension_flag;

	// program clock reference
	ts_uint64_t 	pcr_base;
	ts_uint_t 		pcr_ext;

	// original program clock reference
	ts_uint64_t 	opcr_base;
	ts_uint_t 		opcr_ext;

	// splicing point
	ts_uint_t 		splice_countdown;

	// transport private data
	ts_uint_t 		transport_private_data_length;
	ts_byte_t* 		private_data_byte;

	// adaptation field extension
	ts_uint_t 		adaptation_field_extension_length;
	ts_uint_t 		ltw_flag;
	ts_uint_t 		piecewise_rate_flag;
	ts_uint_t 		seamless_splice_flag;

	// ltw
	ts_uint_t 		ltw_valid_flag;
	ts_uint_t 		ltw_offset;

	// piecewise_rate
	ts_uint_t 		piecewise_rate;

	// seamless_splice
	ts_uint_t 		splice_type;
	ts_uint64_t 	dts_next_au;

}ts_adaptation_t;

typedef struct __ts_payload_t
{
	ts_byte_t*		data;
	ts_size_t		size;

}ts_payload_t;

typedef struct __ts_packet_t
{
	// structs
	ts_data_t		data;
	ts_header_t		header;
	ts_adaptation_t	adaptation;
	ts_payload_t	payload;

	// others
	ts_uint_t 		pointer_field;
	ts_bool_t		skip_adaptation;

}ts_packet_t;

typedef struct __ts_decoder_t
{
	// skip adaptation filed
	ts_bool_t		skip_adaptation;

}ts_decoder_t;

/* ////////////////////////////////////////////////////////////////////////
 * bits
 */

/*
  -- get bits out of buffer  (max 48 bit)
  -- extended bitrange, so it's slower
  -- return: value
 */
static ts_uint64_t ts_get_bits48(ts_byte_t *data, ts_int_t byte_offset, ts_int_t bit_pos, ts_size_t bit_n)
{
	ts_byte_t *b;
	ts_uint64_t v;
	ts_uint64_t mask;
	ts_uint64_t tmp;

	if (bit_n > 48)
	{
		TS_DBG(" Error: ts_get_bits48() request out of bound!!!! (report!!)");
		return 0xFEFEFEFEFEFEFEFELL;
	}


	b = &data[byte_offset + (bit_pos / 8)];
	bit_pos %= 8;


	// -- safe is 48 bitlen
	tmp = (ts_uint64_t)(
	 ((ts_uint64_t)*(b) << 48) + ((ts_uint64_t)*(b + 1) << 40) +
	 ((ts_uint64_t)*(b + 2) << 32) + ((ts_uint64_t)*(b + 3) << 24) +
	 (*(b + 4) << 16) + (*(b + 5) << 8) + *(b + 6) );

	bit_pos		= 56 - bit_pos - bit_n;
	tmp			= tmp >> bit_pos;
	mask		= (1ULL << bit_n) - 1;	// 1ULL !!!
	v			= tmp & mask;

	return v;
}

/*
  -- get bits out of buffer (max 32 bit!!!)
  -- return: value
*/
static ts_ulong_t ts_get_bits(ts_byte_t *data, ts_int_t byte_offset, ts_int_t bit_pos, ts_size_t bit_n)
{
	ts_byte_t*		b;
	ts_ulong_t		v;
	ts_ulong_t		mask;
	ts_ulong_t		tmp_long;
	ts_int_t		bit_high;

	b = &data[byte_offset + (bit_pos >> 3)];
	bit_pos %= 8;

	switch ((bit_n - 1) >> 3)
	{
		case -1:	// -- <=0 bits: always 0
			return 0L;
			break;

		case 0:		// -- 1..8 bit
			tmp_long = (ts_ulong_t)((*(b) << 8) + *(b + 1));
			bit_high = 16;
			break;

		case 1:		// -- 9..16 bit
			tmp_long = (ts_ulong_t)((*(b) <<16) + (*(b + 1) << 8) + *(b + 2));
			bit_high = 24;
			break;

		case 2:		// -- 17..24 bit
			tmp_long = (ts_ulong_t)((*(b) << 24) + (*(b + 1) << 16) + (*(b + 2) << 8) + *(b + 3));
			bit_high = 32;
			break;

		case 3:		// -- 25..32 bit
			// -- to be safe, we need 32+8 bit as shift range
			return (ts_ulong_t)ts_get_bits48(b, 0, bit_pos, bit_n);
			break;

		default:	// -- 33.. bits: fail, deliver constant fail value
			TS_DBG(" Error: ts_get_bits() request out of bound!!!! (report!!)");
			return (ts_ulong_t)0xFEFEFEFE;
			break;
	}

	bit_pos		= bit_high - bit_pos - bit_n;
	tmp_long	= tmp_long >> bit_pos;
	mask		= (1ULL << bit_n) - 1;  // 1ULL !!!
	v			= tmp_long & mask;

	return v;
}

// "C" {
#	ifdef __cplusplus
}
#	endif
#endif

