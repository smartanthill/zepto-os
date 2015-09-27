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

uint8_t internal_send_packet_to_time_master( MEMORY_HANDLE mem_h, int type, int _sock, struct sockaddr* _sa_other )
{
	ZEPTO_DEBUG_PRINTF_1( "send_message() called...\n" );

	uint16_t sz = memory_object_get_request_size( mem_h );
	uint8_t* buff = memory_object_get_request_ptr( mem_h );
	ZEPTO_DEBUG_ASSERT( buff != NULL );
	uint8_t buff_base[7];
	buff_base[0] = (uint8_t)(sz+5);
	buff_base[1] = (sz+5) >> 8;
	buff_base[2] = (uint8_t)DEVICE_SELF_ID;
	buff_base[3] = DEVICE_SELF_ID >> 8;
	buff_base[4] = type;
	buff_base[5] = (uint8_t)sz;
	buff_base[6] = sz >> 8;

	int bytes_sent = sendto(_sock, (char*)buff_base, 7, 0, _sa_other, sizeof (struct sockaddr) );

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

	bytes_sent = sendto(_sock, (char*)buff, sz, 0, _sa_other, sizeof (struct sockaddr) );

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

	ZEPTO_DEBUG_PRINTF_3( "message sent for registration; mem_h = %d, size = %d\n", mem_h, sz );

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

uint8_t internal_get_packet_bytes_from_time_master( uint8_t* buff, uint16_t* packet_sz, int _sock, struct sockaddr* _sa_other )
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

	buffer_in_pos_time_master = 0;
	do //TODO: add delays or some waiting
	{
		ret = try_get_packet_from_time_master( buff, sz, _sock, _sa_other );
	}
	while ( ret == COMMLAYER_RET_PENDING );
	*packet_sz = sz;

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


void register_packet_with_time_master( MEMORY_HANDLE mem_h, bool incoming )
{
	uint8_t buff_base[5];
	uint16_t packet_sz;
	uint8_t ret;
	uint8_t type_out = incoming ? TIME_RECORD_REGISTER_INCOMING_PACKET : TIME_RECORD_REGISTER_OUTGOING_PACKET;
	ret = internal_send_packet_to_time_master( mem_h, type_out, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );
	ret = internal_get_packet_bytes_from_time_master( buff_base, &packet_sz, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );
#ifdef SA_DEBUG
	ZEPTO_DEBUG_ASSERT( packet_sz == 5 );
	uint16_t dev_id = buff_base[1];
	dev_id = (dev_id << 8 ) | buff_base[0];
	uint8_t type = buff_base[2];
	uint16_t packet_data_sz = buff_base[4];
	packet_data_sz = (packet_data_sz << 8) | buff_base[3];
	ZEPTO_DEBUG_ASSERT( dev_id == DEVICE_SELF_ID );
	ZEPTO_DEBUG_ASSERT( type == type_out );
	ZEPTO_DEBUG_ASSERT( packet_data_sz == 0 );
#endif // SA_DEBUG
}


void register_incoming_packet( MEMORY_HANDLE mem_h )
{
	register_packet_with_time_master( mem_h, true );
}

void register_outgoing_packet( MEMORY_HANDLE mem_h )
{
	register_packet_with_time_master( mem_h, false );
}

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