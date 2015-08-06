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

#if !defined __HAL_GPIO_H__
#define __HAL_GPIO_H__

#include <simpleiot/siot_common.h>

enum {
  HAL_GPIO_TYPE_INPUT,
  HAL_GPIO_TYPE_OUTPUT,
  HAL_GPIO_TYPE_PULLUP
};

enum {
  HAL_GPIO_VALUE_LOW,
  HAL_GPIO_VALUE_HIGH
};


#ifdef __cplusplus
extern "C" {
#endif

void sa_hal_gpio_mode (uint8_t pin, uint8_t type);
uint8_t sa_hal_gpio_read (uint8_t pin);
void sa_hal_gpio_write (uint8_t pin, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif // __HAL_GPIO_H__