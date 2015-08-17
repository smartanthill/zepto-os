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

#ifdef MESH_TEST
#ifdef SA_RETRANSMITTER

#ifdef _MSC_VER
SOCKET sock;
SOCKET sock_accepted;
SOCKET sock2;
SOCKET sock_accepted2;
#else
int sock;
int sock_accepted;
int sock2;
int sock_accepted2;
#endif
struct sockaddr_in sa_self, sa_other;
const char* inet_addr_as_string = "127.0.0.1";
uint16_t self_port_num = 7654;
uint16_t other_port_num = 7667;

struct sockaddr_in sa_self2, sa_other2;
uint16_t self_port_num2 = 7767;
uint16_t other_port_num2 = 7754;

#else // terminal device
#ifdef _MSC_VER
SOCKET sock;
SOCKET sock_accepted;
#else
int sock;
int sock_accepted;
#endif
struct sockaddr_in sa_self, sa_other;
const char* inet_addr_as_string = "127.0.0.1";
uint16_t self_port_num = 7754;
uint16_t other_port_num = 7767;
#endif
#else
#ifdef _MSC_VER
SOCKET sock;
SOCKET sock_accepted;
#else
int sock;
int sock_accepted;
#endif
struct sockaddr_in sa_self, sa_other;
const char* inet_addr_as_string = "127.0.0.1";
uint16_t self_port_num = 7654;
uint16_t other_port_num = 7667;
#endif

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

#if (defined MESH_TEST) && (defined SA_RETRANSMITTER)
bool _communication_initialize_2()
{
	//Zero out socket address
	memset(&sa_self2, 0, sizeof sa_self2);
	memset(&sa_other2, 0, sizeof sa_other2);

	//create an internet, datagram, socket using UDP
	sock2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sock2) /* if socket failed to initialize, exit */
	{
		ZEPTO_DEBUG_PRINTF_1("Error Creating Socket\n");
		return false;
	}

	//The address is ipv4
	sa_other2.sin_family = AF_INET;
	sa_self2.sin_family = AF_INET;

	//ip_v4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value
	sa_self2.sin_addr.s_addr = inet_addr( inet_addr_as_string );
	sa_other2.sin_addr.s_addr = inet_addr( inet_addr_as_string );

	//sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654
	sa_self2.sin_port = htons( self_port_num2 );
	sa_other2.sin_port = htons( other_port_num2 );

	if (-1 == bind(sock2, (struct sockaddr *)&sa_self2, sizeof(sa_self2)))
	{
#ifdef _MSC_VER
		int error = WSAGetLastError();
#else
		int error = errno;
#endif
		ZEPTO_DEBUG_PRINTF_2( "bind sock failed; error %d\n", error );
		CLOSE_SOCKET(sock2);
		return false;
	}

	if (-1 == connect(sock2, (struct sockaddr *)&sa_other2, sizeof(sa_other2)))
		{
		  perror("connect failed");
			CLOSE_SOCKET(sock2);
		  return false;
		}
	return true;
}

#endif // (defined MESH_TEST) && (defined SA_RETRANSMITTER)

bool _communication_initialize()
{
	//Zero out socket address
	ZEPTO_MEMSET(&sa_self, 0, sizeof sa_self);
	ZEPTO_MEMSET(&sa_other, 0, sizeof sa_other);

	//create an internet, datagram, socket using UDP
//	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
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
	if(-1 == listen(sock, 10))
    {
      perror("error listen failed");
      CLOSE_SOCKET(sock);
      return false;
    }

	sock_accepted = accept(sock, NULL, NULL);

      if ( 0 > sock_accepted )
      {
        perror("error accept failed");
        CLOSE_SOCKET(sock);
        exit(EXIT_FAILURE);
      }

	  sock = sock_accepted; /*just to keep names*/

#ifdef _MSC_VER
    unsigned long ul = 1;
    ioctlsocket(sock, FIONBIO, &ul);
#else
    fcntl(sock,F_SETFL,O_NONBLOCK);
#endif
	return true;
}

void _communication_terminate()
{
	CLOSE_SOCKET(sock);
#if (defined MESH_TEST) && (defined SA_RETRANSMITTER)
	CLOSE_SOCKET(sock2);
#endif
}

