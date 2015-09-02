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

#include "stm32f4xx.h"
#include "stm32f4xx_eeprom.h"

static uint16_t _temp_var = 0;
extern uint16_t address_table[EEPROM_SIZE];

static uint32_t eeprom_format(void);
static uint16_t eeprom_find_page(uint8_t operation_type);
static uint32_t eeprom_verify_and_write(uint16_t virt_address, uint16_t data);
static uint32_t eeprom_page_relocation(uint16_t virt_address, uint16_t data);

uint32_t eeprom_init(void) {
    uint16_t var_index = 0;
    uint32_t eeprom_state = 0;
    uint32_t read_state = 0;
    int16_t index = -1;
    uint32_t flash_status;

    uint16_t page0_state = (*(__IO uint16_t*) FLASH_MEMORY_PAGE0_BASE_ADDRESS);
    uint16_t page1_state = (*(__IO uint16_t*) FLASH_MEMORY_PAGE1_BASE_ADDRESS);

    switch (page0_state) {
    case FLASH_MEMORY_ERASED:
        if (page1_state == FLASH_MEMORY_VALID_PAGE) {
            FLASH_Erase_Sector(FLASH_MEMORY_PAGE0_ID , FLASH_MEMORY_VOLTAGE_RANGE);
            flash_status = HAL_FLASH_GetError();
            if (flash_status != HAL_FLASH_ERROR_NONE) {

                return flash_status;
            }
        } else if (page1_state == FLASH_MEMORY_RECEIVE_DATA) {
            FLASH_Erase_Sector(FLASH_MEMORY_PAGE0_ID , FLASH_MEMORY_VOLTAGE_RANGE);
            flash_status = HAL_FLASH_GetError();
            if (flash_status != HAL_FLASH_ERROR_NONE) {
                return flash_status;
            }
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                    FLASH_MEMORY_PAGE1_BASE_ADDRESS, FLASH_MEMORY_VALID_PAGE) != HAL_OK) {
                return HAL_FLASH_ERROR_OPERATION;
            }
        } else {
            flash_status = eeprom_format();
            if (flash_status != HAL_FLASH_ERROR_NONE) {
                return flash_status;
            }
        }
        break;

    case FLASH_MEMORY_RECEIVE_DATA:
        if (page1_state == FLASH_MEMORY_VALID_PAGE) {
            for (var_index = 0; var_index < EEPROM_SIZE; var_index++) {
                if ((*(__IO uint16_t*) (FLASH_MEMORY_PAGE0_BASE_ADDRESS + 6)) == address_table[var_index]) {
                    index = var_index;
                }
                if (var_index != index) {
                    read_state = eeprom_read_var(address_table[var_index], &_temp_var);
                    if (read_state != 0x1) {
                        eeprom_state = eeprom_verify_and_write(address_table[var_index], _temp_var);
                        if (eeprom_state != HAL_FLASH_ERROR_NONE) {
                            return eeprom_state;
                        }
                    }
                }
            }

            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_MEMORY_PAGE0_BASE_ADDRESS, FLASH_MEMORY_VALID_PAGE) != HAL_OK) {
                return HAL_FLASH_ERROR_OPERATION;
            }

            FLASH_Erase_Sector(FLASH_MEMORY_PAGE1_ID, FLASH_MEMORY_VOLTAGE_RANGE);
            flash_status = HAL_FLASH_GetError();
            if (flash_status != HAL_FLASH_ERROR_NONE) {
                return flash_status;
            }
        } else if (page1_state == FLASH_MEMORY_ERASED) {
            FLASH_Erase_Sector(FLASH_MEMORY_PAGE1_ID, FLASH_MEMORY_VOLTAGE_RANGE);
            flash_status = HAL_FLASH_GetError();
            if (flash_status != HAL_FLASH_ERROR_NONE) {
                return flash_status;
            }
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_MEMORY_PAGE0_BASE_ADDRESS, FLASH_MEMORY_VALID_PAGE) != HAL_OK) {
                return HAL_FLASH_ERROR_OPERATION;
            }
        } else {
            flash_status = eeprom_format();
            if (flash_status != HAL_FLASH_ERROR_NONE) {
                return flash_status;
            }
        }
        break;

    case FLASH_MEMORY_VALID_PAGE:
        if (page1_state == FLASH_MEMORY_VALID_PAGE) {
            flash_status = eeprom_format();
            if (flash_status != HAL_FLASH_ERROR_NONE) {
                return flash_status;
            }
        } else if (page1_state == FLASH_MEMORY_ERASED) {
            FLASH_Erase_Sector(FLASH_MEMORY_PAGE1_ID, FLASH_MEMORY_VOLTAGE_RANGE);
            flash_status = HAL_FLASH_GetError();
            if (flash_status != HAL_FLASH_ERROR_NONE) {
                return flash_status;
            }
        } else {
            for (var_index = 0; var_index < EEPROM_SIZE; var_index++) {
                if ((*(__IO uint16_t*) (FLASH_MEMORY_PAGE1_BASE_ADDRESS + 6)) == address_table[var_index]) {
                    index = var_index;
                }
                if (var_index != index) {
                    read_state = eeprom_read_var(address_table[var_index], &_temp_var);
                    if (read_state != 0x1) {
                        eeprom_state = eeprom_verify_and_write(address_table[var_index], _temp_var);
                        if (eeprom_state != HAL_FLASH_ERROR_NONE) {
                            return eeprom_state;
                        }
                    }
                }
            }
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_MEMORY_PAGE1_BASE_ADDRESS, FLASH_MEMORY_VALID_PAGE) != HAL_OK) {
                return HAL_FLASH_ERROR_OPERATION;
            }
            FLASH_Erase_Sector(FLASH_MEMORY_PAGE0_ID , FLASH_MEMORY_VOLTAGE_RANGE);
            flash_status = HAL_FLASH_GetError();
            if (flash_status != HAL_FLASH_ERROR_NONE) {
                return flash_status;
            }
        }
        break;

    default:
        flash_status = eeprom_format();
        if (flash_status != HAL_FLASH_ERROR_NONE) {
            return flash_status;
        }
        break;
    }
    return HAL_FLASH_ERROR_NONE;
}

