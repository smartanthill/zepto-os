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
/*
extern uint16_t DEVICE_SELF_ID;

#define MAX_PACKET_SIZE 80
*/

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

//uint8_t internal_send_packet_to_time_master( const uint8_t* data_buff, uint16_t data_sz, int type, int _sock, struct sockaddr* _sa_other )
uint8_t internal_send_debug_packet( const uint8_t* data_buff, uint16_t data_sz, int _sock, struct sockaddr* _sa_other )
{
	ZEPTO_DEBUG_PRINTF_1( "send_message() called...\n" );

	uint8_t base_buff[2];
	base_buff[0] = (uint8_t)(data_sz);
	base_buff[1] = (data_sz) >> 8;

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

	bytes_sent = sendto(_sock, (char*)data_buff, data_sz, 0, _sa_other, sizeof (struct sockaddr) );

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

	ZEPTO_DEBUG_PRINTF_2( "message sent for registration; size = %d\n", data_sz );

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

//uint8_t internal_get_packet_bytes_from_time_master( uint8_t* type, uint8_t* buff, uint16_t* packet_data_sz, uint16_t packet_sz_max, int _sock, struct sockaddr* _sa_other )
uint8_t internal_get_debug_packet( uint8_t* buff, uint16_t* packet_data_sz, uint16_t packet_sz_max, int _sock, struct sockaddr* _sa_other )
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

	*packet_data_sz = sz;
	
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

uint8_t get_debug_packet( uint8_t* buff, uint16_t* packet_data_sz, uint16_t packet_sz_max )
{
	return internal_get_debug_packet( buff, packet_data_sz, packet_sz_max, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
}

uint8_t send_debug_packet( const uint8_t* data_buff, uint16_t data_sz )
{
	return internal_send_debug_packet( data_buff, data_sz, sock_with_time_master, (struct sockaddr *)(&sa_other_time_master) );
}


#endif // USE_TIME_MASTER