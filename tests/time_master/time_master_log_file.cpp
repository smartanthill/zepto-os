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

#include "time_master_common.h"
#include "time_master_log_file.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <windows.h>
#include <io.h>
#endif

#ifdef printf
#undef printf
#endif

#include <chrono>
using namespace std;

class time_id_provider
{
private:
	int inter_tick_counter;
	time_id_type last_tick_value;
	std::chrono::steady_clock clk;
	std::chrono::steady_clock::time_point start;
public:
	time_id_provider()
	{
		start = clk.now();
		inter_tick_counter = 0;
		last_tick_value = 0;
	}
	void get_time_id( time_id_type* time_id )
	{
		std::chrono::steady_clock::time_point tp = clk.now();
		long long cnt = std::chrono::duration_cast<std::chrono::microseconds>(tp - start).count();
		if ( cnt == last_tick_value )
		{
			inter_tick_counter++;
		}
		else
		{
			last_tick_value = cnt;
			inter_tick_counter = 0;
		}

		ZEPTO_DEBUG_ASSERT( ( inter_tick_counter >> 8 ) == 0 );
		*time_id = ( last_tick_value << 8 ) + ( inter_tick_counter & 0xFF );
	}
};

static time_id_provider tid;
void get_time_id( time_id_type* time_id )
{
	return tid.get_time_id( time_id );
}



class LogFile
{
private:
	enum {MAX_FORMATTING_BUFFER_SIZE = 4096};
	FILE* flog;
	bool for_logging;
	char formatting_buffer[MAX_FORMATTING_BUFFER_SIZE];

	bool add_record( const char* data, int size )
	{
		if ( flog == NULL )
		{
			return false;
		}
		int res;
		res = fwrite( data, 1, size, flog );
		if ( res != size )
			return false;
		return true;
	}

	bool read_next_record( unsigned char* data, int* size)
	{
		if ( flog == NULL )
		{
			return false;
		}
		int res;
		res = fread( data, 1, *size, flog );
		if ( res != *size )
			return false;
		return true;
	}

public:
	LogFile()
	{
		flog = NULL;
	}
	bool init_access_for_logging( const char* path = NULL )
	{
		if ( flog != NULL )
		{
			return false;
		}
		if ( path == NULL ) path = "default_log.dat";
		flog = fopen( path, "wb" );
		for_logging = true;
		return flog != NULL;
	}
	bool init_access_for_replay( const char* path = NULL )
	{
		if ( flog != NULL )
		{
			return false;
		}
		if ( path == NULL ) path = "default_log.dat";
		flog = fopen( path, "rb" );
		for_logging = false;
		return flog != NULL;
	}
	bool add_in_out_packet_record( time_id_type timestamp, int dev_id, int type, unsigned char* data, int size )
	{
		ZEPTO_DEBUG_ASSERT( size < ( MAX_FORMATTING_BUFFER_SIZE >> 2 ) );
		ZEPTO_DEBUG_ASSERT( type == TIME_RECORD_REGISTER_INCOMING_PACKET || type == TIME_RECORD_REGISTER_OUTGOING_PACKET );
		int sz;
		int i;
		sz = sprintf( formatting_buffer, "%08x %08x: dev: %04x, type: %d, sz: %d, data: ", timestamp, dev_id, type, size );
		for ( i=0; i<size; i++ )
			sz += sprintf( formatting_buffer + sz, "%02x ", data[i] );
		sz += sprintf( formatting_buffer + sz, "\n" );
		return add_record( formatting_buffer, sz );
	}
	bool add_rand_value_request_32_record( time_id_type timestamp, int dev_id, uint32_t rand_val )
	{
		int sz = sprintf( formatting_buffer, "%08x %08x: dev: %04x, type: %d, sz: %d, data: %08x", timestamp, dev_id, TIME_RECORD_REGISTER_RAND_VAL_REQUEST_32, 4, rand_val );
		return add_record( formatting_buffer, sz );
	}
	bool add_time_record( time_id_type timestamp, int dev_id, uint32_t time_returned )
	{
		int sz = sprintf( formatting_buffer, "%08x %08x: dev: %04x, type: %d, sz: %d, data: %08x", timestamp, dev_id, TIME_RECORD_REGISTER_TIME_VALUE, 4, time_returned );
		return add_record( formatting_buffer, sz );
	}
	bool add_waitingfor_ret_record( time_id_type timestamp, int dev_id, uint8_t ret_val )
	{
		int sz = sprintf( formatting_buffer, "%08x %08x: dev: %04x, type: %d, sz: %d, data: %02x", timestamp, dev_id, TIME_RECORD_REGISTER_WAIT_RET_VALUE, 4, ret_val );
		return add_record( formatting_buffer, sz );
	}
};

static LogFile logfile;

bool init_access_for_logging( const char* path )
{
	return logfile.init_access_for_logging( path );
}
bool init_access_for_replay( const char* path )
{
	return logfile.init_access_for_replay( path );
}
bool add_in_out_packet_record( time_id_type timestamp, int dev_id, int type, unsigned char* data, int size )
{
	return logfile.add_in_out_packet_record( timestamp, dev_id, type, data, size );
}
bool add_rand_value_request_32_record( time_id_type timestamp, int dev_id, uint32_t rand_val )
{
	return logfile.add_rand_value_request_32_record( timestamp, dev_id, rand_val );
}
bool add_time_record( time_id_type timestamp, int dev_id, uint32_t time_returned )
{
	return logfile.add_time_record( timestamp, dev_id, time_returned );
}
bool add_waitingfor_ret_record( time_id_type timestamp, int dev_id, uint8_t ret_val )
{
	return logfile.add_waitingfor_ret_record( timestamp, dev_id, ret_val );
}
