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

#include "air_common.h"
#include "air_commlayer.h"

typedef struct _COMM_PARTICIPANT
{
	int dev_id;
	int cnt_to;
	int cnt_from;
} COMM_PARTICIPANT;

#define COMM_PARTICIPANTS_MAX_COUNT 64

COMM_PARTICIPANT participants[COMM_PARTICIPANTS_MAX_COUNT];


bool air_main_init()
{
	return communication_initialize();
	ZEPTO_MEMSET( participants, 0, sizeof(participants) );
	int i;
	for ( i=0; i<COMM_PARTICIPANTS_MAX_COUNT; i++ )
		participants[i].dev_id = i;
}

void do_whatever_with_packet_to_be_sent( uint8_t* packet_buff, int packet_sz, int src, int destination )
{
	ZEPTO_DEBUG_ASSERT( destination < COMM_PARTICIPANTS_MAX_COUNT );
	(participants[src].cnt_from )++;
	(participants[destination].cnt_to )++;
//	if ( participants[destination].cnt_to < 30 || participants[destination].cnt_to > 40 )
		send_packet( packet_buff, packet_sz, destination );
}

int air_main_loop()
{
	uint8_t packet_buff[1024];
	uint16_t items[COMM_PARTICIPANTS_MAX_COUNT];
	uint8_t item_cnt;
	int packet_sz;
	uint8_t ret_code;
	uint8_t i, j;
	for (;;)
	{
		ret_code = wait_for_packet( items, &item_cnt, COMM_PARTICIPANTS_MAX_COUNT );
		if ( ret_code == COMMLAYER_RET_TIMEOUT )
		{
			ZEPTO_DEBUG_PRINTF_1( "Waiting for incoming packets...\n" );
			continue;
		}
		ZEPTO_DEBUG_ASSERT( ret_code == COMMLAYER_RET_OK );
		for ( j=0; j<item_cnt; j++ )
		{
			get_packet( packet_buff, 1024, &packet_sz, items[j] );
			for ( i=0; i<dev_count; i++)
				if ( i != items[j] )
					do_whatever_with_packet_to_be_sent( packet_buff, packet_sz, items[j], i );
		}
	}
	return 0;
}

int main( int argc, char *argv[] )
{
	ZEPTO_DEBUG_PRINTF_1( "AIR started\n" );
	ZEPTO_DEBUG_PRINTF_1( "===========\n\n" );

	if ( !air_main_init() )
	{
		ZEPTO_DEBUG_PRINTF_1( "Failed to initialize\n" );
		return 0;
	}
    return air_main_loop();
}