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

#if (defined VERY_DEBUG) && (defined VERY_DEBUG_SIMPLE_MAIN_LOOP)
#include "sa_main_dbg.inc"
#else // (defined VERY_DEBUG) && (defined VERY_DEBUG_SIMPLE_MAIN_LOOP)

#include "sa_main.h"
#include "zepto_os/debugging.h"

// TODO: actual key loading, etc
//uint8_t AES_ENCRYPTION_KEY[16];
DECLARE_AES_ENCRYPTION_KEY
DECLARE_DEVICE_ID


SAGDP_DATA sagdp_context_app;
SAGDP_DATA sagdp_context_ctr;

waiting_for wait_for;

//#define ALLOW_PRINTING_SASP_INCOMING_MESSAGE
//#define ALLOW_PRINTING_SASP_OUTGOING_MESSAGE
//#define TEST_RAM_CONSUMPTION

#ifdef TEST_RAM_CONSUMPTION

const uint8_t INCOMING_TEST_PACKET[] ZEPTO_PROG_CONSTANT_LOCATION =
{
/*	0x02, 0x89, 0x61, 0xd9, 0x1d, 0x54, 0xb4, 0xac,
	0xbb, 0xcb, 0x6c, 0x74, 0x0f, 0x53, 0xbf, 0x7f,
	0x0a, 0x63, 0xac, 0x7d, 0xf8, 0x31, 0x0c, 0xe2,
	0x2f, 0x25, 0xc8, 0x8c, 0x6f, 0xd0, 0x15, 0x53,
	0x79, 0x15, 0xbd, 0x15, 0xf9, 0x8a, 0x10, 0x3c,
	0x73, 0x9e, 0x93, 0xf4,*/
	0x02, 0x89, 0x61, 0xd9, 0x1d, 0x54, 0xb4, 0xac,
	0xbb, 0xcb, 0x6c, 0x74, 0x0c, 0x53, 0xbf, 0x7f,
	0x0a, 0x63, 0xac, 0x7d, 0xd3, 0x31, 0x0c, 0xe2,
	0x2f, 0x36, 0xfe, 0x84, 0x4a, 0xb5, 0xdb, 0x59,
	0xaf, 0xf1, 0x45, 0x08, 0x3c, 0xfa, 0xab, 0x99,
	0x9c, 0x74, 0x68, 0x3c, 0xe5,
};

const uint8_t OUTGOING_TEST_PACKET[] ZEPTO_PROG_CONSTANT_LOCATION =
{
	0x02, 0x88, 0xe1, 0xd6, 0xe3, 0xaa, 0x4a, 0x53,
	0x25, 0x36, 0x6e, 0x7c, 0x03, 0x50, 0x3a, 0xad,
	0x27, 0x46, 0x81, 0x42, 0x14, 0x86, 0x7a, 0xe1,
	0x2c, 0x10, 0xca, 0x1f, 0x25, 0xcb, 0xed, 0xcc,
	0xc0, 0xe2, 0x29, 0xaf,
};

//uint8_t FAKE_ARRAY_UNDER_STACK[1024-291] __attribute__ ((section (".noinit"))); // TODO: 1024 stands for RAM available, and 291 for reported total size of initial allocations for globals/statics
uint8_t FAKE_ARRAY_UNDER_STACK[1024-291];

void StackPaint(void)
{
	volatile int test_byte = 0;
	test_byte++;
	uint8_t *p = FAKE_ARRAY_UNDER_STACK;
	while( p <= FAKE_ARRAY_UNDER_STACK + sizeof(FAKE_ARRAY_UNDER_STACK) - 64 )
	{
		*p = 0xaa;
		p++;
	}
	test_byte++;
}

#endif // TEST_RAM_CONSUMPTION

bool sa_main_init()
{
	uint8_t ret_code;
	uint8_t rid[DATA_REINCARNATION_ID_SIZE];
	ZEPTO_MEMCPY( rid, AES_ENCRYPTION_KEY, DATA_REINCARNATION_ID_SIZE );
#ifdef TEST_RAM_CONSUMPTION
	StackPaint();
#endif // TEST_RAM_CONSUMPTION
	zepto_mem_man_init_memory_management();

	if (!HAL_COMMUNICATION_INITIALIZE())
		return false;

	if (!init_eeprom_access()) // hardware failure
		return false;

	ret_code = eeprom_check_reincarnation( rid );
	switch ( ret_code )
	{
		case EEPROM_RET_REINCARNATION_ID_OLD:
		{
			sasp_init_eeprom_data_at_lifestart();
			eeprom_update_reincarnation_if_necessary( rid );
			break;
		}
		case EEPROM_RET_REINCARNATION_ID_OK_ONE_OK:
		{
			if ( !eeprom_check_at_start() ) // corrupted data; so far, at least one of slots cannot be recovered
			{
				sasp_init_eeprom_data_at_lifestart();
			}
			eeprom_update_reincarnation_if_necessary( rid );
			break;
		}
		case EEPROM_RET_REINCARNATION_ID_OK_BOTH_OK:
		{
			if ( !eeprom_check_at_start() ) // corrupted data; so far, at least one of slots cannot be recovered
			{
				sasp_init_eeprom_data_at_lifestart();
			}
			break;
		}
		default:
		{
			ZEPTO_DEBUG_ASSERT( 0 == "Unexpected ret code" );
			break;
		}
	}

	DEBUG_ON_EEPROM_INIT();

#ifdef ENABLE_COUNTER_SYSTEM
	INIT_COUNTER_SYSTEM
#endif // ENABLE_COUNTER_SYSTEM

	ZEPTO_DEBUG_PRINTF_1("STARTING SERVER...\n");
	ZEPTO_DEBUG_PRINTF_1("==================\n\n");

	ZEPTO_MEMSET( &wait_for, 0, sizeof( waiting_for ) );
	wait_for.wait_packet = 1;
	TIME_MILLISECONDS16_TO_TIMEVAL( 1000, wait_for.wait_time ); //+++TODO: actual processing throughout the code

//    ZEPTO_MEMSET( AES_ENCRYPTION_KEY, 0xab, 16 );
#if 1 //SIOT_MESH_IMPLEMENTATION_WORKS
	siot_mesh_init_tables(); // TODO: this call reflects current development stage and may or may not survive in the future
#endif // SIOT_MESH_IMPLEMENTATION_WORKS
	sasp_restore_from_backup();
	sagdp_init( &sagdp_context_app );
	sagdp_init( &sagdp_context_ctr );
	zepto_vm_init();

	ZEPTO_DEBUG_PRINTF_1("\nAwaiting client connection... \n" );
	ZEPTO_DEBUG_PRINTF_1("Client connected.\n");

    return true;
}

