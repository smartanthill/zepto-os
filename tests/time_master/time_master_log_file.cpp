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
#include <stdio.h>

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
	unsigned int curr_rec;

	int fill_record_header( READ_RECORD_HEAD* record, char* data )
	{
		char* start = data;

		// read time_id
		while ( data[0] == ' ' || data[0] == '\t' ) data++;
		uint32_t high_id = strtoul( data, &data, 16 );
		while ( data[0] == ' ' || data[0] == '\t' ) data++;
		uint32_t low_id = strtoul( data, &data, 16 );
		record->time_id = high_id;
		record->time_id = ( record->time_id << 32 ) + low_id;
		while ( data[0] == ' ' || data[0] == ':' || data[0] == '\t' ) data++;

		// 'dev: '
		if ( !( data[0] == 'd' && data[1] == 'e' && data[2] == 'v' && data[3] == ':' ) )
			return 0;
		data += 4;
		while ( data[0] == ' ' || data[0] == '\t' ) data++;

		// device_id
		record->device_id = strtoul( data, &data, 16 );
		while ( data[0] == ',' || data[0] == ' ' || data[0] == '\t' ) data++;

		// 'type: '
		if ( !( data[0] == 't' && data[1] == 'y' && data[2] == 'p' && data[3] == 'e' && data[4] == ':' ) )
			return 0;
		data += 5;
		while ( data[0] == ' ' || data[0] == '\t' ) data++;

		// record type:
		record->record_type = strtoul( data, &data, 16 );
		while ( data[0] == ',' || data[0] == ' ' || data[0] == '\t' ) data++;

		// 'sz: '
		if ( !( data[0] == 's' && data[1] == 'z' && data[2] == ':' ) )
			return 0;
		data += 3;
		while ( data[0] == ' ' || data[0] == '\t' ) data++;

		// data size:
		record->data_sz = strtoul( data, &data, 16 );
		while ( data[0] == ',' || data[0] == ' ' || data[0] == '\t' ) data++;

		// 'data: '
		if ( !( data[0] == 'd' && data[1] == 'a' && data[2] == 't' && data[3] == 'a' && data[4] == ':' ) )
			return 0;
		data += 5;
		while ( data[0] == ' ' || data[0] == '\t' ) data++;
		
		return data - start;
	}

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

	bool read_next_record_internal( char* data, int* size, int max_size )
	{
		if ( flog == NULL )
		{
			return false;
		}

		bool end_found = false;
		int jump_back = 0;
		int res;
		int record_size = 0;
		int i;
		do
		{
			res = fread( data + record_size, 1, 128, flog );
			for ( i=0; i<res; i++ )
				if ( data[record_size + i] == '\n' )
				{
					end_found = true;
					record_size += i;
					jump_back = - res + i + 1;
					fseek( flog, jump_back, SEEK_CUR );
					break;
				}
			if ( end_found )
				break;
			if ( res == 0 )
				break;
			record_size += res;
			if ( record_size + 128 > max_size )
				return false; // something went wrong
		}
		while (!end_found && record_size <= max_size - 128);

		*size = record_size;
		return true;
	}

