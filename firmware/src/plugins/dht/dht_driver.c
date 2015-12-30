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


#include "dht_driver.h"
#include "../../hal_common/hal_time_provider.h"
#include "../../common/hapi_gpio.h"

dht_operation_result_t dht_get_data(hapi_gpio_t* pin_dht, dht_data* data)
{
    hapi_gpio_set_mode (pin_dht, HAPI_GPIO_TYPE_OUTPUT);
    hapi_gpio_write (pin_dht, 0);
    sa_time_delay_ms(18);
    hapi_gpio_write (pin_dht, 1);
    sa_time_delay_us(20);
    hapi_gpio_set_mode (pin_dht, HAPI_GPIO_TYPE_INPUT);
    while(!(hapi_gpio_read (pin_dht)));
    sa_time_delay_us(80);
    uint8_t count;
    count = 0;
    while(hapi_gpio_read (pin_dht))
    {
        count ++;
        if (count >200){
            return DHT_RESULT_ERROR;
        }
    }

    uint8_t result = 0;
    uint8_t _data[5] = {0};

    for (uint8_t i=0; i<5; i++)
    {
        result = 0;
        for(uint8_t j=0; j<8; j++)
        {
            while(!(hapi_gpio_read (pin_dht)));
            count = 0;
            while(hapi_gpio_read (pin_dht))
            {
                sa_time_delay_us(1);
                count++;
                if (count >150)
                {
                    return DHT_RESULT_ERROR;
                }
            }
            if (count>4)
            {
                result = (result<<1)+1;
            }
            else
            {
                result = (result<<1);
            }
        }
        _data[i] = result;
    }

    uint16_t  crc1 = (_data[0] + _data[1] + _data[2] + _data[3]);
    crc1 &= ~(1<<8);
    if (_data[4] != crc1)
    {
        return DHT_RESULT_ERROR;
    }

    data->temperature = (_data[2] <<8)+ _data[3];
    data->humidity = (_data[0]<<8) +_data[1];

    return DHT_RESULT_OK;
}
