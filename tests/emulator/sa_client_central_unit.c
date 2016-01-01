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
#include "cu_persistent_storage.h"

#include <stdio.h>


#define MAX_INSTANCES_SUPPORTED 3

void test_mode_load_default_test_project()
{
	// we assume that the first field represents basic information and is structured as follows:
	// | device_id (2 bytes, low, high) | key (16 bytes) | is_retransmitter (1 byte) | bus_type_count (1 byte) | bus_type_list (variable size) |
	uint8_t base_key[16] = { 	0x00, 0x01,	0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, };
	uint8_t bus_types[2];
	bus_types[0] = 0;
	bus_types[1] = 1;
	uint8_t base_record[MAX_FIELD_SIZE_AVAILABLE];
	unsigned int i, j;
	for ( i=0; i<MAX_INSTANCES_SUPPORTED; i++ )
	{
		uint16_t device_id = (uint16_t)(i + 1);
		base_record[0] = (uint8_t)device_id;
		base_record[1] = (uint8_t)(device_id>>8);
		for ( j=0; j<16; j++ )
			base_record[2+j] = base_key[j] + ( device_id << 4 );
		if ( i == 0 )
		{
			base_record[18] = 1;
			base_record[19] = 2; // bus type count
			base_record[20] = 0;
			base_record[21] = 1;
			write_field( device_id, 0, 22, base_record );
		}
		else
		{
			base_record[18] = 0;
			base_record[19] = 1; // bus type count
			base_record[20] = 1;
			write_field( device_id, 0, 21, base_record );
		}
	}
}

void test_mode_reload_unconditionally()
{
	return;
	init_default_storage();
	test_mode_load_default_test_project();
}


