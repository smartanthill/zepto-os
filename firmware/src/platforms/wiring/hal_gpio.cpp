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
#include "../../common/hal_gpio.h"

void sa_hal_gpio_mode (uint8_t pin, uint8_t type)
{
    pinMode(pin, type);
}

uint8_t sa_hal_gpio_read (uint8_t pin)
{
    return digitalRead(pin);
}

void sa_hal_gpio_write (uint8_t pin, uint8_t value)
{
    digitalWrite(pin, value);
}
