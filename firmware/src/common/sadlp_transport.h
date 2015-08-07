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

#if !defined __SADLP_TRANSPORT_H__
#define __SADLP_TRANSPORT_H__

#include <simpleiot/siot_common.h>

typedef struct _sadlp_transport{

  bool (* init)(void);
  uint8_t (* read)(void);
  uint32_t (* write)(uint8_t byte);
  bool (* available)(void);

} sadlp_transport;

#define SADLP_SERIAL
#define SADLP_SERIAL_BAUDRATE 9600

#ifdef SADLP_SERIAL
extern const sadlp_transport sadlp_serial_transport;
#define DATALINK_TRANSPORT sadlp_serial_transport
#else
#define DATALINK_TRANSPORT sadlp_void_transport
#endif

#endif // __SADLP_TRANSPORT_H__
