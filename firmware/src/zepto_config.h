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



#if !defined __ZEPTO_CONFIG_H__
#define __ZEPTO_CONFIG_H__

// NOTE: this approach is subject to change; basically there are two ways how key may become available: as a result of pairing or during programming
// in the first caes we just prepare a location where the key will be stored;
// in the second case the key is a part of code
// THIS APPROACH IS SUBJECT TO CHANGE
#ifdef SA_DYN_PAIRING_ENABLED

#define DECLARE_AES_ENCRYPTION_KEY \
uint8_t AES_ENCRYPTION_KEY_STORAGE[16]; \
const uint8_t* AES_ENCRYPTION_KEY = AES_ENCRYPTION_KEY_STORAGE;

#define DECLARE_DEVICE_ID \
uint16_t DEVICE_SELF_ID = 0xFFFF; /* definitely, not 0; must be loaded later */

#else // SA_DYN_PAIRING_ENABLED

#define DECLARE_AES_ENCRYPTION_KEY \
const uint8_t AES_ENCRYPTION_KEY_STORAGE[16] ZEPTO_PROG_CONSTANT_LOCATION = \
{ \
	0x10, \
	0x11, \
	0x12, \
	0x13, \
	0x14, \
	0x15, \
	0x16, \
	0x17, \
	0x18, \
	0x19, \
	0x1a, \
	0x1b, \
	0x1c, \
	0x1d, \
	0x1e, \
	0x1f, \
}; \
const uint8_t* AES_ENCRYPTION_KEY = AES_ENCRYPTION_KEY_STORAGE;

#define DECLARE_DEVICE_ID \
uint16_t DEVICE_SELF_ID = 1; /* definitely, not 0 */

#endif // SA_DYN_PAIRING_ENABLED


#endif // __ZEPTO_CONFIG_H__