uint8_t internal_send_packet( MEMORY_HANDLE mem_h, int _sock, struct sockaddr* _sa_other )
{
	ZEPTO_DEBUG_PRINTF_1( "send_message() called...\n" );

	uint16_t sz = memory_object_get_request_size( mem_h );
	memory_object_request_to_response( mem_h );
	ZEPTO_DEBUG_ASSERT( sz == memory_object_get_response_size( mem_h ) );
	ZEPTO_DEBUG_ASSERT( sz != 0 ); // note: any valid message would have to have at least some bytes for headers, etc, so it cannot be empty
	uint8_t* buff = memory_object_prepend( mem_h, 2 );
	ZEPTO_DEBUG_ASSERT( buff != NULL );
	buff[0] = (uint8_t)sz;
	buff[1] = sz >> 8;
	int bytes_sent = sendto(_sock, (char*)buff, sz+2, 0, _sa_other, sizeof (struct sockaddr) );
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

#ifdef MESH_TEST
uint8_t hal_send_packet( MEMORY_HANDLE mem_h, uint8_t bus_id, uint8_t intrabus_id )
{
#ifdef SA_RETRANSMITTER
	if ( bus_id == 0 )
		return internal_send_packet( mem_h, sock, (struct sockaddr *)(&sa_other) );
	else
	{
		ZEPTO_DEBUG_ASSERT( bus_id == 1 );
		return internal_send_packet( mem_h, sock2, (struct sockaddr *)(&sa_other2) );
	}
#else
	ZEPTO_DEBUG_ASSERT( bus_id == 0 );
	return internal_send_packet( mem_h, sock, (struct sockaddr *)(&sa_other) );
#endif
}
#else
uint8_t send_message( MEMORY_HANDLE mem_h )
{
	return internal_send_packet( mem_h, sock, (struct sockaddr *)(&sa_other) );
}
#endif


uint8_t try_get_packet( uint8_t* buff, uint16_t sz, int _sock, struct sockaddr* _sa_other )
{
	socklen_t fromlen = sizeof(struct sockaddr_in);
	int recsize = recvfrom(_sock, (char *)(buff + buffer_in_pos), sz - buffer_in_pos, 0, _sa_other, &fromlen);
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

uint8_t try_get_packet_size( uint8_t* buff, int _sock, struct sockaddr* _sa_other )
{
	socklen_t fromlen = sizeof(struct sockaddr_in);
	int recsize = recvfrom(_sock, (char *)(buff + buffer_in_pos), 2 - buffer_in_pos, 0, _sa_other, &fromlen);
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

uint8_t internal_get_packet_bytes( MEMORY_HANDLE mem_h, int _sock, struct sockaddr* _sa_other )
{
	// do cleanup
	memory_object_response_to_request( mem_h );
	memory_object_response_to_request( mem_h );
	uint8_t* buff = memory_object_append( mem_h, MAX_PACKET_SIZE );

	buffer_in_pos = 0;
	uint8_t ret;

	do //TODO: add delays or some waiting
	{
		ret = try_get_packet_size( buff, _sock, _sa_other );
	}
	while ( ret == COMMLAYER_RET_PENDING );
	if ( ret != COMMLAYER_RET_OK )
		return HAL_GET_PACKET_BYTES_FAILED;
	uint16_t sz = buff[1]; sz <<= 8; sz += buff[0];

	buffer_in_pos = 0;
	do //TODO: add delays or some waiting
	{
		ret = try_get_packet( buff, sz, _sock, _sa_other );
	}
	while ( ret == COMMLAYER_RET_PENDING );

	memory_object_response_to_request( mem_h );
	memory_object_cut_and_make_response( mem_h, 0, sz );

	return ret == COMMLAYER_RET_OK ? HAL_GET_PACKET_BYTES_DONE : HAL_GET_PACKET_BYTES_FAILED;
}

#ifdef MESH_TEST
	uint8_t bus_id_in = 0;
#endif


uint8_t hal_get_packet_bytes( MEMORY_HANDLE mem_h )
{
#ifdef MESH_TEST
#ifdef SA_RETRANSMITTER
	if ( bus_id_in == 0 )
		return internal_get_packet_bytes( mem_h, sock, (struct sockaddr *)(&sa_other) );
	else
	{
		ZEPTO_DEBUG_ASSERT( bus_id_in == 1 );
		return internal_get_packet_bytes( mem_h, sock2, (struct sockaddr *)(&sa_other2) );
	}
#else
	ZEPTO_DEBUG_ASSERT( bus_id_in == 0 );
	return internal_get_packet_bytes( mem_h, sock, (struct sockaddr *)(&sa_other) );
#endif
#else
	return internal_get_packet_bytes( mem_h, sock, (struct sockaddr *)(&sa_other) );
#endif
}




bool communication_initialize()
{
#if (defined MESH_TEST) && (defined SA_RETRANSMITTER)
	return communication_preinitialize() && _communication_initialize() && _communication_initialize_2();
#else
	return communication_preinitialize() && _communication_initialize();
#endif
}

void communication_terminate()
{
	_communication_terminate();
}

#if (defined MESH_TEST) && (defined SA_RETRANSMITTER)
uint8_t hal_get_busid_of_last_packet()
{
	return bus_id_in;
}
#endif

uint8_t wait_for_communication_event( unsigned int timeout )
{
	ZEPTO_DEBUG_PRINTF_1( "wait_for_communication_event()... " );
    fd_set rfds;
    struct timeval tv;
    int retval;
	int fd_cnt;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);

    FD_SET(sock, &rfds);
#ifdef MESH_TEST
#ifdef SA_RETRANSMITTER
    FD_SET(sock2, &rfds);
	fd_cnt = (int)(sock > sock2 ? sock + 1 : sock2 + 1);
#else
	fd_cnt = (int)(sock + 1);
#endif
#else
	fd_cnt = (int)(sock + 1);
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
		ZEPTO_DEBUG_ASSERT(0);
		ZEPTO_DEBUG_PRINTF_1( "COMMLAYER_RET_FAILED\n" );
		return COMMLAYER_RET_FAILED;
	}
    else if (retval)
	{
#ifdef MESH_TEST
#ifdef SA_RETRANSMITTER
		// so far, just resent to the other direction
		if ( FD_ISSET(sock, &rfds) )
		{
			ZEPTO_DEBUG_PRINTF_1( "Retransmitter: packet has come from MASTER\n" );
			bus_id_in = 0;
/*			MEMORY_HANDLE mem_h = 0;
			hal_get_packet_bytes( mem_h );
			zepto_response_to_request( mem_h );
			hal_send_packet( mem_h, 1, 0 );*/
		}
		else
		{
			ZEPTO_DEBUG_ASSERT( FD_ISSET(sock2, &rfds) );
			ZEPTO_DEBUG_PRINTF_1( "Retransmitter: packet has come from SLAVE\n" );
			bus_id_in = 1;
/*			MEMORY_HANDLE mem_h = 0;
			hal_get_packet_bytes( mem_h );
			zepto_response_to_request( mem_h );
			hal_send_packet( mem_h, 0, 0 );*/
		}
#else
		ZEPTO_DEBUG_PRINTF_1( "COMMLAYER_RET_FROM_DEV\n" );
#endif
#else
		ZEPTO_DEBUG_PRINTF_1( "COMMLAYER_RET_FROM_DEV\n" );
#endif
		return COMMLAYER_RET_FROM_DEV;
	}
    else
	{
		ZEPTO_DEBUG_PRINTF_1( "COMMLAYER_RET_TIMEOUT\n" );
        return COMMLAYER_RET_TIMEOUT;
	}
}

uint8_t wait_for_timeout( unsigned int timeout)
{
    struct timeval tv;
    int retval;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = ((long)timeout % 1000) * 1000;

    retval = select(0, NULL, NULL, NULL, &tv);

    if (retval == -1)
	{
#ifdef _MSC_VER
		int error = WSAGetLastError();
		ZEPTO_DEBUG_PRINTF_2( "error %d\n", error );
#else
        perror("select()");
//		int error = errno;
//		if ( error == EAGAIN || error == EWOULDBLOCK )
#endif
		ZEPTO_DEBUG_ASSERT(0);
		return COMMLAYER_RET_FAILED;
	}
    else
	{
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
	if ( wf->wait_packet )
	{
		ret_code = wait_for_communication_event( timeout );
		switch ( ret_code )
		{
			case COMMLAYER_RET_FROM_DEV: return WAIT_RESULTED_IN_PACKET; break;
			case COMMLAYER_RET_TIMEOUT: return WAIT_RESULTED_IN_TIMEOUT; break;
			case COMMLAYER_RET_FAILED: return WAIT_RESULTED_IN_FAILURE; break;
			default: return WAIT_RESULTED_IN_FAILURE;
		}
	}
	else
	{
		ret_code = wait_for_timeout( timeout );
		switch ( ret_code )
		{
			case COMMLAYER_RET_TIMEOUT: return WAIT_RESULTED_IN_TIMEOUT; break;
			default: return WAIT_RESULTED_IN_FAILURE;
		}
	}
}

void keep_transmitter_on( bool keep_on )
{
	// TODO: add reasonable implementation
}
