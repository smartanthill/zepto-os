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


#if !defined __SA_DHT_DRIVER_H__
#define __SA_DHT_DRIVER_H__

#include <simpleiot/siot_common.h>
#include "../../common/hapi_gpio.h"

typedef struct _dht_data
{
    uint16_t temperature;
    uint16_t humidity;
} dht_data;

typedef enum {
  DHT_RESULT_ERROR,
  DHT_RESULT_OK
} dht_operation_result_t;

#ifdef __cplusplus
extern "C" {
#endif

dht_operation_result_t dht_get_data(hapi_gpio_t* pin_dht, dht_data* data);

#ifdef __cplusplus
}
#endif

#endif // __SA_DHT_DRIVER_H__
