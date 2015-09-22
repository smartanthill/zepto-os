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

#include "air_commlayer.h"
#include <stdio.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined _MSC_VER || defined __MINGW32__

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

// END OF CCOMMON STAFF


typedef struct _DEVICE_CONNNECTION
{
#if defined _MSC_VER || defined __MINGW32__
	SOCKET sock_with_cl_accepted;
#else
	int sock_with_cl_accepted;
#endif
//	struct sockaddr_in sa_self_with_cl;
	struct sockaddr_in sa_other_with_cl;
	bool connected;
} DEVICE_CONNNECTION;

int sock_with_cl = -1;


const char* inet_addr_as_string_with_cl = "127.0.0.1";
uint16_t self_port_num = 7654;

uint16_t buffer_in_with_cl_pos;

#define MAX_PACKET_SIZE 0x1000
#define MEX_DEV_CNT	 10

DEVICE_CONNNECTION devices[ MEX_DEV_CNT ];
int dev_count = 0;




bool communication_preinitialize()
{
	ZEPTO_MEMSET( devices, 0, MEX_DEV_CNT * sizeof( DEVICE_CONNNECTION ) );

#if defined _MSC_VER || defined __MINGW32__
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


bool communication_with_comm_layer_initialize()
{
	//create an internet, datagram, socket using UDP
	struct sockaddr_in sa_self_with_cl;
	sock_with_cl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sock_with_cl) /* if socket failed to initialize, exit */
	{
		ZEPTO_DEBUG_PRINTF_1("Error Creating Socket\n");
		return false;
	}

	//The address is ipv4
//	conn->sa_other_with_cl.sin_family = AF_INET;
	sa_self_with_cl.sin_family = AF_INET;

	//ip_v4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value
	sa_self_with_cl.sin_addr.s_addr = inet_addr( inet_addr_as_string_with_cl );
//	conn->sa_other_with_cl.sin_addr.s_addr = inet_addr( inet_addr_as_string_with_cl );

	//sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654
	sa_self_with_cl.sin_port = htons( self_port_num );
//	conn->sa_other_with_cl.sin_port = htons( conn->other_port_num );

	if (-1 == bind( sock_with_cl, (struct sockaddr *)&sa_self_with_cl, sizeof(sa_self_with_cl)))
	{
#if defined _MSC_VER || defined __MINGW32__
		int error = WSAGetLastError();
#else
		int error = errno;
#endif
		ZEPTO_DEBUG_PRINTF_2( "bind sock_with_cl failed; error %d\n", error );
		CLOSE_SOCKET(sock_with_cl);
		return false;
	}

#if defined _MSC_VER || defined __MINGW32__
    unsigned long ul = 1;
    ioctlsocket(sock_with_cl, FIONBIO, &ul);
#else
    fcntl(conn->sock_with_cl,F_SETFL,O_NONBLOCK);
#endif

	if(-1 == listen(sock_with_cl, 10))
    {
      perror("error listen failed");
      CLOSE_SOCKET(sock_with_cl);
      return false;
    }

	struct sockaddr_in sock_in;
	socklen_t sock_len = sizeof(sock_in);
	if (getsockname(sock_with_cl, (struct sockaddr *)&sock_in, &sock_len) == -1)
	    perror("getsockname");
	else
		ZEPTO_DEBUG_PRINTF_2( "socket: started on port %d\n", ntohs(sock_in.sin_port) );

#if 0

	conn->sock_with_cl_accepted = accept(conn->sock_with_cl, NULL, NULL);

      if ( 0 > conn->sock_with_cl_accepted )
      {
        perror("error accept failed");
        CLOSE_SOCKET(conn->sock_with_cl);
        exit(EXIT_FAILURE);
      }

	  conn->sock_with_cl = conn->sock_with_cl_accepted; /*just to keep names*/
#endif // 0

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

uint8_t try_get_packet_within_master_loop( uint8_t* buff, uint16_t sz, DEVICE_CONNNECTION* conn )
{
	socklen_t fromlen = sizeof(conn->sa_other_with_cl);
	int recsize = recvfrom(conn->sock_with_cl_accepted, (char *)(buff + buffer_in_with_cl_pos), sz - buffer_in_with_cl_pos, 0, (struct sockaddr *)&(conn->sa_other_with_cl), &fromlen);
	if (recsize < 0)
	{
#if defined _MSC_VER || defined __MINGW32__
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

uint8_t try_get_packet_size_within_master_loop( uint8_t* buff, DEVICE_CONNNECTION* conn )
{
	socklen_t fromlen = sizeof(conn->sa_other_with_cl);
	int recsize = recvfrom(conn->sock_with_cl_accepted, (char *)(buff + buffer_in_with_cl_pos), 2 - buffer_in_with_cl_pos, 0, (struct sockaddr *)&(conn->sa_other_with_cl), &fromlen);
	if (recsize < 0)
	{
#if defined _MSC_VER || defined __MINGW32__
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
		if ( buffer_in_with_cl_pos < 2 )
		{
			return COMMLAYER_RET_PENDING;
		}
		return COMMLAYER_RET_OK;
	}

}

uint8_t try_get_packet_using_context( uint8_t* buff, int max_sz, int*size, DEVICE_CONNNECTION* conn )
{
	buffer_in_with_cl_pos = 0;
	uint8_t ret;

	do //TODO: add delays or some waiting
	{
		ret = try_get_packet_size_within_master_loop( buff, conn );
	}
	while ( ret == COMMLAYER_RET_PENDING );
	if ( ret != COMMLAYER_RET_OK )
		return ret;
	uint16_t sz = buff[1]; sz <<= 8; sz += buff[0];

	buffer_in_with_cl_pos = 0;
	do //TODO: add delays or some waiting
	{
		ret = try_get_packet_within_master_loop( buff, sz, conn );
	}
	while ( ret == COMMLAYER_RET_PENDING );

	*size = sz;

#ifdef SA_DEBUG
	uint16_t i;
	ZEPTO_DEBUG_PRINTF_1( "PACKET RECEIVED FROM DEVICE: " );
	for ( i=0; i<sz; i++ )
		ZEPTO_DEBUG_PRINTF_2( "%02x ", buff[i] );
	ZEPTO_DEBUG_PRINTF_1( "\n" );
#endif
		
	return COMMLAYER_RET_OK;
}

uint8_t send_packet_using_context( const uint8_t* buff, int sz, DEVICE_CONNNECTION* conn )
{
#if 0//defSA_DEBUG
	uint16_t i;
	ZEPTO_DEBUG_PRINTF_1( "packet being sent: " );
	for ( i=0; i<sz; i++ )
		ZEPTO_DEBUG_PRINTF_2( "%02x ", buff[i] );
	ZEPTO_DEBUG_PRINTF_1( "\n" );
#endif

	ZEPTO_DEBUG_PRINTF_1( "send_within_master() called...\n" );
	uint8_t buff_prefix[ 2 ];

//	ZEPTO_DEBUG_ASSERT( sz != 0 ); // note: any valid message would have to have at least some bytes for headers, etc, so it cannot be empty
	ZEPTO_DEBUG_ASSERT( buff != NULL );
	buff_prefix[0] = (uint8_t)sz;
	buff_prefix[1] = sz >> 8;

	int bytes_sent = sendto(conn->sock_with_cl_accepted, (char*)buff_prefix, 2, 0, (struct sockaddr*)&(conn->sa_other_with_cl), sizeof( conn->sa_other_with_cl ) );
	if (bytes_sent < 0)
	{
#if defined _MSC_VER || defined __MINGW32__
		int error = WSAGetLastError();
		ZEPTO_DEBUG_PRINTF_2( "Error %d sending packet\n", error );
#else
		ZEPTO_DEBUG_PRINTF_2("Error sending packet: %s\n", strerror(errno));
#endif
		return COMMLAYER_RET_FAILED;
	}

	bytes_sent = sendto(conn->sock_with_cl_accepted, (char*)buff, sz, 0, (struct sockaddr*)&(conn->sa_other_with_cl), sizeof( conn->sa_other_with_cl ) );
	if (bytes_sent < 0)
	{
#if defined _MSC_VER || defined __MINGW32__
		int error = WSAGetLastError();
		ZEPTO_DEBUG_PRINTF_2( "Error %d sending packet\n", error );
#else
		ZEPTO_DEBUG_PRINTF_2("Error sending packet: %s\n", strerror(errno));
#endif
		return COMMLAYER_RET_FAILED;
	}
/*
#if defined _MSC_VER || defined __MINGW32__
	ZEPTO_DEBUG_PRINTF_4( "[%d] message sent within master; mem_h = %d, size = %d\n", GetTickCount(), mem_h, sz );
#else
	ZEPTO_DEBUG_PRINTF_3( "[--] message sent within master; mem_h = %d, size = %d\n", mem_h, sz );
#endif
*/
	return COMMLAYER_RET_OK;
}


uint8_t wait_for_packet_internal( uint16_t* src )
{
	// ZEPTO_DEBUG_PRINTF_1( "wait_for_communication_event()\n" );
    fd_set rfds;
    struct timeval tv;
    int retval;
	int fd_cnt;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);

	int i;
	int max_sock = sock_with_cl;
	FD_SET(sock_with_cl, &rfds);
	for ( i=0; i<MEX_DEV_CNT; i++ )
	{
		if ( devices[i].connected )
		{
			FD_SET(devices[i].sock_with_cl_accepted, &rfds);
			if ( max_sock < devices[i].sock_with_cl_accepted ) 
				max_sock = devices[i].sock_with_cl_accepted;
		}
	}
	fd_cnt = (int)(max_sock + 1);

    /* Wait */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    retval = select(fd_cnt, &rfds, NULL, NULL, &tv);
    /* Don't rely on the value of tv now! */

    if (retval == -1)
	{
#if defined _MSC_VER || defined __MINGW32__
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
		if ( FD_ISSET( sock_with_cl, &rfds) )
			return COMMLAYER_RET_REENTER_LISTEN;
		for ( i=0; i<MEX_DEV_CNT; i++ )
			if ( FD_ISSET( devices[i].sock_with_cl_accepted, &rfds) )
			{
				*src = i;
				return COMMLAYER_RET_OK;
			}
		ZEPTO_DEBUG_ASSERT(0);
		return COMMLAYER_RET_FAILED;
	}
    else
	{
        return COMMLAYER_RET_TIMEOUT;
	}
}

uint8_t start_listening()
{
	int sock_with_cl_accepted = accept(sock_with_cl, NULL, NULL);

      if ( 0 > sock_with_cl_accepted )
      {
        perror("error accept failed");
        CLOSE_SOCKET(sock_with_cl);
        exit(EXIT_FAILURE);
      }

	  ZEPTO_DEBUG_PRINTF_2( "connection accepted with port:%d\n", sock_with_cl_accepted );

	  devices[ dev_count ].sock_with_cl_accepted = sock_with_cl_accepted;
	  devices[ dev_count ].connected = true;
	  dev_count++;

	  return COMMLAYER_RET_OK;
}


uint8_t wait_for_packet( uint16_t* src )
{
	uint8_t ret_code;
	for(;;)
	{
		ret_code = wait_for_packet_internal( src );
		if ( ret_code == COMMLAYER_RET_OK )
			return ret_code;
		if ( ret_code == COMMLAYER_RET_REENTER_LISTEN )
			start_listening();
	}
}

uint8_t send_packet( const uint8_t* buff, int size, uint16_t target )
{
	if ( target != 0 )
	{
		target = target;
	}
	ZEPTO_DEBUG_ASSERT( target < MEX_DEV_CNT );
	DEVICE_CONNNECTION* conn = devices + target;
	return send_packet_using_context( buff, size, conn );
}

uint8_t get_packet( uint8_t* buff, int max_sz, int* size, uint16_t src )
{
	ZEPTO_DEBUG_ASSERT( src < MEX_DEV_CNT );
	DEVICE_CONNNECTION* conn = devices + src;
	return try_get_packet_using_context( buff, max_sz, size, conn );
}
