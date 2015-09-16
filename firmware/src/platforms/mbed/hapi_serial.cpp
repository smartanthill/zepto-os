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
#include "../../common/hapi_serial.h"

bool hapi_serial_init(const hapi_serial_t *serial, uint16_t baudrate)
{
    Serial *_serial = (Serial*) serial->obj;
    _serial->baud(baudrate);
    return true;
}

void hapi_serial_read(const hapi_serial_t *serial, uint8_t *buffer, uint16_t length)
{
    //Serial *_serial = (Serial*) serial->obj;
    //_serial->read(buffer, length);
}

int8_t hapi_serial_read_byte(const hapi_serial_t *serial)
{
    Serial *_serial = (Serial*) serial->obj;
    return _serial->getc();
}

uint16_t hapi_serial_write(const hapi_serial_t *serial, const uint8_t *buffer, uint16_t length)
{
    //Serial *_serial = (Serial*) serial->obj;
    //return _serial->write(buffer, length);
    return 0;
}

uint8_t hapi_serial_write_byte (const hapi_serial_t *serial, uint8_t byte)
{
    Serial *_serial = (Serial*) serial->obj;
    return _serial->putc(byte);
}

bool hapi_serial_readable(const hapi_serial_t *serial)
{
    Serial *_serial = (Serial*) serial->obj;
    return _serial->readable();
}
