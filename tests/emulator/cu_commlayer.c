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

#include "cu_commlayer.h"
#include <simpleiot_hal/hal_waiting.h>
#include <stdio.h>

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
SOCKET sock;
SOCKET sock_accepted;
#else
int sock;
int sock_accepted;
#endif
struct sockaddr_in sa_self, sa_other;
const char* inet_addr_as_string = "127.0.0.1";

uint16_t self_port_num = 7667;
uint16_t other_port_num = 7654;

uint16_t buffer_in_pos;



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

bool _communication_initialize()
{
	//Zero out socket address
	memset(&sa_self, 0, sizeof sa_self);
	memset(&sa_other, 0, sizeof sa_other);

	//create an internet, datagram, socket using UDP
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sock) /* if socket failed to initialize, exit */
	{
		ZEPTO_DEBUG_PRINTF_1("Error Creating Socket\n");
		return false;
	}

	//The address is ipv4
	sa_other.sin_family = AF_INET;
	sa_self.sin_family = AF_INET;

	//ip_v4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value
	sa_self.sin_addr.s_addr = inet_addr( inet_addr_as_string );
	sa_other.sin_addr.s_addr = inet_addr( inet_addr_as_string );

	//sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654
	sa_self.sin_port = htons( self_port_num );
	sa_other.sin_port = htons( other_port_num );

	if (-1 == bind(sock, (struct sockaddr *)&sa_self, sizeof(sa_self)))
	{
#ifdef _MSC_VER
		int error = WSAGetLastError();
#else
		int error = errno;
#endif
		ZEPTO_DEBUG_PRINTF_2( "bind sock failed; error %d\n", error );
		CLOSE_SOCKET(sock);
		return false;
	}

	if (-1 == connect(sock, (struct sockaddr *)&sa_other, sizeof(sa_other)))
		{
		  perror("connect failed");
			CLOSE_SOCKET(sock);
		  return false;
		}
	return true;
}

void _communication_terminate()
{
	CLOSE_SOCKET(sock);
}

uint8_t send_message_to_slave( MEMORY_HANDLE mem_h, uint16_t bus_id )
{
	ZEPTO_DEBUG_PRINTF_1( "send_message() called...\n" );
	ZEPTO_DEBUG_ASSERT( bus_id == 0 ); // remove this assertion if more that a single bus is expected to be used at MASTER (ROOT)

	uint16_t sz = memory_object_get_request_size( mem_h );
	memory_object_request_to_response( mem_h );
	ZEPTO_DEBUG_ASSERT( sz == memory_object_get_response_size( mem_h ) );
	ZEPTO_DEBUG_ASSERT( sz != 0 ); // note: any valid message would have to have at least some bytes for headers, etc, so it cannot be empty
	uint8_t* buff = memory_object_prepend( mem_h, 3 );
	ZEPTO_DEBUG_ASSERT( buff != NULL );
	buff[0] = (uint8_t)(sz+1);
	buff[1] = (sz+1) >> 8;
	buff[2] = 0; // "regular" packet within testing system. NOTE: since AIR is used, it won't be received by an ultimate recipient
	int bytes_sent = sendto(sock, (char*)buff, sz+3, 0, (struct sockaddr*)&sa_other, sizeof sa_other);
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
	ZEPTO_DEBUG_PRINTF_4( "[%d] message sent; mem_h = %d, size = %d\n", GetTickCount(), mem_h, sz );
#else
	ZEPTO_DEBUG_PRINTF_3( "[--] message sent; mem_h = %d, size = %d\n", mem_h, sz );
#endif
	return COMMLAYER_RET_OK;
}

uint8_t try_get_packet( uint8_t* buff, uint16_t sz )
{
	socklen_t fromlen = sizeof(sa_other);
	int recsize = recvfrom(sock, (char *)(buff + buffer_in_pos), sz - buffer_in_pos, 0, (struct sockaddr *)&sa_other, &fromlen);
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
		buffer_in_pos += recsize;
		if ( buffer_in_pos < sz )
		{
			return COMMLAYER_RET_PENDING;
		}
		return COMMLAYER_RET_OK;
	}

}

uint8_t try_get_packet_size( uint8_t* buff )
{
	socklen_t fromlen = sizeof(sa_other);
	int recsize = recvfrom(sock, (char *)(buff + buffer_in_pos), 2 - buffer_in_pos, 0, (struct sockaddr *)&sa_other, &fromlen);
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
		buffer_in_pos += recsize;
		if ( buffer_in_pos < 2 )
		{
			return COMMLAYER_RET_PENDING;
		}
		return COMMLAYER_RET_OK;
	}

}

