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
#include "../../common/sa_transport.h"
#include "serial.h"

static bool serial_init (const void* transport_config)
{
	serial_transport_config* tc = (serial_transport_config*) transport_config;
	return hapi_serial_init(tc->serial, tc->baudrate);
}

static void serial_read (const void* transport_config, uint8_t *buffer, uint16_t length)
{
	serial_transport_config* tc = (serial_transport_config*) transport_config;
	hapi_serial_read(tc->serial, buffer, length);
}

static int8_t serial_read_byte (const void* transport_config)
{
	serial_transport_config* tc = (serial_transport_config*) transport_config;
	return hapi_serial_read_byte(tc->serial);
}

static uint16_t serial_write (const void* transport_config, const uint8_t *buffer, uint16_t length)
{
	serial_transport_config* tc = (serial_transport_config*) transport_config;
	return hapi_serial_write(tc->serial, buffer, length);
}

static uint8_t serial_write_byte (const void* transport_config, uint8_t byte)
{
	serial_transport_config* tc = (serial_transport_config*) transport_config;
	return hapi_serial_write_byte (tc->serial, byte);
}

static bool serial_readable(const void* transport_config)
{
	serial_transport_config* tc = (serial_transport_config*) transport_config;
	return hapi_serial_readable (tc->serial);
}

extern const sa_transport serial_transport =
{
	serial_init,
	serial_read,
	serial_read_byte,
	serial_write,
	serial_write_byte,
	serial_readable
};
