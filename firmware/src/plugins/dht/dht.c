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


#include "dht.h"
#include "dht_driver.h"
#include <simpleiot/siot_bodypart_list_common.h>
#include "../../common/hapi_gpio.h"

uint8_t dht_plugin_handler_init( const void* plugin_config, void* plugin_state )
{
    return PLUGIN_OK;
}

uint8_t dht_plugin_exec_init( const void* plugin_config, void* plugin_state )
{
    dht_plugin_config* pc = (dht_plugin_config*)plugin_config;
    hapi_gpio_init(pc->dht_pin);
    return PLUGIN_OK;
}

uint8_t dht_plugin_handler( const void* plugin_config, void* plugin_persistent_state, void* plugin_state, parser_obj* command, MEMORY_HANDLE reply, waiting_for* wf, uint8_t first_byte )
{
    dht_plugin_config* pc = (dht_plugin_config*)plugin_config;
    dht_data data = {0};
    if (dht_get_data(pc->dht_pin, &data) == DHT_RESULT_OK)
    {
        zepto_write_uint8(reply, (data.temperature>>8));
        return PLUGIN_OK;
    }

    return 0;
}
