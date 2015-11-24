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
//#include <simpleiot_hal/siot_eeprom.h>
//#include <zepto_mem_mngmt_hal_spec.h>
//#include <simpleiot_hal/hal_waiting.h>
//#include <simpleiot_hal/siot_mem_mngmt.h>
#include "air_commlayer.h"

//#include <simpleiot_hal/siot_eeprom.h>
//#include <hal_eeprom.h>

extern uint8_t send_debug_packet( const uint8_t* data_buff, uint16_t data_sz );
extern uint8_t get_debug_packet( uint8_t* buff, uint16_t* packet_data_sz, uint16_t packet_sz_max );
extern bool communication_initialize_with_time_master();


#ifdef USE_TIME_MASTER

const uint16_t DEVICE_SELF_ID = 0xFFFF;
#define MAX_PACKET_SIZE 1024

// record types:
#define TIME_RECORD_REGISTER_INCOMING_PACKET 0
#define TIME_RECORD_REGISTER_OUTGOING_PACKET 1
#define TIME_RECORD_REGISTER_DEVICE_CONNECT 3
#define TIME_RECORD_REGISTER_DEVICE_DISCONNECT 4
#define TIME_RECORD_REGISTER_RAND_VAL_REQUEST_32 5
#define TIME_RECORD_REGISTER_PACKET_WAIT_RES 6

#define OUTGOING_DEBUG_PACKET_HEADER_SIZE 5
#ifdef USE_TIME_MASTER_REGISTER
#define INCOMING_DEBUG_PACKET_HEADER_SIZE 5
#else // USE_TIME_MASTER_REGISTER
#define INCOMING_DEBUG_PACKET_HEADER_SIZE 6
#endif // USE_TIME_MASTER_REGISTER


