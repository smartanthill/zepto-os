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
#include "../../common/sa_transport.h"
#include "../../common/sadlp_protocol.h"
#include "../../transports/serial/serial.h"


uint8_t hal_wait_for( waiting_for* wf, uint16_t* bus_id )
{
    ZEPTO_DEBUG_ASSERT( wf->wait_legs == 0 ); // not implemented
    ZEPTO_DEBUG_ASSERT( wf->wait_i2c == 0 ); // not implemented

    for (;;)
    {
        if (wf->wait_packet)
        {
            uint16_t i;
            for (i = 0; i < ZEPTO_PROG_CONSTANT_READ_BYTE(&SA_BUSES_MAX); i++)
            {
                sa_transport* transport = (sa_transport*) ZEPTO_PROG_CONSTANT_READ_PTR(&(buses[i].t));
                serial_transport_config* tc = (serial_transport_config*) ZEPTO_PROG_CONSTANT_READ_PTR(&(buses[i].t_config));
                if (handler_sadlp_is_packet(transport, tc))
                {
                    *bus_id = i;
                    return WAIT_RESULTED_IN_PACKET;
                }
            }
        }
    }

    return WAIT_RESULTED_IN_FAILURE;
}

uint8_t wait_for_timeout (uint32_t timeout)
{
    ZEPTO_DEBUG_ASSERT(0);
    return 0;
}

uint8_t hal_get_packet_bytes (MEMORY_HANDLE mem_h, uint16_t bus_id)
{
    sa_transport* transport = (sa_transport*) ZEPTO_PROG_CONSTANT_READ_PTR(&(buses[bus_id].t));
    serial_transport_config* tc = (serial_transport_config*) ZEPTO_PROG_CONSTANT_READ_PTR(&(buses[bus_id].t_config));
    return handler_sadlp_get_packet (transport, tc, mem_h);
}

bool communication_initialize()
{
    uint16_t i;
    for (i = 0; i < ZEPTO_PROG_CONSTANT_READ_BYTE(&SA_BUSES_MAX); i++)
    {
        sa_transport* transport = (sa_transport*) ZEPTO_PROG_CONSTANT_READ_PTR(&(buses[i].t));
        transport->init((void*)ZEPTO_PROG_CONSTANT_READ_PTR(&(buses[i].t_config)));
    }
    return true;
}

uint8_t send_message (MEMORY_HANDLE mem_h, uint16_t bus_id )
{
    sa_transport* transport = (sa_transport*) ZEPTO_PROG_CONSTANT_READ_PTR(&(buses[bus_id].t));
    serial_transport_config* tc = (serial_transport_config*) ZEPTO_PROG_CONSTANT_READ_PTR(&(buses[bus_id].t_config));
    return handler_sadlp_send_packet (transport, tc, mem_h);
}

void keep_transmitter_on( bool keep_on )
{
    // TODO: add reasonable implementation
}
