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


#if !defined __SA_PING_PLUGIN_H__
#define __SA_PING_PLUGIN_H__

#include <simpleiot/siot_common.h>
#include <simpleiot/siot_data_types.h>
#include <simpleiot_hal/hal_waiting.h>

typedef struct _ping_plugin_config
{
	uint8_t dummy_byte;
} ping_plugin_config;

typedef struct _ping_plugin_persistent_state
{
	uint8_t dummy_byte;
} ping_plugin_persistent_state;

typedef struct _ping_plugin_state
{
	uint8_t dummy_byte;
} ping_plugin_state;

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ping_plugin_handler_init( const void* plugin_config, void* plugin_state );
uint8_t ping_plugin_exec_init( const void* plugin_config, void* plugin_state );
uint8_t ping_plugin_handler( const void* plugin_config, void* plugin_persistent_state, void* plugin_state, parser_obj* command, MEMORY_HANDLE reply, waiting_for* wf, uint8_t first_byte );

#ifdef __cplusplus
}
#endif

#endif // __SA_PING_PLUGIN_H__
