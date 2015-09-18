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


bool air_main_init()
{
	return communication_initialize();
}

int air_main_loop()
{
	uint8_t packet_buff[1024];
	uint16_t src;
	int packet_sz;
	uint8_t ret_code;
	for (;;)
	{
		ret_code = wait_for_packet( &src );
		if ( ret_code == COMMLAYER_RET_TIMEOUT )
		{
			ZEPTO_DEBUG_PRINTF_1( "Waiting for incoming packets...\n" );
			continue;
		}
		ZEPTO_DEBUG_ASSERT( ret_code == COMMLAYER_RET_OK );
		get_packet( packet_buff, 1024, &packet_sz, src );
		int i;
		for ( i=0; i<dev_count; i++)
			if ( i != src )
				send_packet( packet_buff, packet_sz, i );
	}
	return 0;
}

int main( int argc, char *argv[] )
{
    if ( !air_main_init() )
	{
		ZEPTO_DEBUG_PRINTF_1( "Failed to initialize\n" );
		return 0;
	}
    return air_main_loop();
}