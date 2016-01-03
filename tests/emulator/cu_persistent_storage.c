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

#include "cu_persistent_storage.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#if defined _MSC_VER || defined __MINGW32__
#include <windows.h>
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif


// Interface is implemented based on file IO
// File has a standard name

//FILE* f = NULL;
int efile = -1;
#if defined _MSC_VER || defined __MINGW32__
HANDLE hfile = INVALID_HANDLE_VALUE;
#endif

uint8_t hal_init_eeprom_access( char* path )
{
	FILE* ftest;
	if ( path == NULL ) path = "sa_client_dummy_db.dat";

	bool exists = false;
	ftest = fopen( path, "r" );
	if ( ftest )
	{
		exists = true;
		fclose( ftest );
	}

	efile = open( path, O_RDWR | O_CREAT | O_BINARY, S_IWRITE | S_IREAD );
#if defined _MSC_VER || defined __MINGW32__
	hfile = (HANDLE) _get_osfhandle (efile);
	if ( hfile == INVALID_HANDLE_VALUE )
		return HAL_PS_INIT_FAILED;
#endif

	if (exists && efile != -1)
		return HAL_PS_INIT_OK;
	else if (efile != -1)
		return HAL_PS_INIT_OK_NEEDS_INITIALIZATION;
	else
		return HAL_PS_INIT_FAILED;
}

bool hal_eeprom_write( const uint8_t* data, uint16_t size, uint32_t address )
{
	int res;
	res = lseek( efile, address, SEEK_SET);
	if ( res == -1 )
		return false;
	res = write( efile, data, size );
	if ( res !=  size )
		return false;
	return true;
}

bool hal_eeprom_read( uint8_t* data, uint16_t size, uint32_t address)
{
	int res;
	res = lseek( efile, address, SEEK_SET);
	if ( res == -1 )
		return false;
	res = read( efile, data, size );
	if ( res !=  size )
		return false;
	return true;
}

void hal_eeprom_flush()
{
#if defined _MSC_VER || defined __MINGW32__
	FlushFileBuffers(hfile);
#else
	fsync( efile );
#endif
}


// quick and dirty solution for a while...

bool write_field( uint16_t device_id, uint8_t field_id, uint16_t data_sz, uint8_t* data )
{
	ZEPTO_DEBUG_ASSERT( device_id < MAX_DEVICE_COUNT );
	ZEPTO_DEBUG_ASSERT( field_id < MAX_FILELDS_PER_RECORD );
	ZEPTO_DEBUG_ASSERT( data_sz < MAX_FIELD_SIZE - 2 );
	uint32_t base_offset = ((uint32_t)device_id) * MAX_FIELD_SIZE * MAX_FILELDS_PER_RECORD;
	uint32_t field_offset = base_offset + ((uint32_t)field_id) * MAX_FIELD_SIZE;

	uint8_t szbts[2];
	szbts[0] = (uint8_t)data_sz;
	szbts[1] = (uint8_t)(data_sz>>8);

	bool ret;
	ret = hal_eeprom_write( szbts, 2, field_offset );
	ret = ret && hal_eeprom_write( data, data_sz, field_offset + 2 );
	return ret;
}

bool read_field( uint16_t device_id, uint8_t field_id, uint16_t* data_sz, uint8_t* data )
{
	ZEPTO_DEBUG_ASSERT( device_id < MAX_DEVICE_COUNT );
	ZEPTO_DEBUG_ASSERT( field_id < MAX_FILELDS_PER_RECORD );
	uint32_t base_offset = ((uint32_t)device_id) * MAX_FIELD_SIZE * MAX_FILELDS_PER_RECORD;
	uint32_t field_offset = base_offset + ((uint32_t)field_id) * MAX_FIELD_SIZE;

	bool ret;
	uint8_t szbts[2];
	ret = hal_eeprom_read( szbts, 2, field_offset );
	if ( !ret ) return false;

	*data_sz = szbts[1]; *data_sz <<= 8; *data_sz += szbts[0];
	ZEPTO_DEBUG_ASSERT( *data_sz < MAX_FIELD_SIZE - 2 );
	return hal_eeprom_read( data, *data_sz, field_offset + 2 );
}

bool init_default_storage()
{
	uint32_t full_size = MAX_FILELDS_PER_RECORD * MAX_FIELD_SIZE * MAX_DEVICE_COUNT;
	uint8_t block[ MAX_FIELD_SIZE ];
	uint16_t i, j;
	ZEPTO_MEMSET( block, 0, MAX_FIELD_SIZE );
	uint32_t offset = 0;
	for ( i=0; i<MAX_DEVICE_COUNT; i++ )
		for ( j=0; j<MAX_FILELDS_PER_RECORD; j++ )
		{
			if ( !hal_eeprom_write( block, MAX_FIELD_SIZE, offset ) )
				return false;
			offset += MAX_FIELD_SIZE;
		}
	return true;
}
