/*******************************************************************************
Copyright (C) 2015 OLogN Technologies AG

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*******************************************************************************/

#include "time_master_common.h"
#include "time_master_commlayer.h"
#include "time_master_log_file.h"

// items below were defined for various reasons in this projec;
// STL does not like such redefinitions (see <xkeycheck.h> for details); we do favor for STL
// If one knows how this could be addressed properly, just do it!
#ifdef printf
#undef printf
#endif

#include <chrono>
using namespace std;


bool time_main_init()
{
	if ( ! communication_initialize() )
		return false;
	if ( !init_access_for_logging() )
		return false;
	return true;
}

bool process_packet( uint8_t* packet_in, uint16_t packet_sz, uint8_t* packet_out, int* packet_out_sz )
{
	if ( packet_sz < 5 )
	{
		ZEPTO_DEBUG_PRINTF_2( "potentialy incomplete request received (size = %d); dropped\n", packet_sz );
		return false;
	}

	time_id_type time_id;
	get_time_id( &time_id );

	int dev_id = packet_in[1];
	dev_id = (dev_id << 8 ) | packet_in[0];
	int type = packet_in[2];
	int data_sz = packet_in[4];
	data_sz = (data_sz << 8 ) | packet_in[3];
	if ( data_sz < packet_sz - 5 )
		ZEPTO_DEBUG_PRINTF_4( "potentialy inconsistent request received: packet size = %d, data size declared = %d (expected: %d)\n", packet_sz, data_sz, packet_sz - 5 );
	else if ( data_sz > packet_sz - 5 )
		ZEPTO_DEBUG_PRINTF_4( "potentialy incomplete request received: packet size = %d, data size declared = %d (expected: %d)\n", packet_sz, data_sz, packet_sz - 5 );

	uint8_t* data_buff = packet_in + 5;

	switch ( type )
	{
		case TIME_RECORD_REGISTER_INCOMING_PACKET:
		case TIME_RECORD_REGISTER_OUTGOING_PACKET:
		{
			ZEPTO_DEBUG_PRINTF_1( "Adding packet record\n" );
			add_in_out_packet_record( time_id, dev_id, type, data_buff, data_sz );
			packet_out[0] = (uint8_t)dev_id;
			packet_out[1] = (uint8_t)(dev_id>>8);
			packet_out[2] = type;
			packet_out[3] = 0;
			packet_out[4] = 0;
			*packet_out_sz = 5;
			return true;
			break;
		}
		case TIME_RECORD_REGISTER_TIME_VALUE:
		{
			ZEPTO_DEBUG_PRINTF_1( "Adding time record\n" );
			uint8_t point_id = data_buff[0];
			uint32_t time_ret;
			if ( 1 ) // TODO: add an option when we return "our own" time
			{
				time_ret = data_buff[4];
				time_ret = (time_ret<<8)+data_buff[3];
				time_ret = (time_ret<<8)+data_buff[2];
				time_ret = (time_ret<<8)+data_buff[1];
			}
			else
			{
			}
			add_time_record( time_id, dev_id, point_id, time_ret );
			packet_out[0] = (uint8_t)dev_id;
			packet_out[1] = (uint8_t)(dev_id>>8);
			packet_out[2] = type;
			packet_out[3] = 5;
			packet_out[4] = 0;
			*packet_out_sz = 10;
			packet_out[5] = point_id;
			packet_out[6] = (uint8_t)time_ret;
			packet_out[7] = (uint8_t)(time_ret>>8);
			packet_out[8] = (uint8_t)(time_ret>>16);
			packet_out[9] = (uint8_t)(time_ret>>24);
			return true;
			break;
		}
		case TIME_RECORD_REGISTER_WAIT_RET_VALUE:
		{
			ZEPTO_DEBUG_PRINTF_1( "Adding wait_ret record\n" );;
			uint8_t ret_val = data_buff[0];
			ZEPTO_DEBUG_ASSERT( data_sz == 1 );
			add_waitingfor_ret_record( time_id, dev_id, ret_val );
			packet_out[0] = (uint8_t)dev_id;
			packet_out[1] = (uint8_t)(dev_id>>8);
			packet_out[2] = type;
			packet_out[3] = 1;
			packet_out[4] = 0;
			*packet_out_sz = 6;
			packet_out[5] = ret_val;
			return true;
			break;
		}
		case TIME_RECORD_REGISTER_RAND_VAL_REQUEST_32:
		default:
		{
			ZEPTO_DEBUG_PRINTF_2( "potentialy incomplete request received (size = %d); dropped\n", packet_sz );
			return false;
			break;
		}
	}
}

int time_main_loop()
{
	uint8_t packet_buff[1024];
	uint8_t packet_out_buff[1024];
	uint16_t items[64];
	uint8_t item_cnt;
	int packet_sz;
	uint8_t ret_code;
	uint8_t j;
	for (;;)
	{
		ret_code = wait_for_packet( items, &item_cnt, 64 );
		if ( ret_code == COMMLAYER_RET_TIMEOUT )
		{
			ZEPTO_DEBUG_PRINTF_1( "Waiting for incoming packets...\n" );
			continue;
		}
		ZEPTO_DEBUG_ASSERT( ret_code == COMMLAYER_RET_OK );
		for ( j=0; j<item_cnt; j++ )
		{
			get_packet( packet_buff, 1024, &packet_sz, items[j] );
			if ( process_packet( packet_buff, packet_sz, packet_out_buff, &packet_sz ) )
				send_packet( packet_out_buff, packet_sz, items[j] );
		}
	}
	return 0;
}

int main( int argc, char *argv[] )
{
/*	time_id_type time_id, prev = 0;
	for ( int i=0; ; i++ )
	{
		get_time_id( &time_id );
		uint32_t time32 = time_id_to_uint32_time( time_id );
		ZEPTO_DEBUG_PRINTF_6( "%d: %08x %08x [%08x] -- %d\n", i, (uint32_t)(time_id>> 32), (uint32_t)time_id, (uint32_t)(time_id - prev), time32 );
		prev = time_id;
	}*/
	ZEPTO_DEBUG_PRINTF_1( "TIME MASTER started\n" );
	ZEPTO_DEBUG_PRINTF_1( "===================\n\n" );

    if ( !time_main_init() )
	{
		ZEPTO_DEBUG_PRINTF_1( "Failed to initialize\n" );
		return 0;
	}
    return time_main_loop();
}