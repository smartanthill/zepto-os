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

#include "pin_digital_write.h"
#include "../../common/hal_gpio.h"

#define PLUGIN_OK 0

uint8_t pin_digital_write_plugin_handler_init( const void* plugin_config, void* plugin_state )
{
    pin_digital_write_plugin_config* pc = (pin_digital_write_plugin_config*)plugin_config;
    sa_hal_gpio_mode(pc->pin_num, HAL_GPIO_TYPE_OUTPUT);
	return PLUGIN_OK;
}

uint8_t pin_digital_write_plugin_handler( const void* plugin_config, void* plugin_state, parser_obj* command, MEMORY_HANDLE reply/*, WaitingFor* waiting_for*/, uint8_t first_byte )
{
    pin_digital_write_plugin_config* pc = (pin_digital_write_plugin_config*)plugin_config;
    uint8_t level = zepto_parse_uint8( command );
    sa_hal_gpio_write(pc->pin_num, level);
    zepto_write_uint8( reply, level);
    return PLUGIN_OK;
}

