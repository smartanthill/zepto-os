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


/*******************************************************************************
  CONTENT OF THIS FILE IS INTENDED FOR DEBUG PURPOSES ONLY
  and implements a highly simplified main loop.
  
  In short, packet processing is reduced to sipmly echoing an incoming packet
  back (using the same bus_id).
  
  Main purpose is to make sure that packets have a chance to successfully pass
  communication layer and to be delivered to higher level protocols
*******************************************************************************/

#if !defined VERY_DEBUG
#error This file is intended for DEBUG PURPOSES ONLY and cannot be used when VERY_DEBUG is not defined
#else // VERY_DEBUG
#if !defined VERY_DEBUG_SIMPLE_MAIN_LOOP
#error This file is intended for DEBUG PURPOSES ONLY and cannot be used when VERY_DEBUG_SIMPLE_MAIN_LOOP is not defined
#else // VERY_DEBUG_SIMPLE_MAIN_LOOP


#include "sa_main.h"

DECLARE_DEVICE_ID

waiting_for wait_for;

bool sa_main_init()
{
	zepto_mem_man_init_memory_management();
	if (!init_eeprom_access())
		return false;
//	format_eeprom_at_lifestart();

	ZEPTO_DEBUG_PRINTF_1("STARTING SERVER...\n");
	ZEPTO_DEBUG_PRINTF_1("==================\n\n");

	ZEPTO_MEMSET( &wait_for, 0, sizeof( waiting_for ) );
	wait_for.wait_packet = 1;
	TIME_MILLISECONDS16_TO_TIMEVAL( 1000, wait_for.wait_time ); //+++TODO: actual processing throughout the code

	ZEPTO_DEBUG_PRINTF_1("\nAwaiting client connection... \n" );
	if (!communication_initialize())
		return false;

	ZEPTO_DEBUG_PRINTF_1("Client connected.\n");

    return true;
}

int sa_main_loop()
{

//	waiting_for ret_wf;
	uint8_t ret_code;

	wait_for.wait_packet = 1;

	for (;;)
	{
		// 1. Wait for an event
wait_for_comm_event:
			ret_code = hal_wait_for( &wait_for );

			switch ( ret_code )
			{
				case WAIT_RESULTED_IN_FAILURE:
				{
					// TODO: consider potential failures and think about proper processing
					return 0;
					break;
				}
				case WAIT_RESULTED_IN_PACKET:
				{
//					uint8_t bus_id = hal_get_busid_of_last_packet();
					ret_code = hal_get_packet_bytes( MEMORY_HANDLE_MAIN_LOOP_1 );
					switch ( ret_code )
					{
						case HAL_GET_PACKET_BYTES_FAILED:
						{
							zepto_parser_free_memory( MEMORY_HANDLE_MAIN_LOOP_1 );
							return 0; // TODO: think about recovery (later attempts, etc)
							break;
						}
						case HAL_GET_PACKET_BYTES_DONE:
						{
							zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
//							hal_send_packet( MEMORY_HANDLE_MAIN_LOOP_1, 1-bus_id, 0 );
							ret_code = send_message( MEMORY_HANDLE_MAIN_LOOP_1 );
							goto wait_for_comm_event;
							break;
						}
						default:
						{
							ZEPTO_DEBUG_PRINTF_2( "Unexpected ret_code %d\n", ret_code );
							ZEPTO_DEBUG_ASSERT( 0 );
							return 0;
							break;
						}
					}
					goto wait_for_comm_event;
				}
				case WAIT_RESULTED_IN_TIMEOUT:
				{
					goto wait_for_comm_event;
					break;
				}
				default:
				{
					// unexpected ret_code
					ZEPTO_DEBUG_PRINTF_2( "Unexpected ret_code %d\n", ret_code );
					ZEPTO_DEBUG_ASSERT( 0 );
					break;
				}
			}


	}

	return 0;
}

#endif // VERY_DEBUG_SIMPLE_MAIN_LOOP
#endif // VERY_DEBUG