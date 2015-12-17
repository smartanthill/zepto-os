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


#include <simpleiot/siot_common.h>
#include "cu_commlayer.h"
#include "sa_test_control_prog.h"
#include <simpleiot_hal/siot_mem_mngmt.h>
#include "saccp_protocol_client_side_cu.h"
#ifdef SA_ACTIVE_AIR_DEBUG
#include "test_generator.h"
#endif // SA_ACTIVE_AIR_DEBUG

#include <stdio.h>


int main_loop()
{
#ifdef ENABLE_COUNTER_SYSTEM
	INIT_COUNTER_SYSTEM
#endif // ENABLE_COUNTER_SYSTEM


	ZEPTO_DEBUG_PRINTF_1("starting CLIENT...\n");
	ZEPTO_DEBUG_PRINTF_1("==================\n\n");

#ifdef SA_ACTIVE_AIR_DEBUG
	tester_initTestSystem();
#endif // SA_ACTIVE_AIR_DEBUG


	uint8_t ret_code;
	uint16_t dev_in_use;

	// test setup values
//	bool wait_for_incoming_chain_with_timer = false;
//	uint16_t wake_time_to_start_new_chain;

	uint8_t wait_to_continue_processing = 0;
//	uint16_t wake_time_continue_processing;


	// Try to initialize connection
	bool comm_init_ok = communication_initialize();
	if ( !comm_init_ok )
	{
		return -1;
	}



#ifdef MASTER_ENABLE_ALT_TEST_MODE

#define MAX_INSTANCES_SUPPORTED 3


	DefaultTestingControlProgramState DefaultTestingControlProgramState_struct[ MAX_INSTANCES_SUPPORTED ];
	for ( dev_in_use=0; dev_in_use<MAX_INSTANCES_SUPPORTED; dev_in_use++ )
		default_test_control_program_init( DefaultTestingControlProgramState_struct + dev_in_use, dev_in_use + 1 );

	for ( dev_in_use=0; dev_in_use<MAX_INSTANCES_SUPPORTED; dev_in_use++ )
//	dev_in_use=1;
	{
		ret_code = default_test_control_program_start_new( DefaultTestingControlProgramState_struct + dev_in_use, MEMORY_HANDLE_MAIN_LOOP_1 );
		zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
		handler_saccp_prepare_to_send( MEMORY_HANDLE_MAIN_LOOP_1 );
		zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
	/*	parser_obj po;
		zepto_parser_init( &po, MEMORY_HANDLE_MAIN_LOOP_1 );
		uint8_t bu[40];
		memset( bu, 0, 40 );
		zepto_parse_read_block( &po, bu, zepto_parsing_remaining_bytes( &po ) );
		for ( int k=0;k<30; k++)
			ZEPTO_DEBUG_PRINTF_3( "%c [0x%x]\n", bu[k], bu[k] );
		return 0;*/
	//	goto send_command;
		send_to_commm_stack_as_from_master( MEMORY_HANDLE_MAIN_LOOP_1, dev_in_use + 1 );
		zepto_parser_free_memory( MEMORY_HANDLE_MAIN_LOOP_1 );
	}
#else
		uint8_t buff_base[] = {0x2, 0x0, 0x8, 0x1, 0x1, 0x2, 0x0, 0x1, '-', '-', '>' };
		uint8_t buff[128];
		buff[0] = 1; // first in the chain
		ZEPTO_MEMCPY( buff+1, buff_base, sizeof(buff_base) );
		zepto_write_block( MEMORY_HANDLE_MAIN_LOOP_1, buff, 1+sizeof(buff_base) );
		zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
		// should appear on the other side: Packet received: [8 bytes]  [1][0x0001][0x0001][0x0002][0x0000][0x0001]-->
		// should come back: 02 01 01 02 01 02 2d 2d 2d 2d 3e
#endif

	// MAIN LOOP
	for (;;)
	{
wait_for_comm_event:
		ret_code = wait_for_communication_event( 200 ); // TODO: recalculation
//		zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );

		switch ( ret_code )
		{
			case COMMLAYER_RET_TIMEOUT:
			{
				// regular processing will be done below in the next block
//				ZEPTO_DEBUG_PRINTF_1( "just waiting...\n" );
				zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
				goto wait_for_comm_event;
				break;
			}
			case COMMLAYER_RET_FROM_COMMM_STACK:
			{
				// regular processing will be done below in the next block
				ret_code = try_get_message_within_master( MEMORY_HANDLE_MAIN_LOOP_1 );
				if ( ret_code == COMMLAYER_RET_FAILED )
					return 0;
				if ( ret_code == COMMLAYER_RET_OK_FOR_CU )
				{
					zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
					parser_obj po, po1;
					zepto_parser_init( &po, MEMORY_HANDLE_MAIN_LOOP_1 );
					dev_in_use = zepto_parse_encoded_uint16( &po );
					ZEPTO_DEBUG_ASSERT( dev_in_use > 0 );
					ZEPTO_DEBUG_PRINTF_2( "Packet received from device %d\n", dev_in_use );
					dev_in_use --;
					zepto_parser_init_by_parser( &po1, &po );
					zepto_parse_skip_block( &po1, zepto_parsing_remaining_bytes( &po ) );
					zepto_convert_part_of_request_to_response( MEMORY_HANDLE_MAIN_LOOP_1, &po, &po1 );
					zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
					ZEPTO_DEBUG_PRINTF_4( "msg received; rq_size: %d, rsp_size: %d, src: %d\n", ugly_hook_get_request_size( MEMORY_HANDLE_MAIN_LOOP_1 ), ugly_hook_get_response_size( MEMORY_HANDLE_MAIN_LOOP_1 ), dev_in_use );
					goto process_reply;
					break;
				}
				else if ( ret_code == COMMLAYER_RET_OK_FOR_SLAVE )
				{
					zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
					ZEPTO_DEBUG_PRINTF_3( "msg is about to be sent to slave; rq_size: %d, rsp_size: %d\n", ugly_hook_get_request_size( MEMORY_HANDLE_MAIN_LOOP_1 ), ugly_hook_get_response_size( MEMORY_HANDLE_MAIN_LOOP_1 ) );
					// [[AIR test block START]]
#ifdef SA_ACTIVE_AIR_DEBUG
					tester_registerOutgoingPacket( MEMORY_HANDLE_MAIN_LOOP_1 );
					if ( tester_shouldInsertOutgoingPacket( MEMORY_HANDLE_TEST_SUPPORT ) )
					{
//						INCREMENT_COUNTER( 80, "MAIN LOOP, packet inserted" );
						zepto_response_to_request( MEMORY_HANDLE_TEST_SUPPORT );
						send_message( MEMORY_HANDLE_TEST_SUPPORT );
						zepto_response_to_request( MEMORY_HANDLE_TEST_SUPPORT );
					}
					bool is_packet_to_send = true;
					if ( !tester_shouldDropOutgoingPacket() )
					{
						if ( is_packet_to_send )
						{
							ret_code = send_message( MEMORY_HANDLE_MAIN_LOOP_1 );
							zepto_parser_free_memory( MEMORY_HANDLE_MAIN_LOOP_1 );
							if (ret_code != COMMLAYER_RET_OK )
							{
								return -1;
							}
//							INCREMENT_COUNTER( 82, "MAIN LOOP, packet sent" );
							ZEPTO_DEBUG_PRINTF_1("\nMessage sent to slave\n");
						}
					}
					else
					{
//						INCREMENT_COUNTER( 81, "MAIN LOOP, packet dropped" );
						ZEPTO_DEBUG_PRINTF_1("\nOutgoing message lost on the way...\n");
					}
#else // SA_ACTIVE_AIR_DEBUG
					send_message( MEMORY_HANDLE_MAIN_LOOP_1 );
#endif // SA_ACTIVE_AIR_DEBUG
					// [[AIR test block END]]
					goto wait_for_comm_event;
					break;
				}
				else if ( ret_code == COMMLAYER_RET_OK_FOR_CU_ERROR )
				{
					// since now we implement just a testing helper, we simply show the content of the error
					parser_obj po;
					zepto_parser_init( &po, MEMORY_HANDLE_MAIN_LOOP_1 );
					uint8_t bt1 = zepto_parse_uint8( &po );
					uint8_t bt2 = zepto_parse_uint8( &po );
					ZEPTO_DEBUG_PRINTF_3("\n>>>>>>>>>>>>>>>>>>>>>>> Comm.Stack has reported an error: %d, %d\n", bt1, bt2 );
					ZEPTO_DEBUG_ASSERT( 0 );
				}
				else
				{
					ZEPTO_DEBUG_ASSERT( 0 );
				}
			}
			case COMMLAYER_RET_FROM_DEV:
			{
				// regular processing will be done below in the next block
				ret_code = hal_get_packet_bytes( MEMORY_HANDLE_MAIN_LOOP_1 );
				if ( ret_code == HAL_GET_PACKET_BYTES_FAILED )
					return 0;
				ZEPTO_DEBUG_ASSERT( ret_code == HAL_GET_PACKET_BYTES_DONE );
				zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
				ZEPTO_DEBUG_PRINTF_3( "msg received from slave; rq_size: %d, rsp_size: %d\n", ugly_hook_get_request_size( MEMORY_HANDLE_MAIN_LOOP_1 ), ugly_hook_get_response_size( MEMORY_HANDLE_MAIN_LOOP_1 ) );
				// [[AIR test block START]]
#ifdef SA_ACTIVE_AIR_DEBUG
				tester_registerIncomingPacket( MEMORY_HANDLE_MAIN_LOOP_1 );
					if ( tester_shouldInsertIncomingPacket( MEMORY_HANDLE_TEST_SUPPORT ) )
					{
//						INCREMENT_COUNTER( 80, "MAIN LOOP, packet inserted" );
						zepto_response_to_request( MEMORY_HANDLE_TEST_SUPPORT );
						send_to_commm_stack_as_from_slave( MEMORY_HANDLE_TEST_SUPPORT );
						zepto_response_to_request( MEMORY_HANDLE_TEST_SUPPORT );
					}
					bool is_packet_to_send = true;
					if ( !tester_shouldDropIncomingPacket() )
					{
						if ( is_packet_to_send )
						{
							ret_code = send_to_commm_stack_as_from_slave( MEMORY_HANDLE_MAIN_LOOP_1 );
							zepto_parser_free_memory( MEMORY_HANDLE_MAIN_LOOP_1 );
							if (ret_code != COMMLAYER_RET_OK )
							{
								return -1;
							}
//							INCREMENT_COUNTER( 82, "MAIN LOOP, packet sent" );
							ZEPTO_DEBUG_PRINTF_1("\nMessage received from slave\n");
						}
					}
					else
					{
						INCREMENT_COUNTER( 81, "MAIN LOOP, packet dropped" );
						ZEPTO_DEBUG_PRINTF_1("\nIncoming message lost on the way...\n");
					}
#else // SA_ACTIVE_AIR_DEBUG
				send_to_commm_stack_as_from_slave( MEMORY_HANDLE_MAIN_LOOP_1 );
#endif // SA_ACTIVE_AIR_DEBUG
				// [[AIR test block END]]
				goto wait_for_comm_event;
				break;
			}
			default:
			{
				// unexpected ret_code
				ZEPTO_DEBUG_PRINTF_2( "Unexpected ret_code %d received from wait_for_communication_event()\n", ret_code );
				ZEPTO_DEBUG_ASSERT( 0 );
				return 0;
				break;
			}
		}

/*start_new_chain:
		ret_code = default_test_control_program_start_new( &DefaultTestingControlProgramState_struct, MEMORY_HANDLE_MAIN_LOOP_1 );
		zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
		goto send_command;*/

		// 4. Process received command (yoctovm)
#ifdef MASTER_ENABLE_ALT_TEST_MODE
	process_reply:
#if 1
		// do reply preprocessing
		ret_code = handler_saccp_receive( MEMORY_HANDLE_MAIN_LOOP_1, /*sasp_nonce_type chain_id*/NULL, DefaultTestingControlProgramState_struct + dev_in_use );
		zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
		switch ( ret_code )
		{
			case CONTROL_PROG_CHAIN_DONE:
			{
				ret_code = default_test_control_program_start_new( DefaultTestingControlProgramState_struct + dev_in_use, MEMORY_HANDLE_MAIN_LOOP_1 );
				zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
				handler_saccp_prepare_to_send( MEMORY_HANDLE_MAIN_LOOP_1 );
				zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
				break;
			}
			case CONTROL_PROG_CHAIN_CONTINUE:
			{
				break;
			}
			case CONTROL_PROG_CHAIN_CONTINUE_LAST:
			{
				send_to_commm_stack_as_from_master( MEMORY_HANDLE_MAIN_LOOP_1, dev_in_use + 1 );
				ret_code = default_test_control_program_start_new( DefaultTestingControlProgramState_struct + dev_in_use, MEMORY_HANDLE_MAIN_LOOP_1 );
				zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
				handler_saccp_prepare_to_send( MEMORY_HANDLE_MAIN_LOOP_1 );
				zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
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
#else
		ret_code = default_test_control_program_accept_reply( MEMORY_HANDLE_MAIN_LOOP_1, /*sasp_nonce_type chain_id*/NULL, DefaultTestingControlProgramState_struct + dev_in_use );
		zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
		switch ( ret_code )
		{
			case CONTROL_PROG_OK:
			{
//				ret_code = handler_sacpp_start_new_chain( MEMORY_HANDLE_MAIN_LOOP_1, DefaultTestingControlProgramState_struct + dev_in_use );
				ret_code = default_test_control_program_start_new( DefaultTestingControlProgramState_struct + dev_in_use, MEMORY_HANDLE_MAIN_LOOP_1 );
				zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
				break;
			}
			case CONTROL_PROG_CONTINUE:
			{
//				ret_code = handler_sacpp_continue_chain( MEMORY_HANDLE_MAIN_LOOP_1, DefaultTestingControlProgramState_struct + dev_in_use );
				ret_code = default_test_control_program_accept_reply_continue( DefaultTestingControlProgramState_struct + dev_in_use, MEMORY_HANDLE_MAIN_LOOP_1 );
				zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
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
#endif
#else // MASTER_ENABLE_ALT_TEST_MODE
		// so far just print received packet and exit
		uint8_t buff[128];
		uint16_t i;
		parser_obj po;
process_reply:
		zepto_parser_init( &po, MEMORY_HANDLE_MAIN_LOOP_1 );
		uint16_t sz = zepto_parsing_remaining_bytes( &po );
		zepto_parse_read_block( &po, buff, sz );
		ZEPTO_DEBUG_PRINTF_1( "block received:\n" );
		for ( i=0; i<sz; i++ )
			ZEPTO_DEBUG_PRINTF_2( "%02x ", buff[i] );
		ZEPTO_DEBUG_PRINTF_1( "\n\n" );

#endif // MASTER_ENABLE_ALT_TEST_MODE

send_command:
		ZEPTO_DEBUG_PRINTF_4( "=============================================Msg is about to be sent; rq_size: %d, rsp_size: %d; for device %d\n", ugly_hook_get_request_size( MEMORY_HANDLE_MAIN_LOOP_1 ), ugly_hook_get_response_size( MEMORY_HANDLE_MAIN_LOOP_1 ), dev_in_use + 1 );
		send_to_commm_stack_as_from_master( MEMORY_HANDLE_MAIN_LOOP_1, dev_in_use + 1 );
	}

	communication_terminate();

	return 0;
}

void set_port_from_command_line(int argc, char *argv[])
{
	uint8_t i;
	for ( i = 1; i<argc; i++ )
	{
		if ( ZEPTO_MEMCMP( argv[i], "--port=", 7 ) == 0 )
		{
			int port = atoi( argv[i]+7);
			ZEPTO_DEBUG_ASSERT( port >= 0 && port < 0x10000 );
			ZEPTO_DEBUG_PRINTF_2( "port to be actually used: %d\n", port );
			other_port_num_with_cl = port;
			return;
		}
	}
}

int main(int argc, char *argv[])
{
	set_port_from_command_line( argc, argv );
	zepto_mem_man_init_memory_management();

	return main_loop();
}
