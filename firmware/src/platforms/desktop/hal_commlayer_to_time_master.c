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

#ifdef USE_TIME_MASTER

#include "hal_commlayer_to_time_master.h"
#include <zepto_mem_mngmt_hal_spec.h>
#include <simpleiot_hal/hal_commlayer.h>
#include <simpleiot_hal/hal_waiting.h>
#include <stdio.h>

extern uint16_t DEVICE_SELF_ID;

#define MAX_PACKET_SIZE 80


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef _MSC_VER

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define CLOSE_SOCKET( x ) closesocket( x )

#else // _MSC_VER

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // for close() for socket
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#define CLOSE_SOCKET( x ) close( x )

#endif // _MSC_VER

#ifdef _MSC_VER
SOCKET sock_with_time_master;
SOCKET sock_accepted_with_time_master;
#else
int sock_with_time_master;
int sock_accepted_with_time_master;
#endif
struct sockaddr_in sa_self_time_master, sa_other_time_master;
const char* inet_addr_as_string_time_master = "127.0.0.1";
//uint16_t self_port_num_time_master = 7654;
//uint16_t other_port_num_time_master = 7667;
uint16_t self_port_num_time_master = 7668;
uint16_t other_port_num_time_master = 7694;

uint16_t buffer_in_pos_time_master;



bool communication_preinitialize_with_time_master()
{
#ifdef _MSC_VER
	// do Windows magic
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		ZEPTO_DEBUG_PRINTF_2("WSAStartup failed with error: %d\n", iResult);
		return false;
	}
	return true;
#else
	return true;
#endif
}

bool _communication_initialize_with_time_master()
{
	//Zero out socket address
	memset(&sa_self_time_master, 0, sizeof sa_self_time_master);
	memset(&sa_other_time_master, 0, sizeof sa_other_time_master);

	//create an internet, datagram, socket using UDP
	sock_with_time_master = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sock_with_time_master) /* if socket failed to initialize, exit */
	{
		ZEPTO_DEBUG_PRINTF_1("Error Creating Socket\n");
		return false;
	}

	//The address is ipv4
	sa_other_time_master.sin_family = AF_INET;
	sa_self_time_master.sin_family = AF_INET;

	//ip_v4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value
	sa_self_time_master.sin_addr.s_addr = inet_addr( inet_addr_as_string_time_master );
	sa_other_time_master.sin_addr.s_addr = inet_addr( inet_addr_as_string_time_master );

	//sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654
	sa_self_time_master.sin_port = htons( self_port_num_time_master );
	sa_other_time_master.sin_port = htons( other_port_num_time_master );

	if (-1 == connect(sock_with_time_master, (struct sockaddr *)&sa_other_time_master, sizeof(sa_other_time_master)))
		{
		  perror("connect failed");
			CLOSE_SOCKET(sock_with_time_master);
		  return false;
		}
	return true;
}


void _communication_terminate_with_time_master()
{
	CLOSE_SOCKET(sock_with_time_master);
}

