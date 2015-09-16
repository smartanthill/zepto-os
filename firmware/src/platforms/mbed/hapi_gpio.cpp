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
#include "../../common/hapi_gpio.h"

#include <gpio_api.h>

void hapi_gpio_init (hapi_gpio_t* pin)
{
    gpio_init((gpio_t*)pin->pin_obj, (PinName)pin->pin_name);
}

void hapi_gpio_set_mode (hapi_gpio_t* pin, hapi_gpio_mode mode)
{
    gpio_dir((gpio_t*)pin->pin_obj,(PinDirection)mode);
    gpio_mode((gpio_t*)pin->pin_obj, PullDefault);
}

uint8_t hapi_gpio_read (hapi_gpio_t* pin)
{
    return gpio_read ((gpio_t*)pin->pin_obj);
}

void hapi_gpio_write (hapi_gpio_t* pin, uint8_t value)
{
    gpio_write((gpio_t*)pin->pin_obj, value);
}