typedef struct _PACKET_ASSOCIATED_DATA
{
	REQUEST_REPLY_HANDLE packet_h;
	REQUEST_REPLY_HANDLE addr_h;
	uint8_t mesh_val;
	uint8_t resend_cnt;
} PACKET_ASSOCIATED_DATA;

#define SWAP_PACKET_HANDLE_PAIR( p1, p2 ) \
{ \
	REQUEST_REPLY_HANDLE tmp = p1.packet_h; p1.packet_h = p2.packet_h; p2.packet_h = tmp; \
	tmp = p1.addr_h; p1.addr_h = p2.addr_h; p2.addr_h = tmp; \
	uint8_t tmp1 = p1.mesh_val; p1.mesh_val = p2.mesh_val; p2.mesh_val = tmp1; \
	tmp1 = p1.resend_cnt; p1.resend_cnt = p2.resend_cnt; p2.resend_cnt = tmp1; \
}

int sa_main_loop()
{
#ifdef TEST_RAM_CONSUMPTION
	uint8_t j;
	volatile int test_byte = 0;
	test_byte++;
#endif // TEST_RAM_CONSUMPTION

	waiting_for ret_wf;
	uint8_t pid[ SASP_NONCE_SIZE ];
	sa_uint48_t nonce;
	uint8_t ret_code;
	sa_time_val currt;

	// test setup values
	// TODO: all code related to simulation and test generation MUST be moved out here ASAP!
#if 0
	bool wait_for_incoming_chain_with_timer = 0;
#endif
//	uint16_t wake_time_to_start_new_chain = 0;
//	uint8_t wait_to_continue_processing = 0;
//	uint16_t wake_time_continue_processing = 0;
	// END OF test setup values

/*	REQUEST_REPLY_HANDLE interchangable_handles[2];
	interchangable_handles[0] = MEMORY_HANDLE_MAIN_LOOP_1;
	interchangable_handles[1] = MEMORY_HANDLE_MAIN_LOOP_2;*/

/*	REQUEST_REPLY_HANDLE working_handle = MEMORY_HANDLE_MAIN_LOOP_2;
	REQUEST_REPLY_HANDLE packet_getting_handle = MEMORY_HANDLE_MAIN_LOOP_1;*/
	PACKET_ASSOCIATED_DATA working_handle = {MEMORY_HANDLE_MAIN_LOOP_2, MEMORY_HANDLE_MAIN_LOOP_2_SAOUDP_ADDR, MEMORY_HANDLE_INVALID, 0 };
	PACKET_ASSOCIATED_DATA packet_getting_handle = {MEMORY_HANDLE_MAIN_LOOP_1, MEMORY_HANDLE_MAIN_LOOP_1_SAOUDP_ADDR, MEMORY_HANDLE_INVALID, 0 };

	uint16_t bus_id;
	uint16_t ack_bus_id;
	uint8_t signal_level = 0; // TODO: source?
	uint8_t error_count = 0; // TODO: source?


	for (;;)
	{
		// 1. Wait for an event
		ret_code = COMMLAYER_RET_PENDING;
		INCREMENT_COUNTER_IF( 91, "MAIN LOOP, packet received [1]", ret_code == COMMLAYER_RET_OK );
		while ( ret_code == COMMLAYER_RET_PENDING )
		{
wait_for_comm_event:
//#ifdef TEST_AVR
#if !defined TEST_RAM_CONSUMPTION

			// [[QUICK CHECK FOR UNITS POTENTIALLY WAITING FOR TIMEOUT start]]
			// we ask each potential unit; if it reports activity, let it continue; otherwise, ask a next one
			// IMPORTANT: once an order of units is selected and tested, do not change it without extreme necessity
			HAL_GET_TIME( &(currt), TIME_REQUEST_POINT__LOOP_TOP );

#if 1 //SIOT_MESH_IMPLEMENTATION_WORKS
			// 0. Test MESH
			ret_code = handler_siot_mesh_timer( &currt, &wait_for, working_handle.packet_h, &bus_id );
			switch ( ret_code )
			{
				case SIOT_MESH_RET_PASS_TO_SEND:
				{
					// TODO: implement
					zepto_response_to_request( working_handle.packet_h );
					goto hal_send;
					break;
				}
			}
#endif // SIOT_MESH_IMPLEMENTATION_WORKS

			// 1.1. test GDP-ctr
			ret_code = handler_sagdp_timer( &currt, &wait_for, NULL, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_CTR, MEMORY_HANDLE_SAGDP_LSM_CTR_SAOUDP_ADDR, &sagdp_context_ctr, &(working_handle.resend_cnt) );
			if ( ret_code == SAGDP_RET_NEED_NONCE )
			{
				// NOTE: here we assume that, if GDP has something to re-send by timer, working_handle is not in use (say, by CCP)
				ret_code = handler_sasp_get_packet_id( nonce );
				ZEPTO_DEBUG_ASSERT( ret_code == SASP_RET_NONCE );
				ret_code = handler_sagdp_timer( &currt, &wait_for, nonce, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_CTR, MEMORY_HANDLE_SAGDP_LSM_CTR_SAOUDP_ADDR, &sagdp_context_ctr, &(working_handle.resend_cnt) );
				ZEPTO_DEBUG_ASSERT( ret_code != SAGDP_RET_NEED_NONCE && ret_code != SAGDP_RET_OK );
				zepto_response_to_request( working_handle.packet_h );
				zepto_response_to_request( working_handle.addr_h );
				goto saspsend;
			}

			// 1.2. test GDP-app
			ret_code = handler_sagdp_timer( &currt, &wait_for, NULL, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_APP, MEMORY_HANDLE_SAGDP_LSM_APP_SAOUDP_ADDR, &sagdp_context_app, &(working_handle.resend_cnt) );
			if ( ret_code == SAGDP_RET_NEED_NONCE )
			{
				// NOTE: here we assume that, if GDP has something to re-send by timer, working_handle is not in use (say, by CCP)
				ret_code = handler_sasp_get_packet_id( nonce );
				ZEPTO_DEBUG_ASSERT( ret_code == SASP_RET_NONCE );
				ret_code = handler_sagdp_timer( &currt, &wait_for, nonce, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_APP, MEMORY_HANDLE_SAGDP_LSM_APP_SAOUDP_ADDR, &sagdp_context_app, &(working_handle.resend_cnt) );
				ZEPTO_DEBUG_ASSERT( ret_code != SAGDP_RET_NEED_NONCE && ret_code != SAGDP_RET_OK );
				zepto_response_to_request( working_handle.packet_h );
				zepto_response_to_request( working_handle.addr_h );
				goto saspsend;
			}

			// 2. test CCP
			ret_code = handler_saccp_timer( working_handle.packet_h, /*sasp_nonce_type chain_id*/NULL, &currt, &ret_wf );

			switch ( ret_code )
			{
				case SACCP_RET_NO_WAITING:
				{
					break;
				}
				case SACCP_RET_PASS_LOWER:
				{
					// NOTE:
					zepto_response_to_request( working_handle.packet_h );
					ZEPTO_DEBUG_PRINTF_4( "SACCP1: ret: %d; rq_size: %d, rsp_size: %d\n", ret_code, ugly_hook_get_request_size( working_handle.packet_h ), ugly_hook_get_response_size( working_handle.packet_h ) );
					goto alt_entry;
					break;
				}
				case SACCP_RET_DONE:
				{
					zepto_parser_free_memory( working_handle.packet_h );
					break;
				}
				case SACCP_RET_WAIT:
				{
					break;
				}
			}

			// [[QUICK CHECK FOR UNITS POTENTIALLY WAITING FOR TIMEOUT end]]


			ret_code = HAL_WAIT_FOR( &wait_for );
			SA_TIME_SET_INFINITE_TIME( wait_for.wait_time );

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
#if (defined MESH_TEST) && (defined SA_RETRANSMITTER)
					uint8_t bus_id = hal_get_busid_of_last_packet();
					ret_code = hal_get_packet_bytes( packet_getting_handle.packet_h );
					switch ( ret_code )
					{
						case HAL_GET_PACKET_BYTES_FAILED:
						{
							zepto_parser_free_memory( packet_getting_handle.packet_h );
							return 0; // TODO: think about recovery (later attempts, etc)
							goto start_over;
							break;
						}
						case HAL_GET_PACKET_BYTES_DONE:
						{
							zepto_response_to_request( packet_getting_handle.packet_h );
							hal_send_packet( packet_getting_handle.packet_h, 1-bus_id, 0 );
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
#else							
					// regular processing will be done below in the next block
					HAL_GET_PACKET_BYTES( packet_getting_handle.packet_h );
					zepto_response_to_request( packet_getting_handle.packet_h );
					SWAP_PACKET_HANDLE_PAIR( working_handle, packet_getting_handle); // TODO: for "old" packet working handle must be restored!!!
					goto siotmp_rec;
#if 0
					ret_code = hal_get_packet_bytes( packet_getting_handle.packet_h );
					switch ( ret_code )
					{
						case HAL_GET_PACKET_BYTES_FAILED:
						{
							zepto_parser_free_memory( packet_getting_handle.packet_h );
							return 0; // TODO: think about recovery (later attempts, etc)
							goto start_over;
							break;
						}
						case HAL_GET_PACKET_BYTES_DONE:
						{
							zepto_response_to_request( packet_getting_handle.packet_h );
//							REQUEST_REPLY_HANDLE tmp_h = working_handle; working_handle = packet_getting_handle; packet_getting_handle = tmp_h;
							SWAP_PACKET_HANDLE_PAIR( working_handle, packet_getting_handle); // TODO: for "old" packet working handle must be restored!!!
							goto siotmp_rec;
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
#endif // 0
#endif
				}
				case WAIT_RESULTED_IN_TIMEOUT:
				{
//					ZEPTO_DEBUG_PRINTF_1( "no reply received; the last message (if any) will be resent by timer\n" );
#if 0
					HAL_GET_TIME( &(currt) );
					gdp_context = SAGDP_CONTEXT_UNKNOWN;
					ret_code = handler_sagdp_timer( &gdp_context, &currt, &wait_for, NULL, working_handle.packet_h, working_handle.addr_h/*, &sagdp_data*/ );
					if ( ret_code == SAGDP_RET_OK )
					{
						zepto_response_to_request( working_handle.packet_h );
						zepto_response_to_request( working_handle.addr_h );
						goto wait_for_comm_event;
					}
					else if ( ret_code == SAGDP_RET_NEED_NONCE )
					{
						ret_code = handler_sasp_get_packet_id( nonce );
						ZEPTO_DEBUG_ASSERT( ret_code == SASP_RET_NONCE );
						ret_code = handler_sagdp_timer( &gdp_context, &currt, &wait_for, nonce, working_handle.packet_h, working_handle.addr_h/*, &sagdp_data*/ );
						ZEPTO_DEBUG_ASSERT( ret_code != SAGDP_RET_NEED_NONCE && ret_code != SAGDP_RET_OK );
						zepto_response_to_request( working_handle.packet_h );
						zepto_response_to_request( working_handle.addr_h );
						goto saspsend;
						break;
					}
					else
					{
						ZEPTO_DEBUG_PRINTF_2( "ret_code = %d\n", ret_code );
						ZEPTO_DEBUG_ASSERT( 0 );
					}
#endif // 0

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
#else // TEST_RAM_CONSUMPTION
			for ( j=0; j<sizeof(INCOMING_TEST_PACKET); j++ )
				zepto_write_uint8( working_handle.packet_h, ZEPTO_PROG_CONSTANT_READ_BYTE( INCOMING_TEST_PACKET + j ) );
			zepto_response_to_request( working_handle.packet_h );
			goto sasp_rec;
#endif // TEST_RAM_CONSUMPTION


#if 0
//			if ( timer_val && getTime() >= wake_time )
//			if ( tact.action )
//			if ( sagdp_data.event_type ) //TODO: temporary solution
			if ( 1 ) //TODO: temporary solution
			{
				ZEPTO_DEBUG_PRINTF_1( "no reply received; the last message (if any) will be resent by timer\n" );
				HAL_GET_TIME( &(currt) );
				gdp_context = SAGDP_CONTEXT_UNKNOWN;
				ret_code = handler_sagdp_timer( &gdp_context, &currt, &wait_for, NULL, working_handle.packet_h, working_handle.addr_h/*, &sagdp_data*/ );
				if ( ret_code == SAGDP_RET_OK )
				{
					zepto_response_to_request( working_handle.packet_h );
					zepto_response_to_request( working_handle.addr_h );
					goto wait_for_comm_event;
				}
				else if ( ret_code == SAGDP_RET_NEED_NONCE )
				{
					ret_code = handler_sasp_get_packet_id( nonce );
					ZEPTO_DEBUG_ASSERT( ret_code == SASP_RET_NONCE );
					ret_code = handler_sagdp_timer( &gdp_context, &currt, &wait_for, nonce, working_handle.packet_h, working_handle.addr_h/*, &sagdp_data*/ );
		ZEPTO_DEBUG_PRINTF_2( "ret_code = %d\n", ret_code );
					ZEPTO_DEBUG_ASSERT( ret_code != SAGDP_RET_NEED_NONCE && ret_code != SAGDP_RET_OK );
					zepto_response_to_request( working_handle.packet_h );
					zepto_response_to_request( working_handle.addr_h );
					goto saspsend;
					break;
				}
				else
				{
					ZEPTO_DEBUG_PRINTF_2( "ret_code = %d\n", ret_code );
					ZEPTO_DEBUG_ASSERT( 0 );
				}
			}
			else if ( wait_for_incoming_chain_with_timer && getTime() >= wake_time_to_start_new_chain )
			{
				wait_for_incoming_chain_with_timer = false;
				zepto_response_to_request( working_handle.packet_h );
				gdp_context = SAGDP_CONTEXT_UNKNOWN;
				goto alt_entry;
				break;
			}
#endif
		}
		if ( ret_code != COMMLAYER_RET_OK )
		{
			ZEPTO_DEBUG_PRINTF_1("\n\nWAITING FOR ESTABLISHING COMMUNICATION WITH SERVER...\n\n");
			if (!communication_initialize()) // regardles of errors... quick and dirty solution so far
				return -1;
			goto start_over;
		}
		ZEPTO_DEBUG_PRINTF_1("Message from client received\n");
		ZEPTO_DEBUG_PRINTF_4( "ret: %d; rq_size: %d, rsp_size: %d\n", ret_code, ugly_hook_get_request_size( working_handle.packet_h ), ugly_hook_get_response_size( working_handle.packet_h ) );


		// 2.0. Pass to siot/mesh
siotmp_rec:
#if 1 //SIOT_MESH_IMPLEMENTATION_WORKS
#ifdef USED_AS_RETRANSMITTER
		ret_code = handler_siot_mesh_receive_packet( &currt, &wait_for, working_handle.packet_h, MEMORY_HANDLE_MESH_ACK, &(working_handle.mesh_val), signal_level, error_count, &bus_id, &ack_bus_id );
#else // USED_AS_RETRANSMITTER
		ret_code = handler_siot_mesh_receive_packet( &currt, &wait_for, working_handle.packet_h, MEMORY_HANDLE_MESH_ACK, &(working_handle.mesh_val), signal_level, error_count, &ack_bus_id );
#endif // USED_AS_RETRANSMITTER
		zepto_response_to_request( working_handle.packet_h );

		switch ( ret_code )
		{
			case SIOT_MESH_RET_SEND_ACK_AND_PASS_TO_PROCESS:
			{
				zepto_response_to_request( MEMORY_HANDLE_MESH_ACK );
				HAL_SEND_PACKET( MEMORY_HANDLE_MESH_ACK ); // TODO: ack_bus_id is to be passed here!
				zepto_parser_free_memory( MEMORY_HANDLE_MESH_ACK );
				// regular processing will be done below in the next block
				break;
			}
			case SIOT_MESH_RET_PASS_TO_PROCESS:
			{
				// regular processing will be done below in the next block
				break;
			}
			case SIOT_MESH_RET_PASS_TO_SEND:
			{
				goto hal_send;
				break;
			}
			case SIOT_MESH_RET_SEND_ACK_AND_PASS_TO_SEND:
			{
				zepto_response_to_request( MEMORY_HANDLE_MESH_ACK );
				HAL_SEND_PACKET( MEMORY_HANDLE_MESH_ACK ); // TODO: ack_bus_id is to be passed here!
				zepto_parser_free_memory( MEMORY_HANDLE_MESH_ACK );
				goto hal_send;
				break;
			}
			case SIOT_MESH_RET_OK:
			case SIOT_MESH_RET_GARBAGE_RECEIVED:
			case SIOT_MESH_RET_NOT_FOR_THIS_DEV_RECEIVED:
			{
				goto start_over;
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

		// 2.1. Pass to SAoUDP
//saoudp_rec:
		ret_code = handler_saoudp_receive( working_handle.packet_h, working_handle.addr_h );
		zepto_response_to_request( working_handle.packet_h );
		zepto_response_to_request( working_handle.addr_h );

		switch ( ret_code )
		{
			case SAOUDP_RET_OK:
			{
				// regular processing will be done below in the next block
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


		// 2.2. Pass to SASP
#ifdef TEST_RAM_CONSUMPTION
	sasp_rec:
#endif
#ifdef ALLOW_PRINTING_SASP_INCOMING_MESSAGE
		{
			parser_obj po;
			int ctr = 0;
			zepto_parser_init( &po, working_handle.packet_h );
			ZEPTO_DEBUG_PRINTF_2( "SASP_INCOMING_MESSAGE (%d bytes):\n", zepto_parsing_remaining_bytes( &po ) );
			while ( zepto_parsing_remaining_bytes( &po ) != 0 )
			{
				ZEPTO_DEBUG_PRINTF_2( "0x%02x, ", zepto_parse_uint8( &po ) );
				ctr++;
				if ( (ctr & 7) == 0 )
					ZEPTO_DEBUG_PRINTF_1( "\n" );
			}
			ZEPTO_DEBUG_PRINTF_1( "\n\n" );
		}
#endif // ALLOW_PRINTING_SASP_INCOMING_MESSAGE
		ret_code = handler_sasp_receive( AES_ENCRYPTION_KEY, pid, working_handle.packet_h );
		zepto_response_to_request( working_handle.packet_h );
		ZEPTO_DEBUG_PRINTF_4( "SASP1:  ret: %d; rq_size: %d, rsp_size: %d\n", ret_code, ugly_hook_get_request_size( working_handle.packet_h ), ugly_hook_get_response_size( working_handle.packet_h ) );
		switch ( ret_code )
		{
			case SASP_RET_IGNORE_PACKET_BROKEN:
			case SASP_RET_IGNORE_PACKET_NONCE_LS_NOT_APPLIED:
			case SASP_RET_IGNORE_PACKET_LAST_REPEATED:
			{
				ZEPTO_DEBUG_PRINTF_1( "BAD PACKET RECEIVED\n" );
#if 1 //SIOT_MESH_IMPLEMENTATION_WORKS
				handler_siot_mesh_packet_rejected_broken( /*MEMORY_HANDLE mem_h, */&(working_handle.mesh_val) );
#endif
				goto start_over;
				break;
			}
			case SASP_RET_TO_LOWER_ERROR:
			{
				goto saoudp_send;
				break;
			}
			case SASP_RET_TO_HIGHER_NEW:
			{
				// regular processing will be done below in the next block
				break;
			}
			case SASP_RET_TO_HIGHER_LAST_SEND_FAILED: // as a result of error-old-nonce
			{
				bool use_ctr = true;
				ZEPTO_DEBUG_PRINTF_1( "NONCE_LAST_SENT has been reset; the last message (if any) will be resent\n" );
				HAL_GET_TIME( &(currt), TIME_REQUEST_POINT__SASP_RET_TO_HIGHER_LAST_SEND_FAILED ); // motivation: requested after a potentially long operation in sasp handler
				ret_code = handler_sagdp_receive_request_resend_lsp( &currt, &wait_for, NULL, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_CTR, MEMORY_HANDLE_SAGDP_LSM_CTR_SAOUDP_ADDR, &sagdp_context_ctr, &(working_handle.resend_cnt) );
				if ( ret_code == SAGDP_RET_TO_LOWER_NONE )
				{
					use_ctr = false;
					zepto_response_to_request( working_handle.packet_h );
					zepto_response_to_request( working_handle.addr_h );
					ret_code = handler_sagdp_receive_request_resend_lsp( &currt, &wait_for, NULL, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_APP, MEMORY_HANDLE_SAGDP_LSM_APP_SAOUDP_ADDR, &sagdp_context_app, &(working_handle.resend_cnt) );
				}
				if ( ret_code == SAGDP_RET_TO_LOWER_NONE )
				{
					zepto_response_to_request( working_handle.packet_h );
					zepto_response_to_request( working_handle.addr_h );
					goto wait_for_comm_event;
					break;
				}
				if ( ret_code == SAGDP_RET_NEED_NONCE )
				{
					ret_code = handler_sasp_get_packet_id( nonce );
					ZEPTO_DEBUG_ASSERT( ret_code == SASP_RET_NONCE );
					if ( use_ctr )
						ret_code = handler_sagdp_receive_request_resend_lsp( &currt, &wait_for, nonce, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_CTR, MEMORY_HANDLE_SAGDP_LSM_CTR_SAOUDP_ADDR, &sagdp_context_ctr, &(working_handle.resend_cnt) );
					else
						ret_code = handler_sagdp_receive_request_resend_lsp( &currt, &wait_for, nonce, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_APP, MEMORY_HANDLE_SAGDP_LSM_APP_SAOUDP_ADDR, &sagdp_context_app, &(working_handle.resend_cnt) );
					ZEPTO_DEBUG_ASSERT( ret_code != SAGDP_RET_NEED_NONCE && ret_code != SAGDP_RET_TO_LOWER_NONE );
				}
				zepto_response_to_request( working_handle.packet_h );
				zepto_response_to_request( working_handle.addr_h );
				goto saspsend;
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

		// 3. pass to SAGDP a new packet
		HAL_GET_TIME( &(currt), TIME_REQUEST_POINT__AFTER_SASP ); // motivation: requested after a potentially long operation in sasp handler
#ifdef ALLOW_PRINTING_SASP_INCOMING_MESSAGE
		{
			parser_obj po;
			int ctr = 0;
			zepto_parser_init( &po, working_handle.packet_h );
			ZEPTO_DEBUG_PRINTF_2( "SAGDP_INCOMING_MESSAGE (%d bytes):\n", zepto_parsing_remaining_bytes( &po ) );
			while ( zepto_parsing_remaining_bytes( &po ) != 0 )
			{
				ZEPTO_DEBUG_PRINTF_2( "0x%02x, ", zepto_parse_uint8( &po ) );
				ctr++;
				if ( (ctr & 7) == 0 )
					ZEPTO_DEBUG_PRINTF_1( "\n" );
			}
			ZEPTO_DEBUG_PRINTF_1( "\n\n" );
		}
#endif // ALLOW_PRINTING_SASP_INCOMING_MESSAGE
		bool for_ctr = handler_sagdp_is_up_packet_ctr( working_handle.packet_h );
		ZEPTO_DEBUG_PRINTF_2( "Packet understood as for %s\n", for_ctr ? "CONTROL" : "APP" );
		if ( for_ctr )
		{
			ret_code = handler_sagdp_receive_up( &currt, &wait_for, NULL, pid, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_CTR, MEMORY_HANDLE_SAGDP_LSM_CTR_SAOUDP_ADDR, &sagdp_context_ctr, &(working_handle.resend_cnt) );
			if ( ret_code == SAGDP_RET_START_OVER_FIRST_RECEIVED )
			{
				sagdp_init( &sagdp_context_ctr );
				// TODO: do remaining reinitialization
				ret_code = handler_sagdp_receive_up( &currt, &wait_for, NULL, pid, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_CTR, MEMORY_HANDLE_SAGDP_LSM_CTR_SAOUDP_ADDR, &sagdp_context_ctr, &(working_handle.resend_cnt) );
				ZEPTO_DEBUG_ASSERT( ret_code != SAGDP_RET_START_OVER_FIRST_RECEIVED );
			}
			if ( ret_code == SAGDP_RET_NEED_NONCE )
			{
				ret_code = handler_sasp_get_packet_id( nonce );
				ZEPTO_DEBUG_ASSERT( ret_code == SASP_RET_NONCE );
				ret_code = handler_sagdp_receive_up( &currt, &wait_for, nonce, pid, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_CTR, MEMORY_HANDLE_SAGDP_LSM_CTR_SAOUDP_ADDR, &sagdp_context_ctr, &(working_handle.resend_cnt) );
				ZEPTO_DEBUG_ASSERT( ret_code != SAGDP_RET_NEED_NONCE );
			}
			zepto_response_to_request( working_handle.packet_h );
			ZEPTO_DEBUG_PRINTF_4( "SAGDP1 (ctr): ret: %d; rq_size: %d, rsp_size: %d\n", ret_code, ugly_hook_get_request_size( working_handle.packet_h ), ugly_hook_get_response_size( working_handle.packet_h ) );

			switch ( ret_code )
			{
				case SAGDP_RET_SYS_CORRUPTED:
				{
					// TODO: reinitialize all
					sagdp_init( &sagdp_context_ctr );
	//				zepto_response_to_request( working_handle.packet_h );
					goto saspsend;
					break;
				}
				case SAGDP_RET_TO_HIGHER:
				{
					ret_code = handler_saccp_receive( working_handle.packet_h, /*sasp_nonce_type chain_id*/NULL, &currt, &ret_wf );
					ZEPTO_DEBUG_ASSERT( ret_code == SACCP_RET_PASS_LOWER_CONTROL );
					zepto_response_to_request( working_handle.packet_h );
					// HAL_GET_TIME( &(currt) ); // TODO: check whether above processing of CTR packets is a potentially long operation and time should be re-requested
					working_handle.resend_cnt = 0;
					ret_code = handler_sagdp_receive_hlp( &currt, &wait_for, NULL, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_CTR, MEMORY_HANDLE_SAGDP_LSM_CTR_SAOUDP_ADDR, &sagdp_context_ctr );
					if ( ret_code == SAGDP_RET_NEED_NONCE )
					{
						ret_code = handler_sasp_get_packet_id( nonce );
						ZEPTO_DEBUG_ASSERT( ret_code == SASP_RET_NONCE );
						ret_code = handler_sagdp_receive_hlp( &currt, &wait_for, nonce, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_CTR, MEMORY_HANDLE_SAGDP_LSM_CTR_SAOUDP_ADDR, &sagdp_context_ctr );
						ZEPTO_DEBUG_ASSERT( ret_code != SAGDP_RET_NEED_NONCE );
					}
					zepto_response_to_request( working_handle.packet_h );
					ZEPTO_DEBUG_PRINTF_4( "SAGDP2 (ctr): ret: %d; rq_size: %d, rsp_size: %d\n", ret_code, ugly_hook_get_request_size( working_handle.packet_h ), ugly_hook_get_response_size( working_handle.packet_h ) );
			
					switch ( ret_code )
					{
						case SAGDP_RET_TO_LOWER_NEW:
						{
							goto saspsend;
							break;
						}
						case SAGDP_RET_SYS_CORRUPTED: // TODO: is it possible here?
						{
							// TODO: process reset
							sagdp_init( &sagdp_context_ctr );
							zepto_response_to_request( working_handle.packet_h );
							goto saspsend;
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
				case SAGDP_RET_TO_LOWER_REPEATED:
				{
					goto saspsend;
				}
				case SAGDP_RET_OK:
				{
#if 1 //SIOT_MESH_IMPLEMENTATION_WORKS
					handler_siot_mesh_packet_rejected_broken( /*MEMORY_HANDLE mem_h, */&(working_handle.mesh_val) );
#endif
					goto start_over;
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

		// LIKELY BRANCH: PAACKET IS FOR APP

		ZEPTO_DEBUG_ASSERT( !for_ctr ); // we are not supposed to go through the above code
		ret_code = handler_sagdp_receive_up( &currt, &wait_for, NULL, pid, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_APP, MEMORY_HANDLE_SAGDP_LSM_APP_SAOUDP_ADDR, &sagdp_context_app, &(working_handle.resend_cnt) );
		if ( ret_code == SAGDP_RET_START_OVER_FIRST_RECEIVED )
		{
			sagdp_init( & sagdp_context_app );
			// TODO: do remaining reinitialization
			ret_code = handler_sagdp_receive_up( &currt, &wait_for, NULL, pid, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_APP, MEMORY_HANDLE_SAGDP_LSM_APP_SAOUDP_ADDR, &sagdp_context_app, &(working_handle.resend_cnt) );
			ZEPTO_DEBUG_ASSERT( ret_code != SAGDP_RET_START_OVER_FIRST_RECEIVED );
		}
		if ( ret_code == SAGDP_RET_NEED_NONCE )
		{
			ret_code = handler_sasp_get_packet_id( nonce );
			ZEPTO_DEBUG_ASSERT( ret_code == SASP_RET_NONCE );
			ret_code = handler_sagdp_receive_up( &currt, &wait_for, nonce, pid, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_APP, MEMORY_HANDLE_SAGDP_LSM_APP_SAOUDP_ADDR, &sagdp_context_app, &(working_handle.resend_cnt) );
			ZEPTO_DEBUG_ASSERT( ret_code != SAGDP_RET_NEED_NONCE );
		}
		zepto_response_to_request( working_handle.packet_h );
		ZEPTO_DEBUG_PRINTF_4( "SAGDP1: ret: %d; rq_size: %d, rsp_size: %d\n", ret_code, ugly_hook_get_request_size( working_handle.packet_h ), ugly_hook_get_response_size( working_handle.packet_h ) );

		switch ( ret_code )
		{
			case SAGDP_RET_SYS_CORRUPTED:
			{
				// TODO: reinitialize all
				sagdp_init( &sagdp_context_app );
//				zepto_response_to_request( working_handle.packet_h );
				goto saspsend;
				break;
			}
			case SAGDP_RET_TO_HIGHER:
			{
				// regular processing will be done below in the next block
				break;
			}
			case SAGDP_RET_TO_LOWER_REPEATED:
			{
				goto saspsend;
			}
			case SAGDP_RET_OK:
			{
#if 1 //SIOT_MESH_IMPLEMENTATION_WORKS
				handler_siot_mesh_packet_rejected_broken( /*MEMORY_HANDLE mem_h, */&(working_handle.mesh_val) );
#endif
				goto start_over;
			}
			default:
			{
				// unexpected ret_code
				ZEPTO_DEBUG_PRINTF_2( "Unexpected ret_code %d\n", ret_code );
				ZEPTO_DEBUG_ASSERT( 0 );
				break;
			}
		}


		// 4. Process received command (yoctovm)
#ifdef ALLOW_PRINTING_SASP_INCOMING_MESSAGE
		{
			parser_obj po;
			int ctr = 0;
			zepto_parser_init( &po, working_handle.packet.h );
			ZEPTO_DEBUG_PRINTF_2( "SACCP_INCOMING_MESSAGE (%d bytes):\n", zepto_parsing_remaining_bytes( &po ) );
			while ( zepto_parsing_remaining_bytes( &po ) != 0 )
			{
				ZEPTO_DEBUG_PRINTF_2( "0x%02x, ", zepto_parse_uint8( &po ) );
				ctr++;
				if ( (ctr & 7) == 0 )
					ZEPTO_DEBUG_PRINTF_1( "\n" );
			}
			ZEPTO_DEBUG_PRINTF_1( "\n\n" );
		}
#endif // ALLOW_PRINTING_SASP_INCOMING_MESSAGE
		ret_code = handler_saccp_receive( working_handle.packet_h, /*sasp_nonce_type chain_id*/NULL, &currt, &ret_wf );
#if 0
		wait_for_incoming_chain_with_timer = false;
#endif

		switch ( ret_code )
		{
			case SACCP_RET_PASS_LOWER:
			{
				zepto_response_to_request( working_handle.packet_h );
				ZEPTO_DEBUG_PRINTF_4( "SACCP1: ret: %d; rq_size: %d, rsp_size: %d\n", ret_code, ugly_hook_get_request_size( working_handle.packet_h ), ugly_hook_get_response_size( working_handle.packet_h ) );
				// regular processing will be done below in the next block
				break;
			}
			case SACCP_RET_DONE:
			{
				zepto_parser_free_memory( working_handle.packet_h );
				goto start_over;
				break;
			}
			case SACCP_RET_WAIT:
			{
				goto start_over;
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


		// 5. SAGDP
alt_entry:
//		uint8_t timer_val;
		HAL_GET_TIME( &(currt), TIME_REQUEST_POINT__AFTER_SACCP );
#ifdef ALLOW_PRINTING_SASP_INCOMING_MESSAGE
		{
			parser_obj po;
			int ctr = 0;
			zepto_parser_init( &po, working_handle.packet.h );
			ZEPTO_DEBUG_PRINTF_2( "SAGDP_INCOMING_MESSAGE [back] (%d bytes):\n", zepto_parsing_remaining_bytes( &po ) );
			while ( zepto_parsing_remaining_bytes( &po ) != 0 )
			{
				ZEPTO_DEBUG_PRINTF_2( "0x%02x, ", zepto_parse_uint8( &po ) );
				ctr++;
				if ( (ctr & 7) == 0 )
					ZEPTO_DEBUG_PRINTF_1( "\n" );
			}
			ZEPTO_DEBUG_PRINTF_1( "\n\n" );
		}
#endif // ALLOW_PRINTING_SASP_INCOMING_MESSAGE
		ZEPTO_DEBUG_ASSERT( !for_ctr ); // we are not supposed to go through the above code
		working_handle.resend_cnt = 0;
		ret_code = handler_sagdp_receive_hlp( &currt, &wait_for, NULL, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_APP, MEMORY_HANDLE_SAGDP_LSM_APP_SAOUDP_ADDR, &sagdp_context_app );
		if ( ret_code == SAGDP_RET_NEED_NONCE )
		{
			ret_code = handler_sasp_get_packet_id( nonce );
			ZEPTO_DEBUG_ASSERT( ret_code == SASP_RET_NONCE );
			ret_code = handler_sagdp_receive_hlp( &currt, &wait_for, nonce, working_handle.packet_h, working_handle.addr_h, MEMORY_HANDLE_SAGDP_LSM_APP, MEMORY_HANDLE_SAGDP_LSM_APP_SAOUDP_ADDR, &sagdp_context_app );
			ZEPTO_DEBUG_ASSERT( ret_code != SAGDP_RET_NEED_NONCE );
		}
		zepto_response_to_request( working_handle.packet_h );
//		zepto_response_to_request( working_handle.addr_h );
		ZEPTO_DEBUG_PRINTF_4( "SAGDP2: ret: %d; rq_size: %d, rsp_size: %d\n", ret_code, ugly_hook_get_request_size( working_handle.packet_h ), ugly_hook_get_response_size( working_handle.packet_h ) );

		switch ( ret_code )
		{
			case SAGDP_RET_TO_LOWER_NEW:
			{
				// regular processing will be done below in the next block
				// set timer
				break;
			}
			case SAGDP_RET_OK: // TODO: is it possible here?
			{
				goto start_over;
				break;
			}
			case SAGDP_RET_TO_LOWER_REPEATED: // TODO: is it possible here?
			{
				goto start_over;
				break;
			}
			case SAGDP_RET_SYS_CORRUPTED: // TODO: is it possible here?
			{
				// TODO: process reset
				sagdp_init( &sagdp_context_app );
//				bool start_now = tester_get_rand_val() % 3;
//				bool start_now = true;
//				wake_time_to_start_new_chain = start_now ? getTime() : getTime() + tester_get_rand_val() % 8;
//				wake_time_to_start_new_chain = getTime();
#if 0
				wait_for_incoming_chain_with_timer = true;
#endif
				zepto_response_to_request( working_handle.packet_h );
				goto saspsend;
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

		// SASP
saspsend:
#ifdef ALLOW_PRINTING_SASP_INCOMING_MESSAGE
		{
			parser_obj po;
			int ctr = 0;
			zepto_parser_init( &po, working_handle.packet_h );
			ZEPTO_DEBUG_PRINTF_2( "SASP_INCOMING_MESSAGE [back] (%d bytes):\n", zepto_parsing_remaining_bytes( &po ) );
			while ( zepto_parsing_remaining_bytes( &po ) != 0 )
			{
				ZEPTO_DEBUG_PRINTF_2( "0x%02x, ", zepto_parse_uint8( &po ) );
				ctr++;
				if ( (ctr & 7) == 0 )
					ZEPTO_DEBUG_PRINTF_1( "\n" );
			}
			ZEPTO_DEBUG_PRINTF_1( "\n\n" );
		}
#endif // ALLOW_PRINTING_SASP_INCOMING_MESSAGE
		ret_code = handler_sasp_send( AES_ENCRYPTION_KEY, nonce, working_handle.packet_h );
		zepto_response_to_request( working_handle.packet_h );
		ZEPTO_DEBUG_PRINTF_4( "SASP2:  ret: %d; rq_size: %d, rsp_size: %d\n", ret_code, ugly_hook_get_request_size( working_handle.packet_h ), ugly_hook_get_response_size( working_handle.packet_h ) );

		switch ( ret_code )
		{
			case SASP_RET_TO_LOWER_REGULAR:
			{
				// regular processing will be done below in the next block
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


		// Pass to SAoUDP
saoudp_send:
#ifdef ALLOW_PRINTING_SASP_OUTGOING_MESSAGE
		{
			parser_obj po;
			int ctr = 0;
			zepto_parser_init( &po, working_handle.packet_h );
			ZEPTO_DEBUG_PRINTF_2( "SASP_OUTGOING_MESSAGE (%d bytes):\n", zepto_parsing_remaining_bytes( &po ) );
			while ( zepto_parsing_remaining_bytes( &po ) != 0 )
			{
				ZEPTO_DEBUG_PRINTF_2( "0x%02x, ", zepto_parse_uint8( &po ) );
				ctr++;
				if ( (ctr & 7) == 0 )
					ZEPTO_DEBUG_PRINTF_1( "\n" );
			}
			ZEPTO_DEBUG_PRINTF_1( "\n\n" );
		}
#endif // ALLOW_PRINTING_SASP_OUTGOING_MESSAGE
#ifdef TEST_RAM_CONSUMPTION
		{
			parser_obj po;
			zepto_parser_init( &po, working_handle.packet_h );
			test_byte = true;
			for ( j=0; j<sizeof(OUTGOING_TEST_PACKET); j++ )
			{
				test_byte = test_byte && zepto_parsing_remaining_bytes( &po ) != 0 && ZEPTO_PROG_CONSTANT_READ_BYTE( OUTGOING_TEST_PACKET + j ) == zepto_parse_uint8( &po );
			}
			ZEPTO_DEBUG_PRINTF_2( "Testing output: %s\n\n", test_byte ? "OK" : "FAILED" );
			test_byte ++;
			return 0;
		}
#endif // TEST_RAM_CONSUMPTION
		ret_code = handler_saoudp_send( working_handle.packet_h, working_handle.addr_h );
		zepto_response_to_request( working_handle.packet_h );
		zepto_parser_free_memory( working_handle.addr_h );

		switch ( ret_code )
		{
			case SAOUDP_RET_OK:
			{
				// regular processing will be done below in the next block
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

#if 1 //SIOT_MESH_IMPLEMENTATION_WORKS
		ret_code = handler_siot_mesh_send_packet( &currt, &wait_for, working_handle.packet_h, working_handle.mesh_val, working_handle.resend_cnt, 0, &bus_id ); // we can send it only to root, if we're slave TODO: think regarding second argument
		zepto_response_to_request( working_handle.packet_h );

		switch ( ret_code )
		{
			case SIOT_MESH_RET_OK:
			{
				// regular processing will be done below in the next block
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

hal_send:
#ifdef MESH_TEST
#ifdef SA_RETRANSMITTER
			ret_code = hal_send_packet( working_handle.packet_h, 0, 0 );
#else
			ret_code = hal_send_packet( working_handle.packet_h, 0, 0 );
#endif
#else
//			ZEPTO_DEBUG_ASSERT( bus_id == 0 ); // TODO: bus_id must be a part of send_packet() call; we are now just in the middle of development...
			HAL_SEND_PACKET( working_handle.packet_h );
#endif
			zepto_parser_free_memory( working_handle.packet_h );
			INCREMENT_COUNTER( 90, "MAIN LOOP, packet sent" );
			ZEPTO_DEBUG_PRINTF_1("\nMessage replied to client\n");

start_over:;
#ifdef TEST_RAM_CONSUMPTION
			return 0;
#endif // TEST_RAM_CONSUMPTION
	}

	return 0;
}

#endif // (defined VERY_DEBUG) && (defined VERY_DEBUG_SIMPLE_MAIN_LOOP)
