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
#include "../../common/sadlp_transport.h"

static bool serial_init (void)
{
	Serial.begin(SADLP_SERIAL_BAUDRATE);
    while (!Serial) {
        // wait for serial port to connect. Needed for Leonardo only
    }
    return true;
}

static uint8_t serial_read (void)
{
	return (uint8_t) Serial.read();
}

static uint32_t serial_write (uint8_t byte)
{
	return Serial.write(byte);
}

static bool serial_available (void)
{
	return Serial.available();
}


extern const sadlp_transport sadlp_serial_transport =
{
	serial_init,
	serial_read,
	serial_write,
	serial_available
};