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

#if !defined __HAPI_SERIAL_H__
#define __HAPI_SERIAL_H__

#include <simpleiot/siot_common.h>

typedef enum _hapi_serial_type
{
    SERIAL_TYPE_HARDWARE,
    SERIAL_TYPE_SOFTWARE
} hapi_serial_type;

typedef struct _hapi_serial_t
{
    void* obj;
    hapi_serial_type type;
} hapi_serial_t;

#ifdef __cplusplus
extern "C" {
#endif

bool hapi_serial_init(const hapi_serial_t* serial_obj, uint16_t baudrate);
void hapi_serial_read(const hapi_serial_t* serial_obj, uint8_t *buffer, uint16_t length);
int8_t hapi_serial_read_byte(const hapi_serial_t* serial_obj);
uint16_t hapi_serial_write(const hapi_serial_t* serial_obj, const uint8_t *buffer, uint16_t length);
uint8_t hapi_serial_write_byte (const hapi_serial_t* serial_obj, uint8_t byte);
bool hapi_serial_readable(const hapi_serial_t* serial_obj);

#ifdef __cplusplus
}
#endif

#endif // __HAPI_SERIAL_H__
