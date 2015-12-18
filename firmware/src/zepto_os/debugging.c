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
#include <simpleiot_hal/siot_eeprom.h>
#include <zepto_mem_mngmt_hal_spec.h>
#include <simpleiot_hal/hal_waiting.h>
#include <simpleiot_hal/siot_mem_mngmt.h>
#include <simpleiot_hal/hal_commlayer.h>

//#include <simpleiot_hal/siot_eeprom.h>
//#include <hal_eeprom.h>

extern uint8_t send_debug_packet( const uint8_t* data_buff, uint16_t data_sz );
extern uint8_t get_debug_packet( uint8_t* buff, uint16_t* packet_data_sz, uint16_t packet_sz_max );
extern bool communication_initialize_with_time_master();


#ifdef USE_TIME_MASTER

extern uint16_t DEVICE_SELF_ID;
#define MAX_PACKET_SIZE 80

// record types:
#define TIME_RECORD_REGISTER_INCOMING_PACKET 0
#define TIME_RECORD_REGISTER_OUTGOING_PACKET 1
#define TIME_RECORD_REGISTER_RAND_VAL_REQUEST_32 2
#define TIME_RECORD_REGISTER_TIME_VALUE 3
#define TIME_RECORD_REGISTER_WAIT_RET_VALUE 4
#define TIME_RECORD_REGISTER_EEPROM_STATE 5

#define OUTGOING_DEBUG_PACKET_HEADER_SIZE 5
#ifdef USE_TIME_MASTER_REGISTER
#define INCOMING_DEBUG_PACKET_HEADER_SIZE 5
#else // USE_TIME_MASTER_REGISTER
#define INCOMING_DEBUG_PACKET_HEADER_SIZE 6
#endif // USE_TIME_MASTER_REGISTER


uint16_t form_debug_packet( uint8_t* buff, uint8_t type, const uint8_t* data_buff_1, uint16_t data_sz_1, const uint8_t* data_buff_2, uint16_t data_sz_2 )
{
	uint16_t data_sz = data_sz_1 + data_sz_2;
	ZEPTO_DEBUG_ASSERT( data_sz <= MAX_PACKET_SIZE );
	buff[0] = (uint8_t)DEVICE_SELF_ID;
	buff[1] = DEVICE_SELF_ID >> 8;
	buff[2] = type;
	buff[3] = (uint8_t)data_sz;
	buff[4] = data_sz >> 8;
	if ( data_sz_1 )
	{
		ZEPTO_DEBUG_ASSERT( data_buff_1 != NULL );
		ZEPTO_MEMCPY( buff + 5, data_buff_1, data_sz_1 );
	}
	if ( data_sz_2 )
	{
		ZEPTO_DEBUG_ASSERT( data_buff_2 != NULL );
		ZEPTO_MEMCPY( buff + 5 + data_sz_1, data_buff_2, data_sz_2 );
	}
	ZEPTO_DEBUG_ASSERT( OUTGOING_DEBUG_PACKET_HEADER_SIZE == 5 ); // if not, then update it!
	return data_sz + OUTGOING_DEBUG_PACKET_HEADER_SIZE;
}

uint8_t preanalyze_debug_packet( uint8_t* buff, uint16_t buff_size, uint16_t* packet_data_sz, uint8_t expected_type )
{
	ZEPTO_DEBUG_ASSERT( buff_size >= INCOMING_DEBUG_PACKET_HEADER_SIZE );

	uint16_t dev_id = buff[1];
	dev_id = (dev_id << 8 ) | buff[0];
	ZEPTO_DEBUG_ASSERT( dev_id == DEVICE_SELF_ID );

	ZEPTO_DEBUG_ASSERT( expected_type == buff[2] );

#ifdef USE_TIME_MASTER_REGISTER
	*packet_data_sz = buff[4];
	*packet_data_sz = (*packet_data_sz << 8) | buff[3];
	ZEPTO_DEBUG_ASSERT( *packet_data_sz == buff_size - 5 );
	ZEPTO_DEBUG_ASSERT( INCOMING_DEBUG_PACKET_HEADER_SIZE == 5 ); // if not, then update it!
	return INCOMING_DEBUG_PACKET_HEADER_SIZE;
#else
	uint8_t status = buff[3];
	*packet_data_sz = buff[5];
	*packet_data_sz = (*packet_data_sz << 8) | buff[4];
	ZEPTO_DEBUG_ASSERT( status );
	ZEPTO_DEBUG_ASSERT( *packet_data_sz == buff_size - 6 );
	ZEPTO_DEBUG_ASSERT( INCOMING_DEBUG_PACKET_HEADER_SIZE == 6 ); // if not, then update it!
	return INCOMING_DEBUG_PACKET_HEADER_SIZE;
#endif
}



