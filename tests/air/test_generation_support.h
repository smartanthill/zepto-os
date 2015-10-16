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

#if !defined __TEST_GENERATOR_H__
#define __TEST_GENERATOR_H__

uint16_t tester_get_rand_val();

class PACKET
{
	uint8_t* buff;
	int sz;
public:
	PACKET() {buff = NULL; sz = 0;}
	void init( const uint8_t* _buff, int _sz ) {if ( buff != NULL ) delete [] buff; buff = new uint8_t[sz]; ZEPTO_MEMCPY( buff, _buff, sz ); sz = _sz;}
	~PACKET() {if ( buff != NULL ) delete [] buff;}
	int size() const {return sz;}
	const uint8_t* data() const {return buff;}
};
void add_new_packet( const uint8_t* buff, int sz );
bool get_random_packet( PACKET* p );

///////////////////////////////////////////////////////////

typedef struct _COMM_PARTICIPANT
{
	int dev_id;
	int cnt_to;
	int cnt_from;
	int src_val1;
	int src_val2;
	int dest_val1;
	int dest_val2;
} COMM_PARTICIPANT;

#define COMM_PARTICIPANTS_MAX_COUNT 64


#endif // __TEST_GENERATOR_H__
