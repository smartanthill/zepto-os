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


#if !defined __SA_TRANSPORTS_LIST_H__
#define __SA_TRANSPORTS_LIST_H__

#include "common/sa_transport.h"

typedef struct _transport_item
{
    const sa_transport* t;
    void* t_config;
    void* t_state;
} transport_item;

extern const uint8_t SA_TRANSPORTS_MAX ZEPTO_PROG_CONSTANT_LOCATION;
extern const transport_item transports[];


#endif // __SA_TRANSPORTS_LIST_H__
