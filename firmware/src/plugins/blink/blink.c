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

#include "blink.h"
#include <hal_time_provider.h>
#include "../../common/hal_gpio.h"

#define PLUGIN_OK 0

uint8_t blink_plugin_handler_init( const void* plugin_config, void* plugin_state )
{
    blink_plugin_config* pc = (blink_plugin_config*)plugin_config;
    sa_hal_gpio_mode(pc->pin_led, HAL_GPIO_TYPE_OUTPUT);
	return PLUGIN_OK;
}

uint8_t blink_plugin_handler( const void* plugin_config, void* plugin_state, parser_obj* command, MEMORY_HANDLE reply/*, WaitingFor* waiting_for*/, uint8_t first_byte )
{
    blink_plugin_config* pc = (blink_plugin_config*)plugin_config;
    uint8_t i = 0;
    uint16_t delay_ms = zepto_parse_encoded_uint16( command );
    uint8_t total_blinks = zepto_parse_encoded_uint8( command );
    for (i = 0; i < total_blinks; i++)
    {
        sa_hal_gpio_write(pc->pin_led, HAL_GPIO_VALUE_HIGH);
        sa_time_delay_ms(delay_ms);
        sa_hal_gpio_write(pc->pin_led, HAL_GPIO_VALUE_LOW);
        sa_time_delay_ms(delay_ms);
    }
	zepto_write_uint8( reply, i ); // answer with "1", all done!
	return PLUGIN_OK;
}