public:
	LogFile()
	{
		flog = NULL;
		curr_rec = 0;
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
		curr_rec = 0;
		return flog != NULL;
	}
	bool add_abstract_data_record( time_id_type timestamp, int dev_id, int type, unsigned char* data, int size )
	{
		ZEPTO_DEBUG_ASSERT( size < ( MAX_FORMATTING_BUFFER_SIZE >> 2 ) );
		ZEPTO_DEBUG_ASSERT( type == TIME_RECORD_REGISTER_INCOMING_PACKET || type == TIME_RECORD_REGISTER_OUTGOING_PACKET || type == TIME_RECORD_REGISTER_EEPROM_STATE || TIME_RECORD_REGISTER_INCOMING_PACKET_AT_COMM_STACK );
		int sz;
		int i;
		sz = sprintf( formatting_buffer, "%08x %08x: dev: %04x, type: %02x, sz: %04x, data: ", (uint32_t)(timestamp>>32), (uint32_t)timestamp, dev_id, type, size );
		for ( i=0; i<size; i++ )
			sz += sprintf( formatting_buffer + sz, "%02x ", data[i] );
		sz += sprintf( formatting_buffer + sz, "\n" );
		return add_record( formatting_buffer, sz );
	}
	bool add_rand_value_request_32_record( time_id_type timestamp, int dev_id, uint32_t rand_val )
	{
		int sz = sprintf( formatting_buffer, "%08x %08x: dev: %04x, type: %02x, sz: %04x, data: %02x %02x %02x %02x\n", (uint32_t)(timestamp>>32), (uint32_t)timestamp, dev_id, TIME_RECORD_REGISTER_RAND_VAL_REQUEST_32, 4, (uint8_t)rand_val, (uint8_t)(rand_val>>8), (uint8_t)(rand_val>>16), (uint8_t)(rand_val>>24) );
		return add_record( formatting_buffer, sz );
	}
	bool add_time_record( time_id_type timestamp, int dev_id, int point_id, uint32_t time_returned )
	{
		int sz = sprintf( formatting_buffer, "%08x %08x: dev: %04x, type: %02x, sz: %04x, data: %02x %02x %02x %02x %02x\n", (uint32_t)(timestamp>>32), (uint32_t)timestamp, dev_id, TIME_RECORD_REGISTER_TIME_VALUE, 5, point_id, (uint8_t)time_returned, (uint8_t)(time_returned>>8), (uint8_t)(time_returned>>16), (uint8_t)(time_returned>>24) );
		return add_record( formatting_buffer, sz );
	}
	bool add_waitingfor_ret_record( time_id_type timestamp, int dev_id, unsigned char* data, int size )
	{
//		int sz = sprintf( formatting_buffer, "%08x %08x: dev: %04x, type: %02x, sz: %04x, data: %02x\n", (uint32_t)(timestamp>>32), (uint32_t)timestamp, dev_id, TIME_RECORD_REGISTER_WAIT_RET_VALUE, 1, ret_val );
		int sz;
		int i;
		sz = sprintf( formatting_buffer, "%08x %08x: dev: %04x, type: %02x, sz: %04x, data: ", (uint32_t)(timestamp>>32), (uint32_t)timestamp, dev_id, TIME_RECORD_REGISTER_WAIT_RET_VALUE, size );
		for ( i=0; i<size; i++ )
			sz += sprintf( formatting_buffer + sz, "%02x ", data[i] );
		sz += sprintf( formatting_buffer + sz, "\n" );
		return add_record( formatting_buffer, sz );
	}
	bool add_dev_disconnect_record( time_id_type timestamp, int dev_id, uint8_t ret_val )
	{
		int sz = sprintf( formatting_buffer, "%08x %08x: dev: %04x, type: %02x, sz: %04x, data: %02x\n", (uint32_t)(timestamp>>32), (uint32_t)timestamp, dev_id, TIME_RECORD_REGISTER_WAIT_RET_VALUE, 1, ret_val );
		return add_record( formatting_buffer, sz );
	}

	bool read_next_record( READ_RECORD_HEAD* record, uint8_t* data, int data_max_size, unsigned int stop_pos = (unsigned int)(-1) )
	{
		int input_sz;
		if ( !read_next_record_internal( formatting_buffer, &input_sz, MAX_FORMATTING_BUFFER_SIZE ) )
			return false;

		ZEPTO_DEBUG_ASSERT( input_sz < MAX_FORMATTING_BUFFER_SIZE );

		if ( curr_rec == stop_pos )
			getchar();

		formatting_buffer[input_sz] = 0;
		ZEPTO_DEBUG_PRINTF_3( "Record read %d: \"%s\"\n\n", curr_rec, formatting_buffer );
		curr_rec++;

		int header_size = fill_record_header( record, (char*)formatting_buffer );
		if ( header_size == 0 )
			return false;

		if ( record->data_sz )
		{
			ZEPTO_DEBUG_ASSERT( record->data_sz <= data_max_size );
			char* start = formatting_buffer + header_size;
			char* prev_start;
			int i;
			for ( i=0; i<record->data_sz; i++ )
			{
				while ( start[0] == ' ' || start[0] == '\t' ) start++;
				prev_start = start;
				unsigned int val = strtoul( start, &start, 16 );
				if ( val > 255 ) // bad format
					return false;
				data[i] = (uint8_t)val;
				if ( prev_start == start ) // nothing numerical
					return false;
			}
		}

		return true;
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
	return logfile.add_abstract_data_record( timestamp, dev_id, type, data, size );
}
bool add_rand_value_request_32_record( time_id_type timestamp, int dev_id, uint32_t rand_val )
{
	return logfile.add_rand_value_request_32_record( timestamp, dev_id, rand_val );
}
bool add_time_record( time_id_type timestamp, int dev_id, int point_id, uint32_t time_returned )
{
	return logfile.add_time_record( timestamp, dev_id, point_id, time_returned );
}
bool add_waitingfor_ret_record( time_id_type timestamp, int dev_id, unsigned char* data, int size )
{
	return logfile.add_waitingfor_ret_record( timestamp, dev_id, data, size );
}
bool read_next_record( READ_RECORD_HEAD* record, uint8_t* data, int data_max_size )
{
	return logfile.read_next_record( record, data, data_max_size );
}
bool read_next_record( READ_RECORD_HEAD* record, uint8_t* data, int data_max_size, unsigned int stop_pos )
{
	return logfile.read_next_record( record, data, data_max_size, stop_pos );
}
bool add_eeprom_ini_packet_record( time_id_type timestamp, int dev_id, int type, unsigned char* data, int size )
{
	return logfile.add_abstract_data_record( timestamp, dev_id, type, data, size );
}
bool add_dev_disconnect_record( time_id_type timestamp, int dev_id, uint8_t ret_val )
{
	return logfile.add_dev_disconnect_record( timestamp, dev_id, ret_val );
}


