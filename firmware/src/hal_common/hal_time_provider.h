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

#if !defined __SA_HAL_TIME_PROVIDER_H__
#define __SA_HAL_TIME_PROVIDER_H__

//#include "../common/sa_common.h"
#include <simpleiot/siot_common.h>
#include "hal_platform.h"


typedef struct _sa_time_struct
{
	uint16_t low_t;
	uint16_t high_t;
} sa_time_struct;
// NOTE: the struct above is not to be used directly
// use typedef below instead
typedef sa_time_struct sa_time_val;

#define TIME_MILLISECONDS16_TO_TIMEVAL( mslow, timeval ) HAL_TIME_MILLISECONDS16_TO_TIMEVAL( mslow, timeval )
#define TIME_MILLISECONDS32_TO_TIMEVAL( mslow, mshigh, timeval ) HAL_TIME_MILLISECONDS32_TO_TIMEVAL( mslow, mshigh, timeval )

#ifdef __cplusplus
extern "C" {
#endif

void sa_get_time( sa_time_val* t ); // PLATFORM-SPECIFIC IMPLEMENTATION
uint32_t getTime(); // TODO: get rid of it (or move to HAL internals where applicable)
void sa_time_delay_ms(uint32_t ms); // TODO: move to HAL internals where applicable

#ifdef __cplusplus
}
#endif


// operations (to be added upon necessity)

#define SA_TIME_LOAD_TICKS_FOR_1_SEC( x ) {(x).low_t = 1000; (x).high_t = 0;}
#define SA_TIME_INCREMENT_BY_TICKS( x, y ) {(x).low_t += (y).low_t; (x).high_t += (y).high_t; (x).high_t += (x).low_t < (y).low_t ? 1 : 0;}
#define SA_TIME_SUBTRACT_TICKS_OR_ZERO( x, y ) {if ( sa_hal_time_val_is_less( &(x), (y) ) ) {(x).low_t = 0; (x).high_t = 0;} else {(x).low_t -= (y).low_t; (x).high_t -= (y).high_t; (x).high_t -= (x).low_t < (y).low_t ? 1 : 0;} }
#define SA_TIME_MUL_TICKS_BY_2( x ) {uint16_t tmp = ((x).low_t) >> 15; (x).low_t <<= 1; (x).high_t = ((x).high_t << 1) | tmp;}
#define SA_TIME_MUL_TICKS_BY_1_AND_A_HALF( x ) {uint16_t lo = (x).low_t, hi = (x).high_t; (x).low_t >>= 1; (x).low_t |= ( ((x).high_t & 1 ) << 15 ); (x).high_t >>= 1; (x).low_t += lo; (x).high_t += hi; (x).high_t += (x).low_t < lo ? 1 : 0;}
#define SA_TIME_SET_INFINITE_TIME( x ) {(x).low_t = 0Xffff; (x).high_t = 0xffff;}
#define SA_TIME_SET_ZERO_TIME( x ) {(x).low_t = 0; (x).high_t = 0;}

INLINE void sa_hal_time_val_copy_from( sa_time_val* t1, const sa_time_val* t2 )
{
	t1->high_t = t2->high_t;
	t1->low_t = t2->low_t;
}

INLINE bool sa_hal_time_val_is_less( const sa_time_val* t1,const  sa_time_val* t2 )
{
	if ( t1->high_t < t2->high_t ) return true;
	if ( t1->high_t > t2->high_t ) return false;
	return t1->low_t < t2->low_t;
}

INLINE bool sa_hal_time_val_is_less_eq( const sa_time_val* t1, const sa_time_val* t2 )
{
	if ( t1->high_t < t2->high_t ) return true;
	if ( t1->high_t > t2->high_t ) return false;
	return t1->low_t <= t2->low_t;
}

INLINE void sa_hal_time_val_copy_from_if_src_less( sa_time_val* t1, const sa_time_val* t2 )
{
	if ( t1->high_t < t2->high_t || ( t1->high_t == t2->high_t && t1->low_t <= t2->low_t ) ) return;
	t1->high_t = t2->high_t;
	t1->low_t = t2->low_t;
}

INLINE bool sa_hal_time_val_get_remaining_time( const sa_time_val* now, const sa_time_val* expected, sa_time_val* remaining )
{
	// returns false, if expected time is already in the past
	// returns true, if time remains before 'expected'
	// updates 'remaining' if 'expected' is greater than 'now' (not yet happened), and a new value is less than that currently in 'remaining'
	if ( expected->high_t < now->high_t ) return false; // already happpened; do not change 'remaining'
	if ( expected->high_t == now->high_t )
	{
		if ( expected->low_t <= now->low_t ) return false; // already happpened; do not change 'remaining'
		else
		{
			remaining->high_t = 0;
			if ( remaining->low_t > expected->low_t - now->low_t )
				remaining->low_t = expected->low_t - now->low_t;
			return true;
		}
	}
	else
	{
		sa_time_val diff;
		if ( expected->low_t < now->low_t )
		{
			diff.low_t = expected->low_t - now->low_t;
			diff.high_t = expected->high_t - now->high_t - 1;
		}
		else
		{
			diff.low_t = expected->low_t - now->low_t;
			diff.high_t = expected->high_t - now->high_t;
		}

		if ( remaining->high_t > diff.high_t || ( remaining->high_t == diff.high_t && remaining->low_t > diff.low_t ) )
		{
			remaining->high_t = diff.high_t;
			remaining->low_t = diff.low_t;
		}
		return true;
	}
}

#endif // __SA_HAL_TIME_PROVIDER_H__
