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

#include "commstack_commlayer.h"
#include "../../firmware/src/hal/hal_waiting.h"
#include <stdio.h>

#define MAX_PACKET_SIZE 50


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



bool communication_preinitialize()
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





#ifdef _MSC_VER
SOCKET sock_with_cl;
SOCKET sock_with_cl_accepted;
#else
int sock_with_cl;
int sock_with_cl_accepted;
#endif
const char* inet_addr_as_string_with_cl = "127.0.0.1";
struct sockaddr_in sa_self_with_cl, sa_other_with_cl;

uint16_t self_port_num_with_cl = 7665;
uint16_t other_port_num_with_cl = 7655;

uint16_t buffer_in_with_cl_pos;

bool communication_with_comm_layer_initialize()
{
	//Zero out socket address
	memset(&sa_self_with_cl, 0, sizeof sa_self_with_cl);
	memset(&sa_other_with_cl, 0, sizeof sa_other_with_cl);

	//create an internet, datagram, socket using UDP
	sock_with_cl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sock_with_cl) /* if socket failed to initialize, exit */
	{
		ZEPTO_DEBUG_PRINTF_1("Error Creating Socket\n");
		return false;
	}

	//The address is ipv4
	sa_other_with_cl.sin_family = AF_INET;
	sa_self_with_cl.sin_family = AF_INET;

	//ip_v4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value
	sa_self_with_cl.sin_addr.s_addr = inet_addr( inet_addr_as_string_with_cl );
	sa_other_with_cl.sin_addr.s_addr = inet_addr( inet_addr_as_string_with_cl );

	//sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654
	sa_self_with_cl.sin_port = htons( self_port_num_with_cl );
	sa_other_with_cl.sin_port = htons( other_port_num_with_cl );

	if (-1 == bind(sock_with_cl, (struct sockaddr *)&sa_self_with_cl, sizeof(sa_self_with_cl)))
	{
#ifdef _MSC_VER
		int error = WSAGetLastError();
#else
		int error = errno;
#endif
		ZEPTO_DEBUG_PRINTF_2( "bind sock_with_cl failed; error %d\n", error );
		CLOSE_SOCKET(sock_with_cl);
		return false;
	}

	if(-1 == listen(sock_with_cl, 10))
    {
      perror("error listen failed");
      CLOSE_SOCKET(sock_with_cl);
      return false;
    }

	sock_with_cl_accepted = accept(sock_with_cl, NULL, NULL);

      if ( 0 > sock_with_cl_accepted )
      {
        perror("error accept failed");
        CLOSE_SOCKET(sock_with_cl);
        exit(EXIT_FAILURE);
      }

	  sock_with_cl = sock_with_cl_accepted; /*just to keep names*/

#ifdef _MSC_VER
    unsigned long ul = 1;
    ioctlsocket(sock_with_cl, FIONBIO, &ul);
#else
    fcntl(sock_with_cl,F_SETFL,O_NONBLOCK);
#endif

	return true;
}

void communication_with_comm_layer_terminate()
{
	CLOSE_SOCKET(sock_with_cl);
}

bool communication_initialize()
{
	return communication_preinitialize() && communication_with_comm_layer_initialize();
}

void communication_terminate()
{
//	_communication_terminate();
	communication_with_comm_layer_terminate();
}

uint8_t try_get_packet_within_master_loop( uint8_t* buff, uint16_t sz )
{
	socklen_t fromlen = sizeof(sa_other_with_cl);
	int recsize = recvfrom(sock_with_cl, (char *)(buff + buffer_in_with_cl_pos), sz - buffer_in_with_cl_pos, 0, (struct sockaddr *)&sa_other_with_cl, &fromlen);
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
		buffer_in_with_cl_pos += recsize;
		if ( buffer_in_with_cl_pos < sz )
		{
			return COMMLAYER_RET_PENDING;
		}
		return COMMLAYER_RET_OK;
	}

}

uint8_t try_get_packet_size_within_master_loop( uint8_t* buff )
{
	socklen_t fromlen = sizeof(sa_other_with_cl);
	int recsize = recvfrom(sock_with_cl, (char *)(buff + buffer_in_with_cl_pos), 3 - buffer_in_with_cl_pos, 0, (struct sockaddr *)&sa_other_with_cl, &fromlen);
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
		buffer_in_with_cl_pos += recsize;
		if ( buffer_in_with_cl_pos < 3 )
		{
			return COMMLAYER_RET_PENDING;
		}
		return COMMLAYER_RET_OK;
	}

}

