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

#if !defined __HAL_SERIAL_H__
#define __HAL_SERIAL_H__

#include <simpleiot/siot_common.h>

#ifdef __cplusplus
extern "C" {
#endif

bool sa_hal_serial_init(const void* serial_obj, uint16_t baudrate);
void sa_hal_serial_read(const void* serial_obj, uint8_t *buffer, uint16_t length);
int8_t sa_hal_serial_read_byte(const void* serial_obj);
uint16_t sa_hal_serial_write(const void* serial_obj, const uint8_t *buffer, uint16_t length);
uint8_t sa_hal_serial_write_byte (const void* serial_obj, uint8_t byte);
bool sa_hal_serial_readable(const void* serial_obj);

#ifdef __cplusplus
}
#endif

#endif // __HAL_SERIAL_H__
