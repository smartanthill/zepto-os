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

#include "air_commlayer.h"
#include <stdio.h>

#if !defined __COMMSTACK_COMMLAYER_TO_TIME_MASTER_H__
#define __COMMSTACK_COMMLAYER_TO_TIME_MASTER_H__

#ifdef USE_TIME_MASTER

bool communication_initialize_with_time_master();
void communication_terminate_with_time_master();

#ifdef USE_TIME_MASTER_REGISTER

void register_incoming_packet(  uint16_t target, uint8_t* packet_buff, uint16_t packet_sz, uint8_t ret_code );
void register_outgoing_packet(  uint16_t target, uint8_t* packet_buff, uint16_t packet_sz, uint8_t ret_code );

#else // USE_TIME_MASTER_REGISTER

void request_incoming_packet( uint16_t dev_id, uint8_t* buff, uint16_t* size );
void request_outgoing_packet( uint16_t dev_id, uint8_t* buff, uint16_t* size );

#endif // USE_TIME_MASTER_REGISTER

#endif // USE_TIME_MASTER

#endif // __COMMSTACK_COMMLAYER_TO_TIME_MASTER_H__
