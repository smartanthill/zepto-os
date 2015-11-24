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

#if !defined __SA_COMMLAYER_H__
#define __SA_COMMLAYER_H__

#include "air_common.h"

// RET codes
#define COMMLAYER_RET_FAILED 0
#define COMMLAYER_RET_OK 1
#define COMMLAYER_RET_PENDING 2
#define COMMLAYER_RET_REENTER_LISTEN 3
#define COMMLAYER_RET_ZERO_PACKET 4
//#define COMMLAYER_RET_REENTER_SERVICE 4

#define HAL_GET_PACKET_BYTES_IN_PROGRESS 0
#define HAL_GET_PACKET_BYTES_FAILED 1
#define HAL_GET_PACKET_BYTES_DONE 2
#define COMMLAYER_RET_TIMEOUT 12
#define HAL_GET_PACKET_SERVICE 13

#ifdef __cplusplus
/*extern "C" {*/
#endif

extern int dev_count;



bool communication_initialize();
void communication_terminate();

uint8_t commlayer_get_packet( TEST_DATA* test_data, uint8_t* buff, int max_sz, int* size, uint16_t src );
uint8_t commlayer_send_packet( const uint8_t* buff, int size, uint16_t target );
uint8_t commlayer_wait_for_packet( uint16_t* src, uint8_t* cnt, uint8_t max_items );

#ifdef USE_TIME_MASTER
bool debug_communication_initialize();
uint8_t debug_wait_for_packet( uint16_t* src, uint8_t* cnt, uint8_t max_items );
uint8_t debug_wait_for_packet_internal( uint16_t* src, uint8_t* cnt, uint8_t max_items );
uint8_t debug_send_packet( const uint8_t* buff, int size, uint16_t target );
uint8_t debug_try_get_packet_using_context( uint8_t* buff, int max_sz, int* size, uint16_t src );

#define COMMUNICATION_INITIALIZE() debug_communication_initialize()
#define COMMUNICATION_TERMINATE() communication_terminate()
#define WAIT_FOR_PACKET( src, cnt, max_items ) commlayer_wait_for_packet( src, cnt, max_items )
#define GET_PACKET( test_data, buff, max_sz, size,  src ) commlayer_get_packet( test_data, buff, max_sz, size,  src )
#define SEND_PACKET( buff, size, target ) debug_send_packet( buff, size, target )
#else
#define COMMUNICATION_INITIALIZE() communication_initialize()
#define COMMUNICATION_TERMINATE() communication_terminate()
#define GET_PACKET( test_data, buff, max_sz, size,  src ) commlayer_get_packet( test_data, buff, max_sz, size,  src )
#define SEND_PACKET( buff, size, target ) commlayer_send_packet( buff, size, target )
#define WAIT_FOR_PACKET( src, cnt, max_items ) commlayer_wait_for_packet( src, cnt, max_items )
#endif

void get_position( DEVICE_POSITION* pos, uint16_t src );

#ifdef __cplusplus
//}
#endif



#endif // __SA_COMMLAYER_H__