uint8_t try_get_message_within_master( MEMORY_HANDLE mem_h )
{
	// do cleanup
	memory_object_response_to_request( mem_h );
	memory_object_response_to_request( mem_h );
	uint8_t* buff = memory_object_append( mem_h, MAX_PACKET_SIZE );

	buffer_in_with_cl_pos = 0;
	uint8_t ret;

	do //TODO: add delays or some waiting
	{
		ret = try_get_packet_size_within_master_loop( buff );
	}
	while ( ret == COMMLAYER_RET_PENDING );
	if ( ret != COMMLAYER_RET_OK )
		return ret;
	uint16_t sz = buff[1]; sz <<= 8; sz += buff[0];
	uint8_t packet_src = buff[2];

	buffer_in_with_cl_pos = 0;
	do //TODO: add delays or some waiting
	{
		ret = try_get_packet_within_master_loop( buff, sz );
	}
	while ( ret == COMMLAYER_RET_PENDING );

	memory_object_response_to_request( mem_h );
	memory_object_cut_and_make_response( mem_h, 0, sz );

	ZEPTO_DEBUG_ASSERT( packet_src == 38 || packet_src == 40 );
	if ( packet_src == 38 )
		return COMMLAYER_RET_OK_AS_CU;
	if ( packet_src == 40 )
		return COMMLAYER_RET_OK_AS_SLAVE;
	return ret;
}

uint8_t send_within_master( MEMORY_HANDLE mem_h, uint8_t destination )
{
	ZEPTO_DEBUG_PRINTF_1( "send_within_master() called...\n" );

	uint16_t sz = memory_object_get_request_size( mem_h );
	memory_object_request_to_response( mem_h );
	ZEPTO_DEBUG_ASSERT( sz == memory_object_get_response_size( mem_h ) );
	ZEPTO_DEBUG_ASSERT( sz != 0 ); // note: any valid message would have to have at least some bytes for headers, etc, so it cannot be empty
	uint8_t* buff = memory_object_prepend( mem_h, 3 );
	ZEPTO_DEBUG_ASSERT( buff != NULL );
	buff[0] = (uint8_t)sz;
	buff[1] = sz >> 8;
	buff[2] = destination;
	int bytes_sent = sendto(sock_with_cl, (char*)buff, sz+3, 0, (struct sockaddr*)&sa_other_with_cl, sizeof sa_other_with_cl);
	// do full cleanup
	memory_object_response_to_request( mem_h );
	memory_object_response_to_request( mem_h );


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
#ifdef _MSC_VER
	ZEPTO_DEBUG_PRINTF_4( "[%d] message sent within master; mem_h = %d, size = %d\n", GetTickCount(), mem_h, sz );
#else
	ZEPTO_DEBUG_PRINTF_3( "[--] message sent within master; mem_h = %d, size = %d\n", mem_h, sz );
#endif
	return COMMLAYER_RET_OK;
}


uint8_t wait_for_communication_event( unsigned int timeout )
{
	// ZEPTO_DEBUG_PRINTF_1( "wait_for_communication_event()\n" );
    fd_set rfds;
    struct timeval tv;
    int retval;
	int fd_cnt;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);

    FD_SET(sock_with_cl, &rfds);
	fd_cnt = (int)(sock_with_cl + 1);

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
		ZEPTO_DEBUG_ASSERT(0);
		return COMMLAYER_RET_FAILED;
	}
    else if (retval)
	{
		return COMMLAYER_RET_FROM_CENTRAL_UNIT;
	}
    else
	{
        return COMMLAYER_RET_TIMEOUT;
	}
}

uint8_t send_message( MEMORY_HANDLE mem_h )
{
	return send_within_master( mem_h, 35 );
}

uint8_t send_to_central_unit( MEMORY_HANDLE mem_h )
{
	return send_within_master( mem_h, 37 );
}

// from comm.stack: 35: intended for slave; 37: intended for central unit
//   to comm.stack: 40: received from slave; 38: received from central unit