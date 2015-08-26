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
#include "../../common/hal_serial.h"

bool sa_hal_serial_init(const void* serial_obj, uint16_t baudrate)
{
    Serial *_serial = (Serial*) serial_obj;
    _serial->baud(baudrate);
    return true;
}

void sa_hal_serial_read(const void* serial_obj, uint8_t *buffer, uint16_t length)
{
    //Serial *_serial = (Serial*) serial_obj;
    //_serial->read(buffer, length);
}

int8_t sa_hal_serial_read_byte(const void* serial_obj)
{
    Serial *_serial = (Serial*) serial_obj;
    return _serial->getc();
}

uint16_t sa_hal_serial_write(const void* serial_obj, const uint8_t *buffer, uint16_t length)
{
    //Serial *_serial = (Serial*) serial_obj;
    //return _serial->write(buffer, length);
    return 0;
}

uint8_t sa_hal_serial_write_byte (const void* serial_obj, uint8_t byte)
{
    Serial *_serial = (Serial*) serial_obj;
    return _serial->putc(byte);
}

bool sa_hal_serial_readable(const void* serial_obj)
{
    Serial *_serial = (Serial*) serial_obj;
    return _serial->readable();
}
