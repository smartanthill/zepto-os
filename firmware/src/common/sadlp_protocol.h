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

#if !defined __SADLP_PROTOCOL_H__
#define __SADLP_PROTOCOL_H__

#include <simpleiot/siot_common.h>
#include "sa_transport.h"
#include "../sa_transports_list.h"


#ifdef __cplusplus
extern "C" {
#endif

bool handler_sadlp_is_packet(const sa_transport* transport, void* transport_state);
uint8_t handler_sadlp_send_packet(const sa_transport* transport, void* transport_state, MEMORY_HANDLE mem_h);
uint8_t handler_sadlp_get_packet(const sa_transport* transport, void* transport_state, MEMORY_HANDLE mem_h);

#ifdef __cplusplus
}
#endif

#endif // __SADLP_PROTOCOL_H__