#ifdef USE_TIME_MASTER_REGISTER


void register_packet_with_time_master( uint8_t* packet_buff, uint16_t packet_sz, bool incoming, uint16_t bus_id )
{
	uint8_t ret;
	uint8_t type_out = incoming ? TIME_RECORD_REGISTER_INCOMING_PACKET : TIME_RECORD_REGISTER_OUTGOING_PACKET;
	uint8_t bus_id_buff[2];
	bus_id_buff[0] = (uint8_t)bus_id;
	bus_id_buff[1] = (uint8_t)(bus_id>>8);
	uint8_t buff[OUTGOING_DEBUG_PACKET_HEADER_SIZE + MAX_PACKET_SIZE];

	uint16_t debug_packet_sz = form_debug_packet( buff, type_out, bus_id_buff, 2, packet_buff, packet_sz );
	ret = send_debug_packet( buff, debug_packet_sz );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint16_t packet_data_sz;
	ret = get_debug_packet( buff, &packet_data_sz, OUTGOING_DEBUG_PACKET_HEADER_SIZE + MAX_PACKET_SIZE );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	preanalyze_debug_packet( buff, packet_data_sz, &packet_data_sz, type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == 0 );
}


void register_incoming_packet( MEMORY_HANDLE mem_h, uint16_t bus_id )
{
	uint16_t packet_sz = memory_object_get_response_size( mem_h );
	uint8_t* packet_buff = memory_object_get_response_ptr( mem_h );
	register_packet_with_time_master( packet_buff, packet_sz, true, bus_id );
}

void register_outgoing_packet( MEMORY_HANDLE mem_h, uint16_t bus_id )
{
	uint16_t packet_sz = memory_object_get_request_size( mem_h );
	uint8_t* packet_buff = memory_object_get_request_ptr( mem_h );
	register_packet_with_time_master( packet_buff, packet_sz, false, bus_id );
}

void register_wait_request_ret_val( uint8_t ret_val, uint16_t bus_id )
{
	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_WAIT_RET_VALUE;
	uint8_t buff[OUTGOING_DEBUG_PACKET_HEADER_SIZE+3];
	uint8_t bus_id_buff[2];
	bus_id_buff[0] = (uint8_t)bus_id;
	bus_id_buff[1] = (uint8_t)(bus_id>>8);

	uint16_t debug_packet_sz = form_debug_packet( buff, type_out, &ret_val, 1, bus_id_buff, 2 );
	ret = send_debug_packet( buff, debug_packet_sz );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint16_t packet_data_sz;
	ret = get_debug_packet( buff, &packet_data_sz, OUTGOING_DEBUG_PACKET_HEADER_SIZE+MAX_PACKET_SIZE+1 );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	uint8_t data_offset = preanalyze_debug_packet( buff, packet_data_sz, &packet_data_sz, type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == 1 );
	ZEPTO_DEBUG_ASSERT( buff[data_offset] == ret_val );
}

void register_time_val( uint8_t point_id, const sa_time_val* in, sa_time_val* out )
{
	uint8_t data_buff[sizeof(sa_time_val) + 1];
	data_buff[0] = point_id;
	data_buff[1] = (uint8_t)(in->low_t);
	data_buff[2] = (uint8_t)(in->low_t>>8);
	data_buff[3] = (uint8_t)(in->high_t);
	data_buff[4] = (uint8_t)(in->high_t>>8);

	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_TIME_VALUE;
	uint8_t buff[OUTGOING_DEBUG_PACKET_HEADER_SIZE + sizeof(sa_time_val) + 1];

	uint16_t debug_packet_sz = form_debug_packet( buff, type_out, data_buff, sizeof(sa_time_val) + 1, 0, 0 );
	ret = send_debug_packet( buff, debug_packet_sz );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint16_t packet_data_sz;
	ret = get_debug_packet( buff, &packet_data_sz, OUTGOING_DEBUG_PACKET_HEADER_SIZE+MAX_PACKET_SIZE+1 );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	uint8_t data_offset = preanalyze_debug_packet( buff, packet_data_sz, &packet_data_sz, type_out );

	ZEPTO_DEBUG_ASSERT( packet_data_sz == sizeof(sa_time_val) + 1 );
	uint8_t* data_in = buff + data_offset;
	ZEPTO_DEBUG_ASSERT( data_in[0] == point_id );
	out->low_t = data_in[2];
	out->low_t = (out->low_t<<8) | data_in[1];
	out->high_t = data_in[4];
	out->high_t = (out->high_t<<8) | data_in[3];
}

