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

#if !defined __SA_TRANSPORT_XBEE_H__
#define __SA_TRANSPORT_XBEE_H__

#include <simpleiot/siot_common.h>
#include "../../common/hapi_serial.h"

typedef struct _xbee_transport_config
{
    hapi_serial_t* serial_obj;
    uint16_t baudrate;
    uint8_t channel;
    uint16_t pan_id;
    uint8_t power_level;
} xbee_transport_config;

typedef struct _xbee_transport_state
{
    uint8_t dummy;
} xbee_transport_state;

#endif // __SA_TRANSPORT_XBEE_H__