uint16_t eeprom_read_var(uint16_t virt_address, uint16_t* data) {


    uint32_t valid_page = eeprom_find_page(READ_FROM_VALID_PAGE);

    if (valid_page == FLASH_MEMORY_NO_VALID_PAGE) {
        return FLASH_MEMORY_NO_VALID_PAGE;
    }

    uint32_t page_start_address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(valid_page * FLASH_MEMORY_PAGE_SIZE ));
    uint32_t current_address = (uint32_t)((EEPROM_START_ADDRESS - 2) + (uint32_t)((1 + valid_page) * FLASH_MEMORY_PAGE_SIZE ));

    uint16_t var_address = 0x0;
    uint16_t read_state = 1;
    while (current_address > (page_start_address + 2)) {
        var_address = (*(__IO uint16_t*) current_address);
        if (var_address == virt_address) {
            *data = (*(__IO uint16_t*) (current_address - 2));
            read_state = 0;
            break;
        } else {
            current_address = current_address - 4;
        }
    }
    return read_state;
}

uint32_t eeprom_write_var(uint16_t virt_address, uint16_t data) {

    uint32_t result = eeprom_verify_and_write(virt_address, data);
    if (result == FLASH_MEMORY_PAGE_FULL) {
        result = eeprom_page_relocation(virt_address, data);
    }
    return result;
}

static uint32_t eeprom_format(void) {

    FLASH_Erase_Sector(FLASH_MEMORY_PAGE0_ID , FLASH_MEMORY_VOLTAGE_RANGE);
    uint32_t flash_status = HAL_FLASH_GetError();

    if (flash_status != HAL_FLASH_ERROR_NONE) {
        return flash_status;
    }

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_MEMORY_PAGE0_BASE_ADDRESS, FLASH_MEMORY_VALID_PAGE) != HAL_OK) {
        return HAL_FLASH_ERROR_OPERATION;
    }

    FLASH_Erase_Sector(FLASH_MEMORY_PAGE1_ID, FLASH_MEMORY_VOLTAGE_RANGE);
    flash_status = HAL_FLASH_GetError();
    return flash_status;
}

