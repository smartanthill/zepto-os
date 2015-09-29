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

#include <zepto_mem_mngmt_hal_spec.h>
#include <simpleiot_hal/hal_commlayer.h>
#include <simpleiot_hal/hal_waiting.h>
#include <stdio.h>

#if !defined __HAL_COMMLAYER_TO_TIME_MASTER_H__
#define __HAL_COMMLAYER_TO_TIME_MASTER_H__

void register_incoming_packet( MEMORY_HANDLE mem_h );
void register_outgoing_packet( MEMORY_HANDLE mem_h );
void register_wait_request_ret_val( uint8_t ret_val );
void register_time_val( const sa_time_val* in, sa_time_val* out );

void request_incoming_packet( MEMORY_HANDLE mem_h );
void request_outgoing_packet( MEMORY_HANDLE mem_h );
uint8_t request_wait_request_ret_val()
void request_time_val( sa_time_val* tv );

#endif // __HAL_COMMLAYER_TO_TIME_MASTER_H__
