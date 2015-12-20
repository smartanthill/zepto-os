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

#include "hal_main.h"
#include "../../sa_main.h"

#ifdef SA_DYN_PAIRING_ENABLED 
#include <stdio.h>
bool load_device_ids()
{
	extern uint8_t* AES_ENCRYPTION_KEY;
	extern uint16_t DEVICE_SELF_ID;

	FILE* fme = fopen( "devid.dat", "rb" );
	if ( fme == 0 )
	{
		ZEPTO_DEBUG_PRINTF_1( "failed to access \'devid.dat\' file. Terminating...\n" );
		return false;
	}
	int8_t buff[ 80 ];
	uint8_t ret = fread( buff, 1, 80, fme );
	fclose( fme );
	if ( ret < 50 )
	{
		ZEPTO_DEBUG_PRINTF_1( "file \'devid.dat\' is incomplete or corrupted. Terminating...\n" );
		return false;
	}

	uint8_t i = 0, pos;
	bool in_progress = false;

	i = 0;
	for ( pos=0; pos<ret; pos++ )
	{
		bool val_updated = true;
		int8_t ch = buff[pos];
		uint8_t val;
		if ( ch >='0' && ch <='9' ) val = ch - '0';
		else if ( ch >='a' && ch <='f' ) val = ch - 'a' + 10;
		else if ( ch >='A' && ch <='F' ) val = ch - 'A' + 10;
		else
			val_updated = false;

		if ( val_updated )
		{
			if ( in_progress )
			{
				if ( i < 16 )
					AES_ENCRYPTION_KEY[i] = (AES_ENCRYPTION_KEY[i] << 4 ) | val;
				else
					DEVICE_SELF_ID = ( DEVICE_SELF_ID << 4 ) | val;
				i++;
				in_progress = false;
			}
			else
			{
				if ( i < 16 )
					AES_ENCRYPTION_KEY[i] = val;
				else
					DEVICE_SELF_ID = val;
				in_progress = true;
			}
		}
		else
		{
			if ( in_progress )
			{
				i++;
				in_progress = false;
			}
		}
	}

	if ( i < 17 )
	{
		ZEPTO_DEBUG_PRINTF_1( "file \'devid.dat\' has invalid or incomplete data, or corrupted. Terminating...\n" );
		return false;
	}

	ZEPTO_DEBUG_PRINTF_2( "Running as: node = %d; key = ", DEVICE_SELF_ID );
	for ( i=0; i<16; i++ )
		ZEPTO_DEBUG_PRINTF_2( "%02x ", AES_ENCRYPTION_KEY[i] );
	ZEPTO_DEBUG_PRINTF_1( "\n\n" );

	return true;
}
#endif


int main(int argc, char *argv[])
{
#ifdef SA_DYN_PAIRING_ENABLED 
	if ( !load_device_ids() )
	{
		ZEPTO_DEBUG_PRINTF_1( "load_device_ids() failed\n" );
		return 0;
	}
#endif // SA_DYN_PAIRING_ENABLED

    if ( !sa_main_init() )
	{
		ZEPTO_DEBUG_PRINTF_1( "sa_main_init() failed\n" );
		return 0;
	}

    return sa_main_loop();
}