static uint16_t eeprom_find_page(uint8_t operation_type) {

    uint16_t page0_state = (*(__IO uint16_t*) FLASH_MEMORY_PAGE0_BASE_ADDRESS);
    uint16_t page1_state = (*(__IO uint16_t*) FLASH_MEMORY_PAGE1_BASE_ADDRESS);

    switch (operation_type) {
    case WRITE_IN_VALID_PAGE:
        if (page1_state == FLASH_MEMORY_VALID_PAGE) {
            if (page0_state == FLASH_MEMORY_RECEIVE_DATA) {
                return FLASH_MEMORY_PAGE0;
            } else {

                return FLASH_MEMORY_PAGE1;
            }
        } else if (page0_state == FLASH_MEMORY_VALID_PAGE) {
            if (page1_state == FLASH_MEMORY_RECEIVE_DATA) {
                return FLASH_MEMORY_PAGE1;
            } else {
                return FLASH_MEMORY_PAGE0;
            }
        } else {
            return FLASH_MEMORY_NO_VALID_PAGE;
        }

    case READ_FROM_VALID_PAGE:
        if (page0_state == FLASH_MEMORY_VALID_PAGE) {
            return FLASH_MEMORY_PAGE0;
        } else if (page1_state == FLASH_MEMORY_VALID_PAGE) {
            return FLASH_MEMORY_PAGE1;
        } else {
            return FLASH_MEMORY_NO_VALID_PAGE;
        }

    default:
        return FLASH_MEMORY_PAGE0;
    }
}

static uint32_t eeprom_verify_and_write(uint16_t virt_address, uint16_t data) {

    uint16_t valid_page = eeprom_find_page(WRITE_IN_VALID_PAGE);

    if (valid_page == FLASH_MEMORY_NO_VALID_PAGE) {
        return FLASH_MEMORY_NO_VALID_PAGE;
    }

    uint32_t current_address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(valid_page * FLASH_MEMORY_PAGE_SIZE ));
    uint32_t page_end_adress = (uint32_t)((EEPROM_START_ADDRESS - 2) + (uint32_t)((1 + valid_page) * FLASH_MEMORY_PAGE_SIZE ));

    while (current_address < page_end_adress) {
        if ((*(__IO uint32_t*) current_address) == 0xFFFFFFFF) {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, current_address, data) != HAL_OK) {
                return HAL_FLASH_ERROR_OPERATION;
            }
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, current_address + 2, virt_address) == HAL_OK) {
                return HAL_FLASH_ERROR_NONE;
            } else {
                return HAL_FLASH_ERROR_OPERATION;
            }
        } else {
            current_address = current_address + 4;
        }
    }

    return FLASH_MEMORY_PAGE_FULL;
}

static uint32_t eeprom_page_relocation(uint16_t virt_address, uint16_t data) {

    uint16_t oldpage_id = 0;
    uint32_t newpage_address = EEPROM_START_ADDRESS;
    uint16_t valid_page = eeprom_find_page(READ_FROM_VALID_PAGE);

    if (valid_page == FLASH_MEMORY_PAGE1) {
        newpage_address = FLASH_MEMORY_PAGE0_BASE_ADDRESS;
        oldpage_id = FLASH_MEMORY_PAGE1_ID;
    } else if (valid_page == FLASH_MEMORY_PAGE0) {
        newpage_address = FLASH_MEMORY_PAGE1_BASE_ADDRESS;
        oldpage_id = FLASH_MEMORY_PAGE0_ID ;
    } else {
        return FLASH_MEMORY_NO_VALID_PAGE;
    }
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, newpage_address, FLASH_MEMORY_RECEIVE_DATA) != HAL_OK) {
        return HAL_FLASH_ERROR_OPERATION;
    }
    uint32_t eeprom_state = eeprom_verify_and_write(virt_address, data);
    if (eeprom_state != HAL_FLASH_ERROR_NONE) {
        return eeprom_state;
    }
    uint16_t var_index = 0;
    uint32_t read_state = 0;
    for (var_index = 0; var_index < EEPROM_SIZE; var_index++) {
        if (address_table[var_index] != virt_address) {
            read_state = eeprom_read_var(address_table[var_index], &_temp_var);
            if (read_state != 0x1) {
                eeprom_state = eeprom_verify_and_write(address_table[var_index], _temp_var);
                if (eeprom_state != HAL_FLASH_ERROR_NONE) {
                    return eeprom_state;
                }
            }
        }
    }

    FLASH_Erase_Sector(oldpage_id, FLASH_MEMORY_VOLTAGE_RANGE);
    uint32_t flash_status = HAL_FLASH_GetError();
    if (flash_status != HAL_FLASH_ERROR_NONE) {
        return flash_status;
    }
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, newpage_address, FLASH_MEMORY_VALID_PAGE) != HAL_OK) {
        return HAL_FLASH_ERROR_OPERATION;
    }
    return flash_status;
}

