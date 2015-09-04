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

#ifndef __STM32F4XX_EEPROM_H__
#define __STM32F4XX_EEPROM_H__

#include "stm32f4xx_hal_flash.h"

#define EEPROM_SIZE                         ((uint8_t)128)
#define EEPROM_START_ADDRESS                ((uint32_t)0x08020000)

#define FLASH_MEMORY_PAGE_SIZE              (uint32_t)0x20000  // 16 KByte
#define FLASH_MEMORY_VOLTAGE_RANGE          (uint8_t)FLASH_VOLTAGE_RANGE_3

#define FLASH_MEMORY_PAGE0                  ((uint16_t)0x0000)
#define FLASH_MEMORY_PAGE0_BASE_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS))
#define FLASH_MEMORY_PAGE0_ID               FLASH_SECTOR_5

#define FLASH_MEMORY_PAGE1                  ((uint16_t)0x0001)
#define FLASH_MEMORY_PAGE1_BASE_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS + FLASH_MEMORY_PAGE_SIZE))
#define FLASH_MEMORY_PAGE1_ID               FLASH_SECTOR_6


#define FLASH_MEMORY_ERASED                 ((uint16_t)0xFFFF)
#define FLASH_MEMORY_RECEIVE_DATA           ((uint16_t)0xEEEE)
#define FLASH_MEMORY_VALID_PAGE             ((uint16_t)0x0000)
#define FLASH_MEMORY_NO_VALID_PAGE          ((uint16_t)0x00AB)
#define FLASH_MEMORY_PAGE_FULL              ((uint8_t)0x80)

#define READ_FROM_VALID_PAGE                ((uint8_t)0x00)
#define WRITE_IN_VALID_PAGE                 ((uint8_t)0x01)

#ifdef __cplusplus
extern "C" {
#endif

uint32_t eeprom_init(void);
uint16_t eeprom_read_var(uint16_t virt_address, uint16_t* data);
uint32_t eeprom_write_var(uint16_t virt_address, uint16_t data);

#ifdef __cplusplus
}
#endif


#endif /* __STM32F4XX_EEPROM_H__ */
