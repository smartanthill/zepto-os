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
#include "test_generation_support.h"

COMM_PARTICIPANT participants[COMM_PARTICIPANTS_MAX_COUNT];


bool air_main_init()
{
	return communication_initialize();
	ZEPTO_MEMSET( participants, 0, sizeof(participants) );
	int i;
	for ( i=0; i<COMM_PARTICIPANTS_MAX_COUNT; i++ )
		participants[i].dev_id = i;
}

void testing_scenario_at_destination_drop_none( const uint8_t* packet_buff, int packet_sz, int src, int destination )
{
	send_packet( packet_buff, packet_sz, destination );
}

void testing_scenario_at_destination_drop_at_random( const uint8_t* packet_buff, int packet_sz, int src, int destination )
{
	bool allow = (tester_get_rand_val() & 1) != 0;
	if ( allow )
		send_packet( packet_buff, packet_sz, destination );
	else
		ZEPTO_DEBUG_PRINTF_3( "---- Packet has not reached device %d (src: %d) \n", destination, src );
}

void testing_scenario_at_destination_drop_for_random_period( const uint8_t* packet_buff, int packet_sz, int src, int destination )
{
	const int transm_period_max_length = 128;
	const int drop_period_max_length = 12;
	COMM_PARTICIPANT& dev = participants[destination];
	if ( dev.dest_val1 )
	{
		(dev.dest_val1)--;
		if ( dev.dest_val1 == 0 )
		{
			dev.dest_val2 = tester_get_rand_val() % transm_period_max_length + 1;
		}
		ZEPTO_DEBUG_PRINTF_3( "---- Packet has not reached device %d (src: %d) \n", destination, src );
	}
	else
	{
		send_packet( packet_buff, packet_sz, destination );
		if ( dev.dest_val2 )
			(dev.dest_val2) --;
		else
			dev.dest_val1 = tester_get_rand_val() % drop_period_max_length + 1;
	}
}


bool testing_scenario_at_src_drop_none( int src )
{
	return true;
}

bool testing_scenario_at_src_drop_at_random( int src )
{
	bool allow = (tester_get_rand_val() & 1) != 0;
	return allow;
}

bool testing_scenario_at_src_drop_for_random_period( int src )
{
	const int transm_period_max_length = 128;
	const int drop_period_max_length = 12;
	COMM_PARTICIPANT& dev = participants[src];
	if ( dev.src_val1 )
	{
		(dev.src_val1)--;
		if ( dev.src_val1 == 0 )
			dev.src_val2 = tester_get_rand_val() % transm_period_max_length + 1;
		return false;
	}
	else
	{
		if ( dev.src_val2 )
			(dev.src_val2) --;
		else
			dev.src_val1 = tester_get_rand_val() % drop_period_max_length + 1;
		return true;
	}
}



#include <math.h>

float calc_dist( DEVICE_POSITION* pos1, DEVICE_POSITION* pos2 )
{
	return sqrt( ( pos1->x - pos2->x ) * ( pos1->x - pos2->x ) + ( pos1->y - pos2->y ) * ( pos1->y - pos2->y ) + ( pos1->z - pos2->z ) * ( pos1->z - pos2->z ) );
}

void do_whatever_with_packet_to_be_sent( TEST_DATA* test_data, const uint8_t* packet_buff, int packet_sz, int src, int destination )
{
	ZEPTO_DEBUG_ASSERT( destination < COMM_PARTICIPANTS_MAX_COUNT );
	(participants[destination].cnt_to )++;
	DEVICE_POSITION pos;
	get_position( &pos, destination );
	float dist = calc_dist( &pos, &(test_data->pos) );
	ZEPTO_DEBUG_PRINTF_4( "Scheduling packet from %d to %d (distance = %f)... ", src, destination, dist );
	if ( dist > 1.5 )
	{
		ZEPTO_DEBUG_PRINTF_1( "too far, dropped\n" );
		return;
	}
	ZEPTO_DEBUG_PRINTF_1( "will be passed\n" );

	testing_scenario_at_destination_drop_none( packet_buff, packet_sz, src, destination );
//	testing_scenario_at_destination_drop_at_random( packet_buff, packet_sz, src, destination );
//	testing_scenario_at_destination_drop_for_random_period( packet_buff, packet_sz, src, destination );
}
bool allow_to_pass_packet( int src )
{
	(participants[src].cnt_from )++;
	return testing_scenario_at_src_drop_none( src );
//	return testing_scenario_at_src_drop_at_random( src );
//	return testing_scenario_at_src_drop_for_random_period( src );
}

int air_main_loop()
{
	uint8_t packet_buff[1024];
	uint16_t items[COMM_PARTICIPANTS_MAX_COUNT];
	uint8_t item_cnt;
	int packet_sz;
	uint8_t ret_code;
	uint8_t i, j;
	TEST_DATA test_data;
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
			if ( get_packet( &test_data, packet_buff, 1024, &packet_sz, items[j] ) == COMMLAYER_RET_OK )
			{
				add_new_packet( packet_buff, packet_sz );
				if ( !allow_to_pass_packet( items[j] ) )
				{
					ZEPTO_DEBUG_PRINTF_2( "---- Packet originating from device %d lost on the way \n", items[j] );
					continue;
				}
				(participants[items[j]].cnt_from )++;
				for ( i=0; i<dev_count; i++)
					if ( i != items[j] )
						do_whatever_with_packet_to_be_sent( &test_data, packet_buff, packet_sz, items[j], i );
			}
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