uint8_t internal_send_packet_to_time_master( CONST uint8_t* data_buff, uint16_t data_sz, int type, int _sock, struct sockaddr* _sa_other )
{
	ZEPTO_DEBUG_PRINTF_1( "send_message() called...\n" );

	ZEPTO_DEBUG_ASSERT( data_buff != NULL );
	uint8_t base_buff[MAX_PACKET_SIZE + 5];
	base_buff[0] = (uint8_t)(data_sz+5);
	base_buff[1] = (data_sz+5) >> 8;

	int bytes_sent = sendto(_sock, (char*)base_buff, 2, 0, _sa_other, sizeof (struct sockaddr) );

	if (bytes_sent < 0)
	{
#ifdef _MSC_VER
		int error = WSAGetLastError();
		ZEPTO_DEBUG_PRINTF_2( "Error %d sending packet\n", error );
#else
		ZEPTO_DEBUG_PRINTF_2("Error sending packet: %s\n", strerror(errno));
#endif
		return COMMLAYER_RET_FAILED;
	}

	ZEPTO_DEBUG_ASSERT( data_sz <= MAX_PACKET_SIZE );
	base_buff[0] = (uint8_t)DEVICE_SELF_ID;
	base_buff[1] = DEVICE_SELF_ID >> 8;
	base_buff[2] = type;
	base_buff[3] = (uint8_t)data_sz;
	base_buff[4] = data_sz >> 8;
	ZEPTO_MEMCPY( base_buff + 5, data_buff, data_sz );

	bytes_sent = sendto(_sock, (char*)base_buff, 5 + data_sz, 0, _sa_other, sizeof (struct sockaddr) );

	if (bytes_sent < 0) 
	{
#ifdef _MSC_VER
		int error = WSAGetLastError();
		ZEPTO_DEBUG_PRINTF_2( "Error %d sending packet\n", error );
#else
		ZEPTO_DEBUG_PRINTF_2("Error sending packet: %s\n", strerror(errno));
#endif
		return COMMLAYER_RET_FAILED;
	}

	ZEPTO_DEBUG_PRINTF_3( "message sent for registration; type = %d, size = %d\n", type, data_sz );

	return COMMLAYER_RET_OK;
}


uint8_t try_get_packet_from_time_master( uint8_t* buff, uint16_t sz, int _sock, struct sockaddr* _sa_other )
{
	socklen_t fromlen = sizeof(struct sockaddr_in);
	int recsize = recvfrom(_sock, (char *)(buff + buffer_in_pos_time_master), sz - buffer_in_pos_time_master, 0, _sa_other, &fromlen);
	if (recsize < 0)
	{
#ifdef _MSC_VER
		int error = WSAGetLastError();
		if ( error == WSAEWOULDBLOCK )
#else
		int error = errno;
		if ( error == EAGAIN || error == EWOULDBLOCK )
#endif
		{
			return COMMLAYER_RET_PENDING;
		}
		else
		{
			ZEPTO_DEBUG_PRINTF_2( "unexpected error %d received while getting message\n", error );
			return COMMLAYER_RET_FAILED;
		}
	}
	else
	{
		buffer_in_pos_time_master += recsize;
		if ( buffer_in_pos_time_master < sz )
		{
			return COMMLAYER_RET_PENDING;
		}
		return COMMLAYER_RET_OK;
	}

}

uint8_t try_get_packet_size_from_time_master( uint8_t* buff, int _sock, struct sockaddr* _sa_other )
{
	socklen_t fromlen = sizeof(struct sockaddr_in);
	int recsize = recvfrom(_sock, (char *)(buff + buffer_in_pos_time_master), 2 - buffer_in_pos_time_master, 0, _sa_other, &fromlen);
	if (recsize < 0)
	{
#ifdef _MSC_VER
		int error = WSAGetLastError();
		if ( error == WSAEWOULDBLOCK )
#else
		int error = errno;
		if ( error == EAGAIN || error == EWOULDBLOCK )
#endif
		{
			return COMMLAYER_RET_PENDING;
		}
		else
		{
			ZEPTO_DEBUG_PRINTF_2( "unexpected error %d received while getting message\n", error );
			return COMMLAYER_RET_FAILED;
		}
	}
	else
	{
		buffer_in_pos_time_master += recsize;
		if ( buffer_in_pos_time_master < 2 )
		{
			return COMMLAYER_RET_PENDING;
		}
		return COMMLAYER_RET_OK;
	}

}

