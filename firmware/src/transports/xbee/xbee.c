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
#include "../../hal_common/hal_time_provider.h"
#include "../../common/sa_transport.h"
#include "xbee.h"


static void local_itoa_hex(uint32_t value, uint8_t* buff)
{
	void reverse_string(uint8_t* begin, uint8_t* end)
	{
		uint8_t temp = 0;
		while(end>begin)
			temp=*end, *end--=*begin, *begin++=temp;
	}

	uint8_t num_base[] = "0123456789ABCDEF";
	uint8_t* local_str = buff;
	do *local_str++ = num_base[value%16]; while(value/=16);
	*local_str='\0';
	reverse_string(buff,local_str-1);
}

static uint16_t local_str_len(const uint8_t *str)
{
    uint16_t len = 0;
    while(*(str++)) len++;
    return len;
}

static void local_str_cat(uint8_t *dest, const uint8_t *src)
{
    while (*dest != '\0') dest++;
    do
    {
        *dest++ = *src++;
    }
    while (*src != '\0') ;
}

static void append_xbee_command(uint8_t* commands_buff, uint8_t* command_name, uint32_t value)
{
	local_str_cat (commands_buff, command_name);
	uint8_t value_buff[16] = {0};
	local_itoa_hex(value, value_buff);
	local_str_cat (commands_buff, value_buff);
}

static void xbee_open_at_mode (const void* transport_config)
{
	xbee_transport_config* tc = (xbee_transport_config*) transport_config;
	hapi_serial_write(tc->serial_obj, (uint8_t*)"+++", 3);
}

static void xbee_send_at_command (const void* transport_config, const uint8_t *command, uint8_t length)
{
	xbee_transport_config* tc = (xbee_transport_config*) transport_config;

	hapi_serial_write(tc->serial_obj, (uint8_t*)"AT", 2);
	hapi_serial_write(tc->serial_obj, command, length);
	hapi_serial_write_byte(tc->serial_obj, '\r');
}

static void xbee_close_at_mode (const void* transport_config)
{
	xbee_send_at_command (transport_config, (uint8_t*)"CN", 2); // Set PAN ID
}

static bool xbee_init (const void* transport_config)
{
	xbee_transport_config* tc = (xbee_transport_config*) transport_config;

	hapi_serial_init(tc->serial_obj, tc->baudrate);

	xbee_open_at_mode(transport_config);
	sa_time_delay_ms(1000); // wait OK

	uint8_t command_buffer [128] = {0};
	/* Default values:
	 * Destination Address Low : 0xFFFF (broadcast)
	 * Node Address: FFFF (disable 16bit address)
	 * Mac Mode: 1 (802.15.4 + noACK)
	 * Coordinator enable: 0 (End device)
	 * AES Encryption: 0 (disable)
	*/
	uint8_t default_params[] = "DL FFFF,MY FFFF,MM 1,CE 0,EE 0";
	local_str_cat(command_buffer, default_params);
	local_str_cat(command_buffer, (uint8_t*)",");
	append_xbee_command (command_buffer, (uint8_t*)"CH ", tc->channel);
	local_str_cat(command_buffer, (uint8_t*)",");
	append_xbee_command (command_buffer, (uint8_t*)"ID ", tc->pan_id);
	local_str_cat(command_buffer, (uint8_t*)",");
	append_xbee_command (command_buffer, (uint8_t*)"PL ", tc->power_level);
	xbee_send_at_command(transport_config, command_buffer, local_str_len(command_buffer));

	xbee_close_at_mode(transport_config);

	return true;
}

static void xbee_read (const void* transport_config, uint8_t *buffer, uint16_t length)
{
	xbee_transport_config* tc = (xbee_transport_config*) transport_config;
	hapi_serial_read(tc->serial_obj, buffer, length);
}

static int8_t xbee_read_byte (const void* transport_config)
{
	xbee_transport_config* tc = (xbee_transport_config*) transport_config;
	return hapi_serial_read_byte(tc->serial_obj);
}

static uint16_t xbee_write (const void* transport_config, const uint8_t *buffer, uint16_t length)
{
	xbee_transport_config* tc = (xbee_transport_config*) transport_config;
	return hapi_serial_write(tc->serial_obj, buffer, length);
}

static uint8_t xbee_write_byte (const void* transport_config, uint8_t byte)
{
	xbee_transport_config* tc = (xbee_transport_config*) transport_config;
	return hapi_serial_write_byte (tc->serial_obj, byte);
}

static bool xbee_readable(const void* transport_config)
{
	xbee_transport_config* tc = (xbee_transport_config*) transport_config;
	return hapi_serial_readable (tc->serial_obj);
}

extern const sa_transport xbee_transport =
{
	xbee_init,
	xbee_read,
	xbee_read_byte,
	xbee_write,
	xbee_write_byte,
	xbee_readable
};
