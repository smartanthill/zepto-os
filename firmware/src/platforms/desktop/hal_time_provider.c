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

#include <hal_time_provider.h>
#include <simpleiot_hal/hal_commlayer.h>

#ifdef USE_TIME_MASTER // NOTE: code with USE_TIME_MASTER defined is intended for testing purposes only on 'desktop' platform and should not be taken as a sample for any other platform
#include "hal_commlayer_to_time_master.h"
typedef struct _GETTIME_CALL_POINT { void* file; uint16_t line;  } GETTIME_CALL_POINT;
#define GETTIME_CALL_POINT_MAX 4
static GETTIME_CALL_POINT gettime_call_points[GETTIME_CALL_POINT_MAX];
static uint8_t callpoints_assigned = 0;
uint8_t get_call_point_index( void* file, uint16_t line )
{
	uint8_t i;
	for ( i=0;i<callpoints_assigned; i++)
		if ( gettime_call_points[i].file == file && gettime_call_points[i].line == line )
			return i;
	if ( callpoints_assigned < GETTIME_CALL_POINT_MAX )
	{
		gettime_call_points[callpoints_assigned].file = file;
		gettime_call_points[callpoints_assigned].line = line;
		return callpoints_assigned++;
	}
	ZEPTO_DEBUG_ASSERT( 0 == "Too many get_time call points detected" );
	return 0xFF;
}
#endif // USE_TIME_MASTER


#ifdef _MSC_VER

#include <Windows.h>

#ifdef USE_TIME_MASTER // NOTE: code with USE_TIME_MASTER defined is intended for testing purposes only on 'desktop' platform and should not be taken as a sample for any other platform
void _sa_get_time( sa_time_val* t )
#else
void sa_get_time( sa_time_val* t )
#endif
{
	unsigned int sys_t = GetTickCount();
	t->high_t = sys_t >> 16;
	t->low_t = (unsigned short)sys_t;
}
/*
uint32_t getTime()
{
	return (uint32_t)( GetTickCount() / 200 );
}
*/
void mcu_sleep( uint16_t sec, uint8_t transmitter_state_on_exit )
{
	if ( transmitter_state_on_exit == 0 )
		keep_transmitter_on( false );
	Sleep( ( unsigned int )sec * 1000 );
	if ( transmitter_state_on_exit )
		keep_transmitter_on( true );
}

void just_sleep( sa_time_val* timeval )
{
	unsigned int ms = timeval->high_t;
	ms <<= 16;
	ms += timeval->low_t;
	Sleep( ms );
}


#else

#include <unistd.h>
#include <time.h>

#ifdef __MACH__
#include <sys/time.h>
#define CLOCK_MONOTONIC 1

int clock_gettime(int clk_id, struct timespec* t) {
    struct timeval now;
    int rv = gettimeofday(&now, NULL);
    if (rv) return rv;
    t->tv_sec  = now.tv_sec;
    t->tv_nsec = now.tv_usec * 1000;
    return 0;
}
#endif

uint32_t getTick() {
    struct timespec ts;
    unsigned theTick = 0U;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    theTick  = ts.tv_nsec / 1000000;
    theTick += ts.tv_sec * 1000;
    return theTick;
}


#ifdef USE_TIME_MASTER // NOTE: code with USE_TIME_MASTER defined is intended for testing purposes only on 'desktop' platform and should not be taken as a sample for any other platform
void _sa_get_time( sa_time_val* t )
#else
void sa_get_time( sa_time_val* t )
#endif
{
	unsigned int sys_t = getTick();
	t->high_t = sys_t >> 16;
	t->low_t = (unsigned short)sys_t;
}

void mcu_sleep( uint16_t sec, uint8_t transmitter_state_on_exit )
{
	if ( transmitter_state_on_exit == 0 )
		keep_transmitter_on( false );
	sleep( ( unsigned int )sec * 1000 );
	if ( transmitter_state_on_exit )
		keep_transmitter_on( true );
}

void just_sleep( sa_time_val* timeval )
{
	unsigned int ms = timeval->high_t;
	ms <<= 16;
	ms += timeval->low_t;
	sleep( ms );
}


// TODO: get rid of it
uint32_t getTime()
{
	return (uint32_t)( getTick() / 200 );
}

#endif

#ifdef USE_TIME_MASTER // NOTE: code with USE_TIME_MASTER defined is intended for testing purposes only on 'desktop' platform and should not be taken as a sample for any other platform
void sa_get_time( sa_time_val* t, void* file, uint16_t line )
{
	uint8_t point_id = get_call_point_index( file, line );
#if !defined USE_TIME_MASTER_REGISTER
	request_time_val( point_id, t );
	return;
#endif // USE_TIME_MASTER_REGISTER

	_sa_get_time( t );

#ifdef USE_TIME_MASTER_REGISTER
	register_time_val( point_id, t, t );
#endif // USE_TIME_MASTER_REGISTER
}
#endif // USE_TIME_MASTER