uint8_t internal_get_packet_bytes_from_time_master( uint8_t* type, uint8_t* buff, uint16_t* packet_data_sz, uint16_t packet_sz_max, int _sock, struct sockaddr* _sa_other )
{
	uint8_t buff_base[2];

	buffer_in_pos_time_master = 0;
	uint8_t ret;

	do //TODO: add delays or some waiting
	{
		ret = try_get_packet_size_from_time_master( buff_base, _sock, _sa_other );
	}
	while ( ret == COMMLAYER_RET_PENDING );
	if ( ret != COMMLAYER_RET_OK )
		return HAL_GET_PACKET_BYTES_FAILED;
	uint16_t sz = buff_base[1]; sz <<= 8; sz += buff_base[0];
	ZEPTO_DEBUG_ASSERT( sz <= packet_sz_max );
	ZEPTO_DEBUG_ASSERT( sz >= 5 );

	buffer_in_pos_time_master = 0;
	do //TODO: add delays or some waiting
	{
		ret = try_get_packet_from_time_master( buff, sz, _sock, _sa_other );
	}
	while ( ret == COMMLAYER_RET_PENDING );

	if ( ret != COMMLAYER_RET_OK )
		return HAL_GET_PACKET_BYTES_FAILED;
	
	uint16_t dev_id = buff[1];
	dev_id = (dev_id << 8 ) | buff[0];
	*type = buff[2];
	*packet_data_sz = buff[4];
	*packet_data_sz = (*packet_data_sz << 8) | buff[3];
	ZEPTO_DEBUG_ASSERT( dev_id == DEVICE_SELF_ID );
	ZEPTO_DEBUG_ASSERT( *packet_data_sz == sz - 5 );

	ZEPTO_MEMMOV( buff, buff + 5, *packet_data_sz );

	return ret == COMMLAYER_RET_OK ? HAL_GET_PACKET_BYTES_DONE : HAL_GET_PACKET_BYTES_FAILED;
}




bool communication_initialize_with_time_master()
{
	return communication_preinitialize_with_time_master() && _communication_initialize_with_time_master();
}

void communication_terminate_with_time_master()
{
	_communication_terminate_with_time_master();
}


// record types:
#define TIME_RECORD_REGISTER_INCOMING_PACKET 0
#define TIME_RECORD_REGISTER_OUTGOING_PACKET 1
#define TIME_RECORD_REGISTER_RAND_VAL_REQUEST_32 2
#define TIME_RECORD_REGISTER_TIME_VALUE 3
#define TIME_RECORD_REGISTER_WAIT_RET_VALUE 4

#ifdef USE_TIME_MASTER_REGISTER

