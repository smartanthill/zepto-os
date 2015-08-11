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

#include "ping.h"
#include <simpleiot/siot_bodypart_list_common.h>
#include <simpleiot_hal/hal_waiting.h>


uint8_t ping_plugin_handler_init( const void* plugin_config, void* plugin_persistent_state )
{
	return PLUGIN_OK;
}

uint8_t ping_plugin_exec_init( const void* plugin_config, void* plugin_state )
{
	return PLUGIN_OK;
}

uint8_t ping_plugin_handler( const void* plugin_config, void* plugin_persistent_state, void* plugin_state, parser_obj* command, MEMORY_HANDLE reply, waiting_for* wf, uint8_t first_byte )
{
	zepto_write_uint8( reply, 1 ); // answer with "1", we are on-line!
	return PLUGIN_OK;
}
