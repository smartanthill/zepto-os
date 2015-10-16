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

#include "air_common.h"
#ifdef printf
#undef printf
#endif

#include "test_generation_support.h"
#include <stdlib.h> // for get_rand_val()
#include <vector>
#include <list>
using namespace std;

uint16_t tester_get_rand_val(){	return (uint16_t)( rand() );}


class RECENT_PACKETS
{
	struct packet
	{
		uint8_t* buff;
		int sz;
	};

	typedef list<packet> PACKET_LIST;
	typedef list<packet>::iterator PACKET_LIST_ITERATOR;

	PACKET_LIST packet_list;
	enum { PACKET_LIST_MAX_SIZE = 128};
	int cnt;

	void init_packet( packet* p, const uint8_t* _buff, int _sz ) {p->buff = new uint8_t[_sz]; ZEPTO_MEMCPY( p->buff, _buff, _sz ); p->sz = _sz;}
	void deinit_packet( packet* p ) {if ( p->buff != NULL ) delete [] p->buff; p->buff = NULL; p->sz = 0;}

public:
	RECENT_PACKETS() { cnt = 0; }
	void add_new_packet( const uint8_t* buff, int sz )
	{
		packet p;
		init_packet( &p, buff, sz );
		packet_list.push_back( p );
		if ( cnt < PACKET_LIST_MAX_SIZE )
			cnt ++;
		else
		{
			packet_list.erase( packet_list.begin() );
		}
	}

	bool get_random_packet( PACKET* p )
	{
		if ( cnt == 0 ) return false;
		int pos =  tester_get_rand_val() % cnt;
		PACKET_LIST_ITERATOR it = packet_list.begin();
		while( pos--) ++it;
		p->init( it->buff, it->sz );
		return true;
	}
};

RECENT_PACKETS recent_packets;
void add_new_packet( const uint8_t* buff, int sz ) {recent_packets.add_new_packet( buff, sz );}
bool get_random_packet( PACKET* p ) {return recent_packets.get_random_packet( p );}
