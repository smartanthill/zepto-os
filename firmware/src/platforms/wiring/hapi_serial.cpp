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

#include <SoftwareSerial.h>

bool hapi_serial_init(const hapi_serial_t* serial, uint16_t baudrate)
{
    if (serial->type == SERIAL_TYPE_HARDWARE)
    {
       HardwareSerial *_serial = (HardwareSerial*) serial->obj;
        _serial->begin(baudrate);
        while (!_serial) {}
    }
    else
    {
        static SoftwareSerial *_serial = (SoftwareSerial*) serial->obj;
        _serial->begin(baudrate);
        while (!_serial) {}
    }

    return true;
}

void hapi_serial_read(const hapi_serial_t* serial, uint8_t *buffer, uint16_t length)
{
    if (serial->type == SERIAL_TYPE_HARDWARE)
    {
        HardwareSerial *_serial = (HardwareSerial*) serial->obj;
        _serial->readBytes((char*)buffer, length);
    }
    else
    {
        SoftwareSerial *_serial = (SoftwareSerial*) serial->obj;
        _serial->readBytes((char*)buffer, length);
    }
}

int8_t hapi_serial_read_byte(const hapi_serial_t* serial)
{
    if (serial->type == SERIAL_TYPE_HARDWARE)
    {
        HardwareSerial *_serial = (HardwareSerial*) serial->obj;
        return _serial->read();
    }
    else
    {
        SoftwareSerial *_serial = (SoftwareSerial*) serial->obj;
        return _serial->read();
    }
}

uint16_t hapi_serial_write(const hapi_serial_t* serial, const uint8_t *buffer, uint16_t length)
{
    if (serial->type == SERIAL_TYPE_HARDWARE)
    {
        HardwareSerial *_serial = (HardwareSerial*) serial->obj;
        return _serial->write(buffer, length);
    }
    else
    {
        SoftwareSerial *_serial = (SoftwareSerial*) serial->obj;
        return _serial->write(buffer, length);
    }
}

uint8_t hapi_serial_write_byte (const hapi_serial_t* serial, uint8_t byte)
{
    if (serial->type == SERIAL_TYPE_HARDWARE)
    {
        HardwareSerial *_serial = (HardwareSerial*) serial->obj;
        return _serial->write(byte);
    }
    else
    {
        SoftwareSerial *_serial = (SoftwareSerial*) serial->obj;
        return _serial->write(byte);
    }
}

bool hapi_serial_readable(const hapi_serial_t* serial)
{
    if (serial->type == SERIAL_TYPE_HARDWARE)
    {
        HardwareSerial *_serial = (HardwareSerial*) serial->obj;
        return _serial->available();
    }
    else
    {
        SoftwareSerial *_serial = (SoftwareSerial*) serial->obj;
        return _serial->available();
    }
}