uint8_t hal_get_packet_bytes( MEMORY_HANDLE mem_h )
{
	// do cleanup
	memory_object_response_to_request( mem_h );
	memory_object_response_to_request( mem_h );
	uint8_t* buff = memory_object_append( mem_h, MAX_PACKET_SIZE );

	buffer_in_pos = 0;
	uint8_t ret;

	do //TODO: add delays or some waiting
	{
		ret = try_get_packet_size( buff );
	}
	while ( ret == COMMLAYER_RET_PENDING );
	if ( ret != COMMLAYER_RET_OK )
		return ret;
	uint16_t sz = buff[1]; sz <<= 8; sz += buff[0];

	buffer_in_pos = 0;
	do //TODO: add delays or some waiting
	{
		ret = try_get_packet( buff, sz );
	}
	while ( ret == COMMLAYER_RET_PENDING );

	memory_object_response_to_request( mem_h );
	memory_object_cut_and_make_response( mem_h, 0, sz );

	return HAL_GET_PACKET_BYTES_DONE;
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

uint16_t self_port_num_with_cl = 7655;
uint16_t other_port_num_with_cl = 7665;

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

	if (-1 == connect(sock_with_cl, (struct sockaddr *)&sa_other_with_cl, sizeof(sa_other_with_cl)))
    {
      perror("connect failed");
        CLOSE_SOCKET(sock_with_cl);
      return false;
    }

	return true;
}

void communication_with_comm_layer_terminate()
{
	CLOSE_SOCKET(sock_with_cl);
}

bool communication_initialize()
{
	return communication_preinitialize() && communication_with_comm_layer_initialize() && _communication_initialize();
}

void communication_terminate()
{
	_communication_terminate();
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

uint8_t try_get_packet_size_within_master_loop( uint8_t* buff, uint16_t sz )
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

uint8_t try_get_message_within_master( MEMORY_HANDLE mem_h, uint16_t* bus_id )
{
	// do cleanup
	memory_object_response_to_request( mem_h );
	memory_object_response_to_request( mem_h );
	uint8_t* buff = memory_object_append( mem_h, MAX_PACKET_SIZE );

	buffer_in_with_cl_pos = 0;
	uint8_t ret;

	do //TODO: add delays or some waiting
	{
		ret = try_get_packet_size_within_master_loop( buff, 5 );
	}
	while ( ret == COMMLAYER_RET_PENDING );
	if ( ret != COMMLAYER_RET_OK )
		return ret;
	uint16_t sz = buff[1]; sz <<= 8; sz += buff[0];
	*bus_id = buff[3]; *bus_id <<= 8; *bus_id += buff[2];
	uint8_t packet_src = buff[4];
	ZEPTO_DEBUG_PRINTF_4( "try_get_packet_size_within_master_loop(): sz = %d, bus_id = %d, src = %d\n", sz, *bus_id, packet_src );

	buffer_in_with_cl_pos = 0;
	do //TODO: add delays or some waiting
	{
		ret = try_get_packet_within_master_loop( buff, sz );
	}
	while ( ret == COMMLAYER_RET_PENDING );

	memory_object_response_to_request( mem_h );
	memory_object_cut_and_make_response( mem_h, 0, sz );

	if ( ret ==  COMMLAYER_RET_FAILED )
		return COMMLAYER_TO_CU_STATUS_RESERVED_FAILED;
	ZEPTO_DEBUG_ASSERT( ret == COMMLAYER_RET_OK );

#if 0//def _DEBUG
		uint16_t debug_sz = memory_object_get_response_size( mem_h );
		uint8_t* debug_ptr = memory_object_get_response_ptr( mem_h );
		ZEPTO_DEBUG_PRINTF_3( "try_get_message_within_master(), packet size = %d, for %s\n", debug_sz, packet_src == 37 ? "CU" : "SLAVE" );
		while ( debug_sz-- )
			ZEPTO_DEBUG_PRINTF_2( "%02x ", (debug_ptr++)[0] );
		ZEPTO_DEBUG_PRINTF_1( "\n\n" );
#endif

		return packet_src;
}

uint8_t send_within_master( MEMORY_HANDLE mem_h, uint16_t param, uint8_t destination )
{
//	ZEPTO_DEBUG_PRINTF_1( "send_within_master() called...\n" );

	uint16_t sz = memory_object_get_request_size( mem_h );
	memory_object_request_to_response( mem_h );
	ZEPTO_DEBUG_ASSERT( sz == memory_object_get_response_size( mem_h ) );
	ZEPTO_DEBUG_ASSERT( destination == COMMLAYER_FROM_CU_STATUS_INITIALIZER_LAST || sz != 0 ); // note: any valid message would have to have at least some bytes for headers, etc, so it cannot be empty
	uint8_t* buff = memory_object_prepend( mem_h, 5 );
	ZEPTO_DEBUG_ASSERT( buff != NULL );
	buff[0] = (uint8_t)sz;
	buff[1] = sz >> 8;
	buff[2] = (uint8_t)param;
	buff[3] = param >> 8;
	buff[4] = destination;
	int bytes_sent = sendto(sock_with_cl, (char*)buff, sz+5, 0, (struct sockaddr*)&sa_other_with_cl, sizeof sa_other_with_cl);
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
/*
#ifdef _MSC_VER
	ZEPTO_DEBUG_PRINTF_4( "[%d] message sent within master; mem_h = %d, size = %d\n", GetTickCount(), mem_h, sz );
#else
	ZEPTO_DEBUG_PRINTF_3( "[--] message sent within master; mem_h = %d, size = %d\n", mem_h, sz );
#endif
	*/
	return COMMLAYER_RET_OK;
}


uint8_t wait_for_communication_event( unsigned int timeout, uint16_t* bus_id )
{
//	ZEPTO_DEBUG_PRINTF_1( "wait_for_communication_event()... " );

	*bus_id = 0xFFFF;

    fd_set rfds;
    struct timeval tv;
    int retval;
	int fd_cnt;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);

    FD_SET(sock, &rfds);
	FD_SET(sock_with_cl, &rfds);
	fd_cnt = (int)(sock > sock_with_cl ? sock + 1 : sock_with_cl + 1);

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
		ZEPTO_DEBUG_PRINTF_1( "COMMLAYER_RET_FAILED\n" );
		return COMMLAYER_RET_FAILED;
	}
    else if (retval)
	{
		if ( FD_ISSET(sock, &rfds) )
		{
//			ZEPTO_DEBUG_PRINTF_1( "COMMLAYER_RET_FROM_DEV\n" );
			*bus_id = 0; // TODO: if more than a single bus is supported...
			return COMMLAYER_RET_FROM_DEV;
		}
		else
		{
			ZEPTO_DEBUG_ASSERT( FD_ISSET(sock_with_cl, &rfds) );
//			ZEPTO_DEBUG_PRINTF_1( "COMMLAYER_RET_FROM_CENTRAL_UNIT\n" );
			*bus_id = 0xFFFF; // not applicable
			return COMMLAYER_RET_FROM_CENTRAL_UNIT;
		}
	}
    else
	{
//		ZEPTO_DEBUG_PRINTF_1( "COMMLAYER_RET_TIMEOUT\n" );
        return COMMLAYER_RET_TIMEOUT;
	}
}