void register_eeprom_state()
{
	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_EEPROM_STATE;
	uint8_t buff[OUTGOING_DEBUG_PACKET_HEADER_SIZE + EEPROM_SERIALIZED_SIZE ];
	uint8_t eeprom_buff[EEPROM_SERIALIZED_SIZE ];
	eeprom_serialize( eeprom_buff );

	uint16_t debug_packet_sz = form_debug_packet( buff, type_out, eeprom_buff, EEPROM_SERIALIZED_SIZE, 0, 0 );
	ret = send_debug_packet( buff, debug_packet_sz );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint16_t packet_data_sz;
	ret = get_debug_packet( buff, &packet_data_sz, OUTGOING_DEBUG_PACKET_HEADER_SIZE+MAX_PACKET_SIZE+1 );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	uint8_t data_offset = preanalyze_debug_packet( buff, packet_data_sz, &packet_data_sz, type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == 0 );
}


#else // USE_TIME_MASTER_REGISTER

void request_incoming_packet( MEMORY_HANDLE mem_h, uint16_t bus_id )
{
	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_INCOMING_PACKET;
	uint8_t buff[INCOMING_DEBUG_PACKET_HEADER_SIZE + MAX_PACKET_SIZE ];

	uint16_t debug_packet_sz = form_debug_packet( buff, type_out, NULL, 0, 0, 0 );
	ret = send_debug_packet( buff, debug_packet_sz );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint16_t packet_data_sz;
	ret = get_debug_packet( buff, &packet_data_sz, OUTGOING_DEBUG_PACKET_HEADER_SIZE+MAX_PACKET_SIZE+1 );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	uint8_t data_offset = preanalyze_debug_packet( buff, packet_data_sz, &packet_data_sz, type_out );

	ZEPTO_DEBUG_ASSERT( packet_data_sz > 0 );
	uint8_t* data_in = buff + data_offset;

	ZEPTO_DEBUG_ASSERT( data_in[0] == (uint8_t)bus_id );
	ZEPTO_DEBUG_ASSERT( data_in[1] == (uint8_t)(bus_id>>8) );

	zepto_parser_free_memory( mem_h );
	zepto_write_block( mem_h, data_in + 2, packet_data_sz - 2 );
}

void request_outgoing_packet( MEMORY_HANDLE mem_h, uint16_t bus_id )
{
	uint16_t packet_sz = memory_object_get_request_size( mem_h );
	uint8_t* packet_buff = memory_object_get_request_ptr( mem_h );
	uint8_t bus_id_buff[2];
	bus_id_buff[0] = (uint8_t)bus_id;
	bus_id_buff[1] = (uint8_t)(bus_id>>8);

	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_OUTGOING_PACKET;
	uint8_t buff[INCOMING_DEBUG_PACKET_HEADER_SIZE + MAX_PACKET_SIZE + 3 ];

	uint16_t debug_packet_sz = form_debug_packet( buff, type_out, bus_id_buff, 2, packet_buff, packet_sz );
	ret = send_debug_packet( buff, debug_packet_sz );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint16_t packet_data_sz;
	ret = get_debug_packet( buff, &packet_data_sz, OUTGOING_DEBUG_PACKET_HEADER_SIZE+MAX_PACKET_SIZE+1 );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	uint8_t data_offset = preanalyze_debug_packet( buff, packet_data_sz, &packet_data_sz, type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == 1 );
}

uint8_t request_wait_request_ret_val( uint16_t* bus_id )
{
	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_WAIT_RET_VALUE;
	uint8_t buff[INCOMING_DEBUG_PACKET_HEADER_SIZE + MAX_PACKET_SIZE ];

	uint16_t debug_packet_sz = form_debug_packet( buff, type_out, NULL, 0, NULL, 0 );
	ret = send_debug_packet( buff, debug_packet_sz );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint16_t packet_data_sz;
	ret = get_debug_packet( buff, &packet_data_sz, OUTGOING_DEBUG_PACKET_HEADER_SIZE+MAX_PACKET_SIZE+1 );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	uint8_t data_offset = preanalyze_debug_packet( buff, packet_data_sz, &packet_data_sz, type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == 3 );
	
	*bus_id = buff[data_offset+2];
	*bus_id <<= 8;
	*bus_id += buff[data_offset+1];
	
	return buff[data_offset];
}

void request_time_val( uint8_t point_id, sa_time_val* tv )
{
	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_TIME_VALUE;

	uint8_t buff[INCOMING_DEBUG_PACKET_HEADER_SIZE + MAX_PACKET_SIZE ];

	uint16_t debug_packet_sz = form_debug_packet( buff, type_out, NULL, 0, NULL, 0 );
	ret = send_debug_packet( buff, debug_packet_sz );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint16_t packet_data_sz;
	ret = get_debug_packet( buff, &packet_data_sz, OUTGOING_DEBUG_PACKET_HEADER_SIZE+MAX_PACKET_SIZE+1 );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	uint8_t data_offset = preanalyze_debug_packet( buff, packet_data_sz, &packet_data_sz, type_out );

	ZEPTO_DEBUG_ASSERT( packet_data_sz == sizeof(sa_time_val) + 1 );
	uint8_t* data_in = buff + data_offset;
	ZEPTO_DEBUG_ASSERT( data_in[0] == point_id );
	tv->low_t = data_in[2];
	tv->low_t = (tv->low_t<<8) | data_in[1];
	tv->high_t = data_in[4];
	tv->high_t = (tv->high_t<<8) | data_in[3];
}