int run_init_loop()
{
	unsigned int i;
	uint8_t base_record[MAX_FIELD_SIZE_AVAILABLE];
	uint16_t data_sz;
	MEMORY_HANDLE mem_h = acquire_memory_handle();
	ZEPTO_DEBUG_ASSERT( mem_h != MEMORY_HANDLE_INVALID );
	for ( i=0; i<MAX_DEVICE_COUNT; i++ )
	{
		read_field( i, 0, &data_sz, base_record );
		if ( data_sz )
		{
			zepto_write_block( mem_h, base_record, data_sz );
			zepto_response_to_request( mem_h );
			send_to_commm_stack_initializing_packet( mem_h, i );
			zepto_parser_free_memory( mem_h );
		}
	}
	send_to_commm_stack_end_of_initialization_packet( i );
	release_memory_handle( mem_h );
	return 1;
}


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



	DefaultTestingControlProgramState DefaultTestingControlProgramState_struct[ MAX_INSTANCES_SUPPORTED ];
	for ( dev_in_use=0; dev_in_use<MAX_INSTANCES_SUPPORTED; dev_in_use++ )
		default_test_control_program_init( DefaultTestingControlProgramState_struct + dev_in_use, dev_in_use + 1 );

	for ( dev_in_use=1; dev_in_use<MAX_INSTANCES_SUPPORTED; dev_in_use++ )
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

	uint16_t bus_or_device_id;

	// MAIN LOOP
	for (;;)
	{
wait_for_comm_event:
		ret_code = wait_for_communication_event( 200, &bus_or_device_id ); // TODO: recalculation
//		zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );

		switch ( ret_code )
		{
			case COMMLAYER_RET_TIMEOUT:
			{
				// regular processing will be done below in the next block
//				ZEPTO_DEBUG_PRINTF_1( "just waiting...\n" );
				ZEPTO_DEBUG_ASSERT( bus_or_device_id == 0xFFFF );
				zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
				goto wait_for_comm_event;
				break;
			}
			case COMMLAYER_RET_FROM_COMMM_STACK:
			{
				// regular processing will be done below in the next block
				ZEPTO_DEBUG_ASSERT( bus_or_device_id == 0xFFFF );
				ret_code = try_get_message_within_master( MEMORY_HANDLE_MAIN_LOOP_1, &bus_or_device_id );
				if ( ret_code == COMMLAYER_STATUS_FAILED )
					return 0;
				if ( ret_code == COMMLAYER_STATUS_FOR_CU_FROM_SLAVE )
				{
					ZEPTO_DEBUG_ASSERT( bus_or_device_id != 0xFFFF );
					zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
					dev_in_use = bus_or_device_id;
					dev_in_use --;
					ZEPTO_DEBUG_PRINTF_4( "msg received; rq_size: %d, rsp_size: %d, src: %d\n", ugly_hook_get_request_size( MEMORY_HANDLE_MAIN_LOOP_1 ), ugly_hook_get_response_size( MEMORY_HANDLE_MAIN_LOOP_1 ), dev_in_use );
					goto process_reply;
					break;
				}
				else if ( ret_code == COMMLAYER_STATUS_FOR_SLAVE )
				{
					ZEPTO_DEBUG_ASSERT( bus_or_device_id != 0xFFFF );
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
					send_message( MEMORY_HANDLE_MAIN_LOOP_1, bus_or_device_id );
#endif // SA_ACTIVE_AIR_DEBUG
					// [[AIR test block END]]
					goto wait_for_comm_event;
					break;
				}
				else if ( ret_code == COMMLAYER_STATUS_FOR_CU_SLAVE_ERROR )
				{
					// since now we implement just a testing helper, we simply show the content of the error
					ZEPTO_DEBUG_ASSERT( bus_or_device_id != 0xFFFF );
					parser_obj po;
					zepto_parser_init( &po, MEMORY_HANDLE_MAIN_LOOP_1 );
					uint16_t sz = zepto_parsing_remaining_bytes( &po );
					if ( sz >= 2 )
					{
						uint8_t bt1 = zepto_parse_uint8( &po );
						uint8_t bt2 = zepto_parse_uint8( &po );
						ZEPTO_DEBUG_PRINTF_4("\n>>>>>>>>>>>>>>>>>>>>>>> Comm.Stack has reported an error: [%d, %d] for device %d\n", bt1, bt2, bus_or_device_id );
					}
					else
					{
						ZEPTO_DEBUG_PRINTF_2("\n>>>>>>>>>>>>>>>>>>>>>>> Comm.Stack has reported an error for device %d\n", bus_or_device_id );
					}
					ZEPTO_DEBUG_ASSERT( 0 );
				}
				else if ( ret_code == COMMLAYER_STATUS_FOR_CU_SYNC_REQUEST )
				{
					parser_obj po;
					zepto_parser_init( &po, MEMORY_HANDLE_MAIN_LOOP_1 );
					uint8_t rq_type = zepto_parse_uint8( &po );
					switch ( rq_type )
					{
						case REQUEST_WRITE_DATA:
						{
							uint8_t tmp = zepto_parse_uint8( &po );
							uint16_t dev_id = zepto_parse_uint8( &po );
							dev_id <<= 8;
							dev_id += tmp;
							uint8_t field_id = zepto_parse_uint8( &po );
							tmp = zepto_parse_uint8( &po );
							uint16_t sz = zepto_parse_uint8( &po );
							sz <<= 8;
							sz += tmp;
							uint8_t base_record[MAX_FIELD_SIZE_AVAILABLE];
							ZEPTO_DEBUG_ASSERT( zepto_parsing_remaining_bytes( &po ) == sz );
							zepto_parse_read_block( &po, base_record, sz );
							write_field( dev_id, field_id, sz, base_record );
							// TODO: prepare send reply (confirmation)
							break;
						}
						case REQUEST_READ_DATA:
						{
							uint8_t tmp = zepto_parse_uint8( &po );
							uint16_t dev_id = zepto_parse_uint8( &po );
							dev_id <<= 8;
							dev_id += tmp;
							uint8_t field_id = zepto_parse_uint8( &po );

							uint16_t sz;
							uint8_t base_record[MAX_FIELD_SIZE_AVAILABLE];
							read_field( dev_id, field_id, &sz, base_record );

							// TODO: prepare and send reply (requested data)
							break;
						}
						default:
						{
							ZEPTO_DEBUG_ASSERT( 0 == "Unexpected request type" );
							break;
						}
					}
				}
				else
				{
					ZEPTO_DEBUG_ASSERT( 0 );
				}
			}
			case COMMLAYER_RET_FROM_DEV:
			{
				// regular processing will be done below in the next block
				ZEPTO_DEBUG_ASSERT( bus_or_device_id != 0xFFFF );
				ret_code = hal_get_packet_bytes( MEMORY_HANDLE_MAIN_LOOP_1 );
				if ( ret_code == HAL_GET_PACKET_BYTES_FAILED )
					return 0;
				ZEPTO_DEBUG_ASSERT( ret_code == HAL_GET_PACKET_BYTES_DONE );
				zepto_response_to_request( MEMORY_HANDLE_MAIN_LOOP_1 );
//				ZEPTO_DEBUG_PRINTF_3( "msg received from slave; rq_size: %d, rsp_size: %d\n", ugly_hook_get_request_size( MEMORY_HANDLE_MAIN_LOOP_1 ), ugly_hook_get_response_size( MEMORY_HANDLE_MAIN_LOOP_1 ) );
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
				send_to_commm_stack_as_from_slave( MEMORY_HANDLE_MAIN_LOOP_1, bus_or_device_id );
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

char* get_persistent_storage_path_from_command_line(int argc, char *argv[])
{
	uint8_t i;
	for ( i = 0; i<argc; i++ )
		if ( ZEPTO_MEMCMP( argv[i], "--psp=", 6 ) == 0 )
		{
			ZEPTO_DEBUG_PRINTF_2( "persistent storage is at: \"%s\"\n", argv[i]+6 );
			return argv[i]+6;
		}
	ZEPTO_DEBUG_PRINTF_1( "default persistent storage location will be used\n" );
	return NULL;
}

int init_eeprom(int argc, char *argv[])
{
	char* persistent_storage_path = get_persistent_storage_path_from_command_line( argc, argv );
	uint8_t ret_code = hal_init_eeprom_access( persistent_storage_path );
	switch ( ret_code )
	{
		case HAL_PS_INIT_FAILED:
		{
			ZEPTO_DEBUG_PRINTF_1( "init_eeprom_access() failed\n" );
			return 0;
		}
		case HAL_PS_INIT_OK:
		{
			ZEPTO_DEBUG_PRINTF_1( "hal_init_eeprom_access() passed\n" );
			break;
		}
		case HAL_PS_INIT_OK_NEEDS_INITIALIZATION:
		{
			init_default_storage();
			test_mode_load_default_test_project();
			ZEPTO_DEBUG_PRINTF_1( "initializing eeprom done\n" );
			break;
		}
	}

	return 1;
}

int main(int argc, char *argv[])
{
	set_port_from_command_line( argc, argv );
	zepto_mem_man_init_memory_management();
	if ( !init_eeprom( argc, argv ) )
		return 0;

	test_mode_reload_unconditionally();

	run_init_loop();
	return main_loop();
}
