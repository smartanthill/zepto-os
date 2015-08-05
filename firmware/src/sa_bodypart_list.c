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

/*******************************************************************************
THIS FILE IS MANUALLY OR AUTOMATICALLY GENERATED BASED ON DESIRED PLUGIN LIST
*******************************************************************************/


#include <simpleiot/siot_bodypart_list.h>

// include declarations of respective plugins
#include "plugins/smart_echo/smart_echo.h"
#include "plugins/ping/ping.h"

ping_plugin_config ping_plugin_config_0;
smart_echo_plugin_config smart_echo_plugin_config_1={ .dummy=1 };

ping_plugin_state ping_plugin_state_0;
smart_echo_plugin_state smart_echo_plugin_state_1;

const uint8_t SA_BODYPARTS_MAX ZEPTO_PROG_CONSTANT_LOCATION = 2;
const bodypart_item bodyparts[ 2 ] ZEPTO_PROG_CONSTANT_LOCATION =
{
    { ping_plugin_handler_init, ping_plugin_handler, &ping_plugin_config_0, &ping_plugin_state_0 },
{ smart_echo_plugin_handler_init, smart_echo_plugin_handler, &smart_echo_plugin_config_1, &smart_echo_plugin_state_1 }
};