uint8_t send_message( MEMORY_HANDLE mem_h, uint16_t bus_id )
{
	return send_message_to_slave( mem_h, bus_id );
}


uint8_t send_to_commm_stack_as_from_master( MEMORY_HANDLE mem_h, uint16_t target_id )
{
	ZEPTO_DEBUG_ASSERT( target_id > 0 );
#ifdef _DEBUG
	{
		parser_obj po;
		zepto_parser_init( &po, mem_h );
		uint16_t sz = zepto_parsing_remaining_bytes( &po );
//		ZEPTO_DEBUG_PRINTF_3( "entering send_to_commm_stack_as_from_master(), packet size = %d, target = %d\n", sz, target_id );
		while ( sz-- )
			ZEPTO_DEBUG_PRINTF_2( "%02x ", zepto_parse_uint8( &po ) );
		ZEPTO_DEBUG_PRINTF_1( "\n\n" );
	}
#endif
	ZEPTO_DEBUG_PRINTF_2( "send_to_commm_stack_as_from_master(), about to call send_within_master(): rq.size = %d\n", memory_object_get_request_size( mem_h ) );
	return send_within_master( mem_h, target_id, 38 );
}

uint8_t send_to_commm_stack_as_from_slave( MEMORY_HANDLE mem_h, uint16_t bus_id )
{
	ZEPTO_DEBUG_ASSERT( bus_id != 0xFFFF );
#if 0//def _DEBUG
	parser_obj po;
	zepto_parser_init( &po, mem_h );
	uint16_t sz = zepto_parsing_remaining_bytes( &po );
	ZEPTO_DEBUG_PRINTF_2( "send_to_commm_stack_as_from_slave(), packet size = %d\n", sz );
	while ( sz-- )
		ZEPTO_DEBUG_PRINTF_2( "%02x ", zepto_parse_uint8( &po ) );
	ZEPTO_DEBUG_PRINTF_1( "\n\n" );
#endif
	return send_within_master( mem_h, bus_id, COMMLAYER_FROM_CU_STATUS_FROM_SLAVE );
}

uint8_t send_to_commm_stack_initializing_packet( MEMORY_HANDLE mem_h, uint16_t ordinal )
{
#if 0//def _DEBUG
	parser_obj po;
	zepto_parser_init( &po, mem_h );
	uint16_t sz = zepto_parsing_remaining_bytes( &po );
	ZEPTO_DEBUG_PRINTF_2( "send_to_commm_stack_as_from_slave(), packet size = %d\n", sz );
	while ( sz-- )
		ZEPTO_DEBUG_PRINTF_2( "%02x ", zepto_parse_uint8( &po ) );
	ZEPTO_DEBUG_PRINTF_1( "\n\n" );
#endif
	return send_within_master( mem_h, ordinal, COMMLAYER_FROM_CU_STATUS_INITIALIZER );
}

uint8_t send_to_commm_stack_end_of_initialization_packet( uint16_t count )
{
	MEMORY_HANDLE mem_h = acquire_memory_handle();
	uint8_t ret_code = send_within_master( mem_h, count, COMMLAYER_FROM_CU_STATUS_INITIALIZER_LAST );
	release_memory_handle( mem_h );
	return ret_code;
}

uint8_t send_to_commm_stack_reply( MEMORY_HANDLE mem_h, uint16_t packet_id )
{
	return send_within_master( mem_h, packet_id, COMMLAYER_FROM_CU_STATUS_SYNC_RESPONSE );
}
