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
#include <simpleiot_hal/hal_waiting.h>
#include <simpleiot_hal/hal_commlayer.h>
#include <simpleiot_hal/hal_waiting.h>
#include "sadlp_protocol.h"

#define FRAGMENT_START_CODE 	0x01
#define FRAGMENT_ESCAPE_CODE 	0xFF
#define FRAGMENT_END_CODE 		0x17

#define BUFFER_MAX_LEN 			300

bool handler_sadlp_is_packet(const sa_transport* transport, void* transport_config)
{
    if (transport->readable(transport_config) && transport->read_byte(transport_config) == FRAGMENT_START_CODE)
		return true;
	return false;
}

uint8_t handler_sadlp_send_packet (const sa_transport* transport, void* transport_config, MEMORY_HANDLE mem_h)
{
	uint8_t i = 0;
    uint16_t sz = memory_object_get_request_size (mem_h);
    ZEPTO_DEBUG_ASSERT (sz != 0); // note: any valid message would have to have at least some bytes for headers, etc, so it cannot be empty
    uint8_t* buff = memory_object_get_request_ptr (mem_h);
    ZEPTO_DEBUG_ASSERT (buff != NULL);

    transport->write_byte(transport_config, (uint8_t) FRAGMENT_START_CODE);
    for (i = 0; i < sz; i++) {
        switch (buff[i]) {
            case FRAGMENT_START_CODE:
                transport->write_byte(transport_config, (uint8_t) FRAGMENT_ESCAPE_CODE);
                transport->write_byte(transport_config, (uint8_t) 0x00);
            	break;
            case FRAGMENT_END_CODE:
                transport->write_byte(transport_config, (uint8_t) FRAGMENT_ESCAPE_CODE);
                transport->write_byte(transport_config, (uint8_t) 0x02);
            	break;
            case FRAGMENT_ESCAPE_CODE:
                transport->write_byte(transport_config, (uint8_t) FRAGMENT_ESCAPE_CODE);
                transport->write_byte(transport_config, (uint8_t) 0x03);
            	break;
            default:
                transport->write_byte(transport_config, buff[i]);
            	break;
        }
    }
    transport->write_byte(transport_config, (uint8_t) FRAGMENT_END_CODE);

    return COMMLAYER_RET_OK;

}

uint8_t handler_sadlp_get_packet (const sa_transport* transport, void* transport_config, MEMORY_HANDLE mem_h)
{
	uint8_t buffer[BUFFER_MAX_LEN];
    uint16_t i = 0;
    uint8_t byte = 0;
    uint8_t marker_detected = 0;
    uint32_t timeout = 2000; // in ms
    uint32_t start_reading  = getTime();
    while ((start_reading + timeout) > getTime() && i < BUFFER_MAX_LEN)
    {
        if (!transport->readable(transport_config))
            continue;

        byte = transport->read_byte(transport_config);

        if (byte == FRAGMENT_ESCAPE_CODE)
        {
            marker_detected = true;
            continue;
        }
        else if (byte == FRAGMENT_END_CODE)
        {
            ZEPTO_DEBUG_ASSERT( i && i <= BUFFER_MAX_LEN);
            zepto_write_block( mem_h, buffer, i );
            return WAIT_RESULTED_IN_PACKET;
        }
        else if (marker_detected)
        {
            uint8_t value = 0;
            switch (byte) {
                case 0x00:
                    value = FRAGMENT_START_CODE;
                    break;
                case 0x02:
                    value = FRAGMENT_END_CODE;
                    break;
                case 0x03:
                    value = FRAGMENT_ESCAPE_CODE;
                    break;
                default:
                    return WAIT_RESULTED_IN_FAILURE;
            }
            buffer[i++] = value;
            marker_detected = false;
        }
        else
        {
            buffer[i++] = byte;
        }
    }

    return WAIT_RESULTED_IN_FAILURE;
}