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

#define EEPROM_SIZE 128
static uint8_t EEPROM_IN_RAM [EEPROM_SIZE] = {0};


static inline bool _validate_operation (uint16_t size, uint16_t address)
{
    if ((address+size > EEPROM_SIZE) || (size > EEPROM_SIZE) || (address > EEPROM_SIZE))
        return false;
    else
        return true;
}

bool hal_init_eeprom_access()
{
    uint8_t i = 0;
    for (i = 0; i<EEPROM_SIZE; i++)
    {
        EEPROM_IN_RAM[i] = 0;
    }
    return true;
}

bool hal_eeprom_write( const uint8_t* data, uint16_t size, uint16_t address )
{
    if (_validate_operation (size, address)) {
        for (uint32_t i = 0; i < size; i++) {
            EEPROM_IN_RAM[address++] = data[i];
        }
        return true;
    }
    return false;
}

bool hal_eeprom_read( uint8_t* data, uint16_t size, uint16_t address)
{

    if (_validate_operation (size, address)) {
        for (uint32_t i = 0; i < size; i++) {
            data[i] =  EEPROM_IN_RAM[address];
        }
        return true;
    }
    return false;
}

void hal_eeprom_flush()
{

}