void register_packet_with_time_master( uint8_t ret_code, uint8_t* packet_buff, uint16_t packet_sz, bool incoming )
{
	uint8_t ret;
	uint8_t type_out = incoming ? TIME_RECORD_REGISTER_INCOMING_PACKET : TIME_RECORD_REGISTER_OUTGOING_PACKET;
	ZEPTO_DEBUG_ASSERT( packet_buff != NULL );
	ZEPTO_DEBUG_ASSERT( packet_sz <= MAX_PACKET_SIZE );
	uint8_t buff[MAX_PACKET_SIZE+1];
	buff[0] = ret_code;
	ZEPTO_MEMCPY( buff+1, packet_buff, packet_sz );
	ret = internal_send_packet_to_time_master( buff, packet_sz+1, type_out, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint8_t buff_base[5];
	uint16_t packet_data_sz;
	uint8_t type;
	ret = internal_get_packet_bytes_from_time_master( &type, buff_base, &packet_data_sz, 5, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	ZEPTO_DEBUG_ASSERT( type == type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == 0 );
}


void register_incoming_packet( uint8_t ret_code, MEMORY_HANDLE mem_h )
{
	uint16_t packet_sz = memory_object_get_response_size( mem_h );
	uint8_t* packet_buff = memory_object_get_response_ptr( mem_h );
	register_packet_with_time_master( ret_code, packet_buff, packet_sz, true );
}

void register_outgoing_packet( uint8_t ret_code, MEMORY_HANDLE mem_h )
{
	uint16_t packet_sz = memory_object_get_request_size( mem_h );
	uint8_t* packet_buff = memory_object_get_request_ptr( mem_h );
	register_packet_with_time_master( ret_code, packet_buff, packet_sz, false );
}

void register_wait_request_ret_val( uint8_t ret_val )
{
	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_WAIT_RET_VALUE;
	uint16_t packet_sz = 1;
	uint8_t* buff = &ret_val;
	ZEPTO_DEBUG_ASSERT( buff != NULL );
	ret = internal_send_packet_to_time_master( buff, packet_sz, type_out, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint8_t buff_base[5+1];
	uint16_t packet_data_sz;
	uint8_t type;
	ret = internal_get_packet_bytes_from_time_master( &type, buff_base, &packet_data_sz, 5+1, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	ZEPTO_DEBUG_ASSERT( type == type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == 1 );
	ZEPTO_DEBUG_ASSERT( buff_base[0] == ret_val );
}

void register_time_val( uint8_t point_id, const sa_time_val* in, sa_time_val* out )
{
	uint8_t buff_out[sizeof(sa_time_val) + 1];
	buff_out[0] = point_id;
	buff_out[1] = (uint8_t)(in->low_t);
	buff_out[2] = (uint8_t)(in->low_t>>8);
	buff_out[3] = (uint8_t)(in->high_t);
	buff_out[4] = (uint8_t)(in->high_t>>8);
	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_TIME_VALUE;
	uint16_t packet_sz = sizeof(sa_time_val) + 1;
	ZEPTO_DEBUG_ASSERT( in != NULL );
	ret = internal_send_packet_to_time_master( buff_out, packet_sz, type_out, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

	uint8_t buff_base[5 + sizeof(sa_time_val) + 1];
	uint16_t packet_data_sz;
	uint8_t type;
	ret = internal_get_packet_bytes_from_time_master( &type, buff_base, &packet_data_sz, 5 + sizeof(sa_time_val)+1, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	ZEPTO_DEBUG_ASSERT( type == type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == sizeof(sa_time_val) + 1 );

	ZEPTO_DEBUG_ASSERT( buff_base[0] == point_id );
	out->low_t = buff_base[2];
	out->low_t = (out->low_t<<8) | buff_base[1];
	out->high_t = buff_base[4];
	out->high_t = (out->high_t<<8) | buff_base[3];
}


#else // USE_TIME_MASTER_REGISTER


void request_packet_from_time_master( uint8_t* ret_code, MEMORY_HANDLE mem_h, bool incoming )
{
	uint8_t ret;
	uint8_t type_out = incoming ? TIME_RECORD_REGISTER_INCOMING_PACKET : TIME_RECORD_REGISTER_OUTGOING_PACKET;

	uint8_t buff_base[5+MAX_PACKET_SIZE+1];
	uint16_t packet_data_sz;
	uint8_t type;
	ret = internal_get_packet_bytes_from_time_master( &type, buff_base, &packet_data_sz, 5+MAX_PACKET_SIZE, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	ZEPTO_DEBUG_ASSERT( type == type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz > 1 );

	zepto_parser_free_memory( mem_h );
	zepto_write_block( mem_h, buff_base+1, packet_data_sz - 1 );
	*ret_code = buff_base[0];
}

void request_incoming_packet( uint8_t* ret_code, MEMORY_HANDLE mem_h )
{
	request_packet_from_time_master( ret_code, mem_h, true );
}

void request_outgoing_packet( uint8_t* ret_code, MEMORY_HANDLE mem_h )
{
	request_packet_from_time_master( ret_code, mem_h, false );
}

uint8_t request_wait_request_ret_val()
{
	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_WAIT_RET_VALUE;

	uint8_t buff_base[6];
	uint16_t packet_data_sz;
	uint8_t type;
	ret = internal_get_packet_bytes_from_time_master( &type, buff_base, &packet_data_sz, 6, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	ZEPTO_DEBUG_ASSERT( type == type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == 1 );
	return buff_base[0];
}

void request_time_val( uint8_t point_id, sa_time_val* tv )
{
	uint8_t ret;
	uint8_t type_out = TIME_RECORD_REGISTER_TIME_VALUE;

	uint8_t buff_base[5 + sizeof(sa_time_val) + 1];
	uint16_t packet_data_sz;
	uint8_t type;
	ret = internal_get_packet_bytes_from_time_master( &type, buff_base, &packet_data_sz, 5 + sizeof(sa_time_val) + 1, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
	ZEPTO_DEBUG_ASSERT( ret == HAL_GET_PACKET_BYTES_DONE );
	ZEPTO_DEBUG_ASSERT( type == type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == sizeof(sa_time_val) + 1 );

	ZEPTO_DEBUG_ASSERT( point_id == buff_base[0] );
	tv->low_t = buff_base[2];
	tv->low_t = (tv->low_t<<8) | buff_base[1];
	tv->high_t = buff_base[4];
	tv->high_t = (tv->high_t<<8) | buff_base[3];
}

#endif // USE_TIME_MASTER_REGISTER


#if 0
uint8_t wait_for_communication_event( unsigned int timeout )
{
	ZEPTO_DEBUG_PRINTF_1( "wait_for_communication_event()... " );
    fd_set rfds;
    struct timeval tv;
    int retval;
	int fd_cnt;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);

    FD_SET(sock_with_time_master, &rfds);
#ifdef MESH_TEST
#ifdef SA_RETRANSMITTER
    FD_SET(sock2, &rfds);
	fd_cnt = (int)(sock_with_time_master > sock2 ? sock_with_time_master + 1 : sock2 + 1);
#else
	fd_cnt = (int)(sock_with_time_master + 1);
#endif
#else
	fd_cnt = (int)(sock_with_time_master + 1);
#endif

    /* Wait */
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = ((long)timeout % 1000) * 1000;

    retval = select(fd_cnt, &rfds, NULL, NULL, &tv);
    /* Don't rely on the value of tv now! */

    if (retval == -1)
	{
#ifdef _MSC_VER
		int error = WSAGetLastError();
//		if ( error == WSAEWOULDBLOCK )
		ZEPTO_DEBUG_PRINTF_2( "error %d\n", error );
#else
        perror("select()");
//		int error = errno;
//		if ( error == EAGAIN || error == EWOULDBLOCK )
#endif
        return COMMLAYER_RET_TIMEOUT;
		ZEPTO_DEBUG_ASSERT(0);
		ZEPTO_DEBUG_PRINTF_1( "COMMLAYER_RET_FAILED\n" );
		return COMMLAYER_RET_FAILED;
	}
    else if (retval)
	{
		ZEPTO_DEBUG_PRINTF_1( "COMMLAYER_RET_FROM_DEV\n" );
		return COMMLAYER_RET_FROM_DEV;
	}
    else
	{
		ZEPTO_DEBUG_PRINTF_1( "COMMLAYER_RET_TIMEOUT\n" );
        return COMMLAYER_RET_TIMEOUT;
	}
}

uint8_t hal_wait_for( waiting_for* wf )
{
	unsigned int timeout = wf->wait_time.high_t;
	timeout <<= 16;
	timeout += wf->wait_time.low_t;
	uint8_t ret_code;
	ZEPTO_DEBUG_ASSERT( wf->wait_legs == 0 ); // not implemented
	ZEPTO_DEBUG_ASSERT( wf->wait_i2c == 0 ); // not implemented

	ret_code = wait_for_communication_event( timeout );
	switch ( ret_code )
	{
		case COMMLAYER_RET_FROM_DEV: return WAIT_RESULTED_IN_PACKET; break;
		case COMMLAYER_RET_TIMEOUT: return WAIT_RESULTED_IN_TIMEOUT; break;
		case COMMLAYER_RET_FAILED: return WAIT_RESULTED_IN_FAILURE; break;
		default: return WAIT_RESULTED_IN_FAILURE;
	}
}
#endif // 0

#endif // USE_TIME_MASTER