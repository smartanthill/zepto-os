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

#include <hal_eeprom.h>
#include "stm32f4xx_eeprom.h"

uint16_t address_table[EEPROM_SIZE] = {0};

static inline bool _validate_operation (uint16_t size, uint16_t address)
{
    if ((address+size > EEPROM_SIZE) || (size > EEPROM_SIZE) || (address > EEPROM_SIZE))
        return false;
    else
        return true;
}

bool hal_init_eeprom_access()
{
    bool result = false;
    /* Init memory map shift */
    for (uint32_t i = 0; i < EEPROM_SIZE; i++) {
        address_table[i] = i;
    }

    HAL_FLASH_Unlock();
    if (eeprom_init() == 0)
        result = true;
    else
        result = false;
    HAL_FLASH_Lock();
    return result;
}

bool hal_eeprom_write( const uint8_t* data, uint16_t size, uint16_t address )
{
    bool result = false;
    HAL_FLASH_Unlock();
    if (_validate_operation (size, address)) {
        for (uint32_t i = 0; i < size; i++) {
            if (eeprom_write_var(address++, (uint16_t) data[i]) == HAL_FLASH_ERROR_NONE) {
                result = true;
            }
            else {
                result = false;
                break;
            }
        }
    }
    HAL_FLASH_Lock();
    return result;
}

bool hal_eeprom_read( uint8_t* data, uint16_t size, uint16_t address)
{
    bool result = false;
    HAL_FLASH_Unlock();
    if (_validate_operation (size, address)) {
        for (uint32_t i = 0; i < size; i++) {
            uint16_t value = 0;
            if (eeprom_read_var(address++, &value) == 0) {
                data[i] = (uint8_t) value;
                result = true;
            }
            else {
                result = false;
                break;
            }
        }
    }
    HAL_FLASH_Lock();
    return result;
}

void hal_eeprom_flush()
{
}
