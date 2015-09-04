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
#include "stm32l1xx.h"

#define EEPROM_SIZE                         ((uint8_t)512)
#define EEPROM_START_ADDRESS                ((uint32_t)0x08080000)

static inline bool _validate_operation (uint16_t size, uint16_t address)
{
    if ((address+size > EEPROM_SIZE) || (size > EEPROM_SIZE) || (address > EEPROM_SIZE))
        return false;
    else
        return true;
}

bool hal_init_eeprom_access()
{
    if (HAL_FLASHEx_DATAEEPROM_Unlock() == HAL_OK && HAL_FLASHEx_DATAEEPROM_Lock() == HAL_OK)
        return true;
    return false;
}

bool hal_eeprom_write( const uint8_t* data, uint16_t size, uint16_t address )
{
    bool result = false;
    if (HAL_FLASHEx_DATAEEPROM_Unlock() == HAL_OK)
    {
        if (_validate_operation (size, address))
        {
            for (uint16_t i = 0; i < size; i++)
            {
                if (HAL_FLASHEx_DATAEEPROM_Program (TYPEPROGRAMDATA_BYTE, EEPROM_START_ADDRESS + address, data[i]) == HAL_OK)
                {
                    result = true;
                    address++;
                }
                else
                {
                    result = false;
                    break;
                }
            }
        }
        else
        {
            result = false;
        }
    }
    else
    {
        result = false;
    }

    HAL_FLASHEx_DATAEEPROM_Lock();
    return result;
}

bool hal_eeprom_read( uint8_t* data, uint16_t size, uint16_t address)
{
    if (_validate_operation (size, address))
    {
        for (uint16_t i = 0; i < size; i++)
        {
            uint32_t* var_ptr = (uint32_t*)(EEPROM_START_ADDRESS + address);
            data[i] = (uint8_t)(*var_ptr);
            address++;
        }

        return true;
    }

    return false;
}

void hal_eeprom_flush()
{
}
