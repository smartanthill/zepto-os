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

#if !defined __SA_TRANSPORT_H__
#define __SA_TRANSPORT_H__

#include <simpleiot/siot_common.h>

typedef struct _sa_transport {

  bool (* init) (const void* transport_config);
  void (* read) (const void* transport_config, uint8_t *buffer, uint16_t length);
  int8_t(* read_byte) (const void* transport_config);
  uint16_t (* write) (const void* transport_config, const uint8_t *buffer, const uint16_t length);
  uint8_t (* write_byte) (const void* transport_config, uint8_t byte);
  bool (* readable) (const void* transport_config);

} sa_transport;

#endif // __SA_TRANSPORT_H__
