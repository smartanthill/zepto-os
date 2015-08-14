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

#include <simpleiot_hal/hal_commlayer.h>
#include <simpleiot_hal/hal_waiting.h>
#include <zepto_mem_mngmt_hal_spec.h>

#include "../../common/sadlp_protocol.h"
#include "../../common/sadlp_transport.h"

uint8_t hal_wait_for (waiting_for* wf)
{
    for (;;)
    {
        if (wf->wait_packet && handler_sadlp_is_packet(&DATALINK_TRANSPORT))
        {
            return WAIT_RESULTED_IN_PACKET;
        }
    }

    return WAIT_RESULTED_IN_FAILURE;
}

uint8_t wait_for_timeout (uint32_t timeout)
{
    ZEPTO_DEBUG_ASSERT(0);
    return 0;
}

uint8_t hal_get_packet_bytes (MEMORY_HANDLE mem_h)
{
    return handler_sadlp_frame_received (&DATALINK_TRANSPORT, mem_h);
}

bool communication_initialize()
{
    return DATALINK_TRANSPORT.init();
}

uint8_t send_message (MEMORY_HANDLE mem_h)
{
    return handler_sadlp_send_packet (&DATALINK_TRANSPORT, mem_h);
}

void keep_transmitter_on( bool keep_on )
{
    // TODO: add reasonable implementation
}