uint16_t form_debug_packet( uint8_t* buff, uint8_t type, const uint8_t* data_buff, uint16_t data_sz_1 )
{
	uint16_t data_sz = data_sz_1;
	buff[0] = (uint8_t)DEVICE_SELF_ID;
	buff[1] = DEVICE_SELF_ID >> 8;
	buff[2] = type;
	buff[3] = (uint8_t)data_sz;
	buff[4] = data_sz >> 8;
	if ( data_sz_1 )
	{
		ZEPTO_DEBUG_ASSERT( data_buff != NULL );
		ZEPTO_MEMCPY( buff + 5, data_buff, data_sz_1 );
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


void register_packet( uint16_t target, const uint8_t* packet_buff, uint16_t packet_sz, uint8_t ret_code, uint8_t type )
{
	uint8_t interm_packet_buff[ MAX_PACKET_SIZE + 1];
	interm_packet_buff[0] = (uint8_t)target;
	interm_packet_buff[1] = (uint8_t)(target >> 8);
	interm_packet_buff[2] = ret_code;
	if ( packet_sz )
	{
		ZEPTO_DEBUG_ASSERT( packet_buff != NULL );
		ZEPTO_MEMCPY( interm_packet_buff + 3, packet_buff, packet_sz );
	}

	uint8_t ret;
	uint8_t buff[OUTGOING_DEBUG_PACKET_HEADER_SIZE + MAX_PACKET_SIZE + 3];

	uint16_t debug_packet_sz = form_debug_packet( buff, type, interm_packet_buff, packet_sz + 3 );
	ret = send_debug_packet( buff, debug_packet_sz );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint16_t packet_data_sz;
	ret = get_debug_packet( buff, &packet_data_sz, OUTGOING_DEBUG_PACKET_HEADER_SIZE + MAX_PACKET_SIZE );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	preanalyze_debug_packet( buff, packet_data_sz, &packet_data_sz, type );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == 0 );
}

void register_incoming_packet(  uint16_t target, const uint8_t* packet_buff, uint16_t packet_sz, uint8_t ret_code )
{
	register_packet( target, packet_buff, packet_sz, ret_code, TIME_RECORD_REGISTER_INCOMING_PACKET );
}

void register_outgoing_packet(  uint16_t target, const uint8_t* packet_buff, uint16_t packet_sz, uint8_t ret_code )
{
	register_packet( target, packet_buff, packet_sz, ret_code, TIME_RECORD_REGISTER_OUTGOING_PACKET );
}

void register_device_connect(  uint16_t dev_id, uint8_t ret_code )
{
	register_packet( dev_id, NULL, 0, ret_code, TIME_RECORD_REGISTER_DEVICE_CONNECT );
}

void register_device_disconnect(  uint16_t dev_id, uint8_t ret_code )
{
	register_packet( dev_id, NULL, 0, ret_code, TIME_RECORD_REGISTER_DEVICE_DISCONNECT );
}

void register_packet_wait_res( uint16_t* src, uint8_t cnt, uint8_t ret_code )
{
	register_packet( 0xFFFF, (uint8_t*)src, cnt * sizeof(uint16_t), ret_code, TIME_RECORD_REGISTER_PACKET_WAIT_RES );
}


#else // USE_TIME_MASTER_REGISTER

uint8_t request_packet( uint16_t dev_id, uint8_t* buff, uint16_t* size, uint8_t type_out )
{
	uint8_t ret;
	uint8_t tmp_buff[OUTGOING_DEBUG_PACKET_HEADER_SIZE+MAX_PACKET_SIZE+1];
	uint16_t debug_packet_sz = form_debug_packet( tmp_buff, type_out, buff, *size );
	ret = send_debug_packet( tmp_buff, debug_packet_sz );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint16_t packet_data_sz;
	ret = get_debug_packet( buff, &packet_data_sz, OUTGOING_DEBUG_PACKET_HEADER_SIZE+MAX_PACKET_SIZE+1 );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	uint8_t data_offset = preanalyze_debug_packet( buff, packet_data_sz, &packet_data_sz, type_out );

	ZEPTO_DEBUG_ASSERT( packet_data_sz > 0 );
	uint8_t* data_in = buff + data_offset;

	if ( packet_data_sz >= 3 )
	{
		uint16_t _dev_id = data_in[1];
		_dev_id <<= 8;
		_dev_id += data_in[0];
		ZEPTO_DEBUG_ASSERT(_dev_id == dev_id );

		ret = data_in[2];

		data_in += 3;
		ZEPTO_MEMMOV( buff, data_in, packet_data_sz - 3 );
		*size = packet_data_sz - 3;
	}
	else
	{
		ret = data_in[0];
	}

	return ret;
}

uint8_t request_incoming_packet( uint16_t dev_id, uint8_t* buff, uint16_t* size )
{
	*size = 0;
	uint8_t ret = request_packet( dev_id, buff, size, TIME_RECORD_REGISTER_INCOMING_PACKET );
	return ret;
}

uint8_t request_outgoing_packet( uint16_t dev_id, const uint8_t* buff, uint16_t size )
{
	uint8_t temp_buff[MAX_PACKET_SIZE+MAX_PACKET_SIZE+1];
	uint16_t _size = size + 3;
	temp_buff[0] = (uint8_t)dev_id;
	temp_buff[1] = (uint8_t)(dev_id >> 8);
	temp_buff[2] = 1;
	ZEPTO_MEMCPY( temp_buff + 3, buff, size );

	uint8_t ret = request_packet( dev_id, temp_buff, &_size, TIME_RECORD_REGISTER_OUTGOING_PACKET );
//	ZEPTO_DEBUG_ASSERT( size == _size );
	return ret;
}

uint8_t request_packet_wait_res( uint16_t* src, uint8_t* cnt )
{
	uint16_t sz = 0;
	uint8_t ret = request_packet( 0xFFFF, (uint8_t*)src, &sz, TIME_RECORD_REGISTER_PACKET_WAIT_RES );
	*cnt = sz / sizeof(uint16_t);
	return ret;
}

#endif // USE_TIME_MASTER_REGISTER

extern uint8_t try_get_packet_using_context( uint8_t* buff, int max_sz, int*size, int src );
extern uint8_t commlayer_wait_for_packet( uint16_t* src, uint8_t* cnt, uint8_t max_items );
extern uint8_t wait_for_packet_internal( uint16_t* src, uint8_t* cnt, uint8_t max_items );
extern uint8_t debug_start_listening();

uint8_t debug_wait_for_packet( uint16_t* src, uint8_t* cnt, uint8_t max_items )
{
#ifdef USE_TIME_MASTER_REGISTER
	uint8_t ret_code = commlayer_wait_for_packet( src, cnt, max_items );
	register_packet_wait_res( src, *cnt, ret_code );
	return ret_code;
#else
	uint8_t ret_code = request_packet_wait_res( src, cnt );
	debug_start_listening();
	return ret_code;
#endif // USE_TIME_MASTER_REGISTER
}

uint8_t debug_wait_for_packet_internal( uint16_t* src, uint8_t* cnt, uint8_t max_items )
{
#ifdef USE_TIME_MASTER_REGISTER
	uint8_t ret_code = wait_for_packet_internal( src, cnt, max_items );
	register_packet_wait_res( src, *cnt, ret_code );
	return ret_code;
#else
	uint8_t ret_code = request_packet_wait_res( src, cnt );
//	debug_start_listening();
	return ret_code;
#endif // USE_TIME_MASTER_REGISTER
}

uint8_t debug_try_get_packet_using_context( uint8_t* buff, int max_sz, int* size, uint16_t src )
{
#ifdef USE_TIME_MASTER_REGISTER
	uint8_t ret_code = try_get_packet_using_context( buff, max_sz, size, src );
	register_incoming_packet( src, buff, *size, ret_code );
	return ret_code;
#else
	uint16_t sz;
	uint8_t ret = request_incoming_packet( src, buff, &sz );
	*size = sz;
	return ret;
#endif // USE_TIME_MASTER_REGISTER
}

uint8_t debug_send_packet( const uint8_t* buff, int size, uint16_t target )
{
#if !defined USE_TIME_MASTER_REGISTER
	return request_outgoing_packet( target, buff, size );
#else
	register_outgoing_packet( target, buff, size, 1 );
	return commlayer_send_packet( buff, size, target );
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