void request_eeprom_state()
{
	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_EEPROM_STATE;

	uint8_t buff[INCOMING_DEBUG_PACKET_HEADER_SIZE + EEPROM_SERIALIZED_SIZE ];

	uint16_t debug_packet_sz = form_debug_packet( buff, type_out, NULL, 0, NULL, 0 );
	ret = send_debug_packet( buff, debug_packet_sz );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint16_t packet_data_sz;
	ret = get_debug_packet( buff, &packet_data_sz, OUTGOING_DEBUG_PACKET_HEADER_SIZE+MAX_PACKET_SIZE+1 );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	uint8_t data_offset = preanalyze_debug_packet( buff, packet_data_sz, &packet_data_sz, type_out );

	ZEPTO_DEBUG_ASSERT( packet_data_sz == EEPROM_SERIALIZED_SIZE );
	uint8_t* data_in = buff + data_offset;
	eeprom_deserialize( data_in );
}

#endif // USE_TIME_MASTER_REGISTER

typedef struct _GETTIME_CALL_POINT { const char* file; uint16_t line;  } GETTIME_CALL_POINT;
#define TIME_REQUEST_POINT_MAX 4
static GETTIME_CALL_POINT gettime_call_points[TIME_REQUEST_POINT_MAX];
void check_get_time_call_point( uint8_t call_point, const char* file, uint16_t line )
{
	ZEPTO_DEBUG_ASSERT( call_point < TIME_REQUEST_POINT_MAX );
	if ( gettime_call_points[call_point].file == NULL )
	{
		gettime_call_points[call_point].file = file;
		gettime_call_points[call_point].line = line;
	}
	else
	{
		ZEPTO_DEBUG_ASSERT( gettime_call_points[call_point].line == line );
		uint8_t i;
		bool same = true;
		for ( i=0; i<255; i++ )
		{
			if ( gettime_call_points[call_point].file[i] == 0 )
			{
				same = file[i] == 0;
				break;
			}
			same = same && gettime_call_points[call_point].file[i] == file[i];
		}
		same == same && (i < 255);
		ZEPTO_DEBUG_ASSERT( same );
	}
}


void debug_hal_get_packet_bytes( MEMORY_HANDLE mem_h, uint16_t bus_id )
{
#ifdef USE_TIME_MASTER_REGISTER
	hal_get_packet_bytes( mem_h, bus_id );
	register_incoming_packet( mem_h, bus_id );
	return;
#else
	request_incoming_packet( mem_h, bus_id );
	return;
#endif // USE_TIME_MASTER_REGISTER
}

void debug_hal_send_message( MEMORY_HANDLE mem_h, uint16_t bus_id )
{
#if !defined USE_TIME_MASTER_REGISTER
	request_outgoing_packet( mem_h, bus_id );
	return;
#else
	register_outgoing_packet( mem_h, bus_id );
	send_message( mem_h, bus_id );
	return;
#endif // USE_TIME_MASTER_REGISTER
}

void  debug_hal_get_time( sa_time_val* tv, uint8_t call_point, const char* file, uint16_t line )
{
	check_get_time_call_point( call_point, file, line );

#ifdef USE_TIME_MASTER_REGISTER
	sa_get_time( tv );
	register_time_val( call_point, tv, tv );
	return;
#else
	request_time_val( call_point, tv );
	return;
#endif // USE_TIME_MASTER_REGISTER
}

uint8_t debug_hal_wait_for( waiting_for* wf, uint16_t* bus_id )
{
#ifdef USE_TIME_MASTER_REGISTER
	uint8_t ret_code = hal_wait_for( wf, bus_id );
	register_wait_request_ret_val( ret_code, *bus_id );
	return ret_code;
#else
	return request_wait_request_ret_val( bus_id );
#endif // USE_TIME_MASTER_REGISTER
}

bool debug_communication_initialize()
{
#ifdef USE_TIME_MASTER_REGISTER
	if ( !communication_initialize() )
		return false;
	if ( !communication_initialize_with_time_master() )
		return false;
	return true;
#else
	if ( !communication_initialize_with_time_master() )
		return false;
	return true;
#endif
}

#endif // USE_TIME_MASTER

