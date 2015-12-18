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

#ifdef USED_AS_MASTER
#error not applicable - do not include
#else
#ifdef USED_AS_RETRANSMITTER
#define BUS_COUNT 2
#else
#define BUS_COUNT 1
#endif
#endif

uint8_t hal_get_bus_count() // bus IDs are then expected in the range 0..(ret_val-1)
{
	return BUS_COUNT;
}

