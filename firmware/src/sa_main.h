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

#if !defined __SA_MAIN_H__
#define __SA_MAIN_H__

#include <simpleiot/siot_common.h>
#include <simpleiot/siot_uint48.h>
#include <hal_time_provider.h>
#include <simpleiot_hal/hal_commlayer.h>
#include <simpleiot_hal/hal_waiting.h>
#include <simpleiot/siot_m_protocol.h>
#include <simpleiot/siot_oud_protocol.h>
#include <simpleiot/siot_s_protocol.h>
#include <simpleiot/siot_gd_protocol.h>
#include <simpleiot/siot_cc_protocol.h>
#include "zepto_config.h"

#ifdef __cplusplus
extern "C" {
#endif

bool sa_main_init();
int sa_main_loop();

#ifdef __cplusplus
}
#endif

#endif // __SA_MAIN_H__
