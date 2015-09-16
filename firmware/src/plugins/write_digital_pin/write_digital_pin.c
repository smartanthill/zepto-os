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


#include "write_digital_pin.h"
#include <simpleiot/siot_bodypart_list_common.h>
#include "../../common/hapi_gpio.h"

uint8_t write_digital_pin_plugin_handler_init( const void* plugin_config, void* plugin_state )
{
	return PLUGIN_OK;
}

uint8_t write_digital_pin_plugin_exec_init( const void* plugin_config, void* plugin_state )
{
    write_digital_pin_plugin_config* pc = (write_digital_pin_plugin_config*)plugin_config;
    hapi_gpio_init(pc->pin);
    hapi_gpio_set_mode(pc->pin, HAPI_GPIO_TYPE_OUTPUT);
    return PLUGIN_OK;
}

uint8_t write_digital_pin_plugin_handler( const void* plugin_config, void* plugin_persistent_state, void* plugin_state, parser_obj* command, MEMORY_HANDLE reply, waiting_for* wf, uint8_t first_byte )
{
    write_digital_pin_plugin_config* pc = (write_digital_pin_plugin_config*)plugin_config;
    uint8_t level = zepto_parse_uint8( command );
    hapi_gpio_write(pc->pin, level);
    zepto_write_uint8(reply, level);
    return PLUGIN_OK;
}
