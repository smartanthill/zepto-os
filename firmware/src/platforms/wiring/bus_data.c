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

#include <simpleiot/siot_bus_data.h>

// NOTE 1: see comments @ impleiot/siot_bus_data.h
// NOTE 2: currently we try to address only a "2+N" model with a RETRANSMITTER between a ROOT and a number of TERMINATING devices so that TERMINATING ones are reachable by a bus other than that for the ROOT

typedef struct _BUS_LIST_ITEM
{
//	uint16_t bus_id;
	uint8_t bus_type;
} BUS_LIST_ITEM;

#ifdef USED_AS_MASTER
#define BUS_LIST_ITEM_COUNT 1
BUS_LIST_ITEM bus_list[ BUS_LIST_ITEM_COUNT ] = {0};
#else
#ifdef USED_AS_RETRANSMITTER
#define BUS_LIST_ITEM_COUNT 2 // WHY? - it's just for immediate testing purposes
BUS_LIST_ITEM bus_list[ BUS_LIST_ITEM_COUNT ] = {{0}, {1}};
#else
#define BUS_LIST_ITEM_COUNT 1
BUS_LIST_ITEM bus_list[ BUS_LIST_ITEM_COUNT ] = {1};
#endif
#endif

uint16_t siot_stats_counters_bus_specific_16__[SIOT_STATS_CTR_BUS_SPECIFIC_16_MAX * BUS_LIST_ITEM_COUNT];
uint16_t* siot_stats_counters_bus_specific_16 = siot_stats_counters_bus_specific_16__;

uint8_t hal_get_bus_count() // bus IDs are then expected in the range 0..(ret_val-1)
{
	return BUS_LIST_ITEM_COUNT;
}


uint8_t hal_get_bus_type_by_bus_id( uint16_t bus_id )
{
/*	uint8_t idx;
	for ( idx=0; idx<BUS_LIST_ITEM_COUNT; idx++ )
		if ( bus_list[idx].bus_id == bus_id )
			return bus_list[idx].bus_type;*/
	if ( bus_id < BUS_LIST_ITEM_COUNT )
		bus_list[bus_id].bus_type;
	return BUS_TYPE_UNDEFINED;
}

uint16_t hal_get_next_bus_of_type(uint8_t bus_type, uint16_t prev_bus_id )
{
	uint16_t idx = prev_bus_id < BUS_LIST_ITEM_COUNT ? prev_bus_id + 1 : 0;
	for ( ; idx<BUS_LIST_ITEM_COUNT; idx++ )
		if ( bus_list[idx].bus_type == bus_type )
			return idx;
	return BUS_ID_UNDEFINED;
}
