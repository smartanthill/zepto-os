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

#if !defined __DEBUGGING_H__
#define __DEBUGGING_H__

#include <simpleiot/siot_common.h>
#include <zepto_mem_mngmt_hal_spec.h>
#include <simpleiot_hal/hal_waiting.h>
#include <simpleiot_hal/siot_mem_mngmt.h>

#define TIME_REQUEST_POINT__LOOP_TOP 0
#define TIME_REQUEST_POINT__SASP_RET_TO_HIGHER_LAST_SEND_FAILED 1
#define TIME_REQUEST_POINT__AFTER_SASP 2
#define TIME_REQUEST_POINT__AFTER_SACCP 3
#define TIME_REQUEST_POINT_MAX 4 // keep updated!!!

#ifdef USE_TIME_MASTER

bool debug_communication_initialize();
void debug_hal_get_packet_bytes( MEMORY_HANDLE mem_h );
void debug_hal_send_message( MEMORY_HANDLE mem_h );
void  debug_hal_get_time( sa_time_val* tv, uint8_t call_point, const char* file, uint16_t line );
uint8_t debug_hal_wait_for( waiting_for* wf );

#ifdef USE_TIME_MASTER_REGISTER
void register_eeprom_state();
#define DEBUG_ON_EEPROM_INIT() register_eeprom_state()
#else
void request_eeprom_state();
#define DEBUG_ON_EEPROM_INIT() request_eeprom_state()
#endif
#define HAL_COMMUNICATION_INITIALIZE debug_communication_initialize
#define HAL_GET_PACKET_BYTES( packet_handle ) debug_hal_get_packet_bytes( packet_handle )
#define HAL_SEND_PACKET( packet_handle ) debug_hal_send_message( packet_handle )
#define HAL_GET_TIME( time_val_ptr, REQUEST_POINT )  debug_hal_get_time( time_val_ptr, REQUEST_POINT, __FILE__, (uint16_t)__LINE__ )
#define HAL_WAIT_FOR( wait_for_ptr ) debug_hal_wait_for( wait_for_ptr )

#else // USE_TIME_MASTER

#define DEBUG_ON_EEPROM_INIT()
#define HAL_COMMUNICATION_INITIALIZE communication_initialize
#define HAL_GET_PACKET_BYTES( packet_handle ) hal_get_packet_bytes( packet_handle )
#define HAL_SEND_PACKET( packet_handle ) send_message( packet_handle )
#define HAL_GET_TIME( time_val_ptr, REQUEST_POINT )  sa_get_time( time_val_ptr )
#define HAL_WAIT_FOR( wait_for_ptr ) hal_wait_for( wait_for_ptr )

#endif // USE_TIME_MASTER

#endif // __DEBUGGING_H__