#include <vector>
using namespace std;

typedef vector<REPLAY_REQUEST> replay_requests;

class ReplayRequestCollection
{
private:
	replay_requests r_requests;

public:
	bool add_request( int conn_id, int device_id, int record_type, int data_sz, const uint8_t* data )
	{
		REPLAY_REQUEST request;
		request.conn_id = conn_id;
		request.device_id = device_id;
		request.record_type = record_type;
		request.data_sz = data_sz;
		if (data_sz)
		{
			request.data = new uint8_t [data_sz];
			memcpy( request.data, data, data_sz );
		}
		else
			request.data = NULL;

		unsigned int i;
		for ( i=0; i<r_requests.size(); i++ )
			if ( r_requests[i].device_id == device_id || r_requests[i].conn_id == conn_id )
				return false;

		r_requests.push_back( request );
		return true;
	}

	REPLAY_REQUEST* get_request_of_device( int device_id )
	{
		unsigned int i;
		for ( i=0; i<r_requests.size(); i++ )
			if ( r_requests[i].device_id == device_id )
				return &(r_requests[i]);
		return NULL;
	}

	bool remove_request_of_device( int device_id )
	{
		unsigned int i;
		for ( i=0; i<r_requests.size(); i++ )
			if ( r_requests[i].device_id == device_id )
			{
				if ( r_requests[i].data != NULL )
					delete [] r_requests[i].data;
				r_requests.erase( r_requests.begin() + i );
			}
		return NULL;
	}
};

static ReplayRequestCollection replay_requests_coll;

bool add_request( int conn_id, int device_id, int record_type, int data_sz, const uint8_t* data )
{
	return replay_requests_coll.add_request( conn_id, device_id, record_type, data_sz, data );
}
REPLAY_REQUEST* get_request_of_device( int device_id )
{
	return replay_requests_coll.get_request_of_device( device_id );
}
bool remove_request_of_device( int device_id )
{
	return replay_requests_coll.remove_request_of_device( device_id );
}
