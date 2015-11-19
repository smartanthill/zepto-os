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

#if !defined __TIME_LOG_FILE_H__
#define __TIME_LOG_FILE_H__

#include "time_master_common.h"


// RECORDS AS STORED IN LOG FILE: 
// | timestamp (8 bytes) | device_id (2 bytes) | record_type (1 byte) | data_size (2 bytes) | data (variable size) |
// rcords are stored in text form and are '\n' terminated
// 'device_id' is as reported by a device (typically, mesh node_id for devices (with Root == 0), and negative values for testing units such as AIRs
// 'record_type' is one of TIME_RECORD_XXX and depends on a particular request from a calling device 
// 'data' depends on a record type and is what has been received from or sent to the device (for instance, a copy of a packet, a random value, etc)
// in present embodiment all numerical values are stored 

// record types:
#define TIME_RECORD_REGISTER_INCOMING_PACKET 0
#define TIME_RECORD_REGISTER_OUTGOING_PACKET 1
#define TIME_RECORD_REGISTER_RAND_VAL_REQUEST_32 2
#define TIME_RECORD_REGISTER_TIME_VALUE 3
#define TIME_RECORD_REGISTER_WAIT_RET_VALUE 4
#define TIME_RECORD_REGISTER_EEPROM_STATE 5
#define TIME_RECORD_REGISTER_INCOMING_PACKET_AT_COMM_STACK 6

// TIME ID calls
void get_time_id( time_id_type* time_id );
inline uint32_t time_id_to_uint32_time( time_id_type timestamp )
{
	uint32_t ret = (uint32_t)( (timestamp >> 8) / 1000 );
	return ret;
}

typedef struct _read_record_head
{
	time_id_type time_id;
	int device_id;
	int record_type;
	int data_sz;
} READ_RECORD_HEAD;

// LOGGING calls
bool init_access_for_logging( const char* path = NULL );
bool init_access_for_replay( const char* path = NULL );

bool add_in_out_packet_record( time_id_type timestamp, int dev_id, int type, unsigned char* data, int size );
bool add_rand_value_request_32_record( time_id_type timestamp, int dev_id, uint32_t rand_val );
bool add_time_record( time_id_type timestamp, int dev_id, int point_id, uint32_t time_returned );
bool add_waitingfor_ret_record( time_id_type timestamp, int dev_id, uint8_t ret_val );
bool add_eeprom_ini_packet_record( time_id_type timestamp, int dev_id, int type, unsigned char* data, int size );

bool read_next_record( READ_RECORD_HEAD* record, uint8_t* data, int data_max_size );
bool read_next_record( READ_RECORD_HEAD* record, uint8_t* data, int data_max_size, unsigned int stop_pos );

/*
class LogFile
{
private:
	enum {MAX_FORMATTING_BUFFER_SIZE = 4096};
	FILE* flog;
	bool for_logging;
	char formatting_buffer[MAX_FORMATTING_BUFFER_SIZE];

	bool add_record( const char* data, int size );
	bool read_next_record( unsigned char* data, int* size);

public:
	LogFile()	{flog = NULL;}
	bool init_access_for_logging( const char* path = NULL );
	bool init_access_for_replay( const char* path = NULL );
	bool add_in_out_packet_record( time_id_type timestamp, int dev_id, int type, unsigned char* data, int size );
	bool add_rand_value_request_32_record( time_id_type timestamp, int dev_id, uint32_t rand_val );
};
*/

typedef struct _REPLAY_REQUEST
{
	int conn_id;
	int device_id;
	int record_type;
	int data_sz;
	uint8_t* data;
} REPLAY_REQUEST;

bool add_request( int conn_id, int device_id, int record_type, int data_sz, const uint8_t* data );
REPLAY_REQUEST* get_request_of_device( int device_id );
bool remove_request_of_device( int device_id );

#endif