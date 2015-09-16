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

static bool serial_init (const void* transport_config, void* transport_state)
{
	serial_transport_config* tc = (serial_transport_config*) transport_config;
	serial_transport_state* ts = (serial_transport_state*) transport_state;
	return hapi_serial_init(ts->serial, tc->baudrate);
}

static void serial_read (const void* transport_state, uint8_t *buffer, uint16_t length)
{
	serial_transport_state* ts = (serial_transport_state*) transport_state;
	hapi_serial_read(ts->serial, buffer, length);
}

static int8_t serial_read_byte (const void* transport_state)
{
	serial_transport_state* ts = (serial_transport_state*) transport_state;
	return hapi_serial_read_byte(ts->serial);
}

static uint16_t serial_write (const void* transport_state, const uint8_t *buffer, uint16_t length)
{
	serial_transport_state* ts = (serial_transport_state*) transport_state;
	return hapi_serial_write(ts->serial, buffer, length);
}

static uint8_t serial_write_byte (const void* transport_state, uint8_t byte)
{
	serial_transport_state* ts = (serial_transport_state*) transport_state;
	return hapi_serial_write_byte (ts->serial, byte);
}

static bool serial_readable(const void* transport_state)
{
	serial_transport_state* ts = (serial_transport_state*) transport_state;
	return hapi_serial_readable (ts->serial);
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
