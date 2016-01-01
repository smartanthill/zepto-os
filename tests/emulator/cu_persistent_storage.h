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

#if !defined __HAL_EEPROM_H__
#define __HAL_EEPROM_H__

#include <simpleiot/siot_common.h>
#include <simpleiot_hal/siot_eeprom.h>

#define HAL_PS_INIT_FAILED 0
#define HAL_PS_INIT_OK 1
#define HAL_PS_INIT_OK_NEEDS_INITIALIZATION 2

#define MAX_FILELDS_PER_RECORD 16
#define MAX_FIELD_SIZE 512
#define MAX_FIELD_SIZE_AVAILABLE ( MAX_FIELD_SIZE - 2 )
#define MAX_DEVICE_COUNT 64

#ifdef __cplusplus
extern "C" {
#endif

uint8_t hal_init_eeprom_access( char* path );

bool hal_eeprom_write( const uint8_t* data, uint16_t size, uint16_t address );
bool hal_eeprom_read( uint8_t* data, uint16_t size, uint16_t address);
void hal_eeprom_flush();

bool write_field( uint16_t device_id, uint8_t field_id, uint16_t data_sz, uint8_t* data );
bool read_field( uint16_t device_id, uint8_t field_id, uint16_t* data_sz, uint8_t* data );
bool init_default_storage();

#ifdef __cplusplus
}
#endif

#endif // __HAL_EEPROM_H__