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
    HardwareSerial *_serial = (HardwareSerial*) serial_obj;
    _serial->begin(baudrate);
    while (!_serial) {}
    return true;
}

void sa_hal_serial_read(const void* serial_obj, uint8_t *buffer, uint16_t length)
{
    HardwareSerial *_serial = (HardwareSerial*) serial_obj;
    _serial->readBytes((char*)buffer, length);
}

int8_t sa_hal_serial_read_byte(const void* serial_obj)
{
    HardwareSerial *_serial = (HardwareSerial*) serial_obj;
    return _serial->read();
}

uint16_t sa_hal_serial_write(const void* serial_obj, const uint8_t *buffer, uint16_t length)
{
    HardwareSerial *_serial = (HardwareSerial*) serial_obj;
    return _serial->write(buffer, length);
}

uint8_t sa_hal_serial_write_byte (const void* serial_obj, uint8_t byte)
{
    HardwareSerial *_serial = (HardwareSerial*) serial_obj;
    return _serial->write(byte);
}

bool sa_hal_serial_readable(const void* serial_obj)
{
    HardwareSerial *_serial = (HardwareSerial*) serial_obj;
    return _serial->available();
}
