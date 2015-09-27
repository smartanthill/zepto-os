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

#if !defined __TIME_COMMON_H__
#define __TIME_COMMON_H__

// data types
#if 0//def _MSC_VER
#define uint8_t unsigned char
#define int8_t char
#define uint16_t unsigned short
#define int16_t short
#define uint32_t unsigned int
#else
#include "stdint.h"
#endif

#ifdef _MSC_VER
#define NOINLINE      __declspec(noinline)
#define INLINE __inline
#define FORCE_INLINE	__forceinline
#else
#define INLINE static inline
#define NOINLINE      __attribute__ ((noinline))
#define	FORCE_INLINE static inline __attribute__((always_inline))
#endif

// compiler-specific: disabling certain warnings
#ifdef _MSC_VER
#pragma warning (disable: 4996) // "The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name <...>"
#else
#endif

#ifndef INLINE
#define INLINE static inline
#endif
#ifndef NOINLINE
#define NOINLINE      __attribute__ ((noinline))
#endif
#ifndef FORCE_INLINE
#define FORCE_INLINE static inline __attribute__((always_inline))
#endif

typedef unsigned long long time_id_type;


#ifdef SA_DEBUG
#if !defined DEBUG_PRINTING
#define DEBUG_PRINTING
#endif
#endif

#ifdef DEBUG_PRINTING
#include <stdio.h>
#define printf FORBIDDEN_CALL_USE_MACROS_INSTEAD
#define ZEPTO_DEBUG_PRINTF_1( s ) fprintf( stdout, s )
#define ZEPTO_DEBUG_PRINTF_2( s, x1 ) fprintf( stdout, s, x1 )
#define ZEPTO_DEBUG_PRINTF_3( s, x1, x2 ) fprintf( stdout, s, x1, x2 )
#define ZEPTO_DEBUG_PRINTF_4( s, x1, x2, x3 ) fprintf( stdout, s, x1, x2, x3 )
#define ZEPTO_DEBUG_PRINTF_5( s, x1, x2, x3, x4 ) fprintf( stdout, s, x1, x2, x3, x4 )
#define ZEPTO_DEBUG_PRINTF_6( s, x1, x2, x3, x4, x5 ) fprintf( stdout, s, x1, x2, x3, x4, x5 )
#define ZEPTO_DEBUG_PRINTF_7( s, x1, x2, x3, x4, x5, x6 ) fprintf( stdout, s, x1, x2, x3, x4, x5, x6 )
#else // DEBUG_PRINTING
#include <stdio.h>
//#define printf FORBIDDEN_CALL_OF_PRINTF
#define fprintf FORBIDDEN_CALL_OF_FPRINTF
#define ZEPTO_DEBUG_PRINTF_1( s )
#define ZEPTO_DEBUG_PRINTF_2( s, x1 )
#define ZEPTO_DEBUG_PRINTF_3( s, x1, x2 )
#define ZEPTO_DEBUG_PRINTF_4( s, x1, x2, x3 )
#define ZEPTO_DEBUG_PRINTF_5( s, x1, x2, x3, x4 )
#define ZEPTO_DEBUG_PRINTF_6( s, x1, x2, x3, x4, x5 )
#define ZEPTO_DEBUG_PRINTF_7( s, x1, x2, x3, x4, x5, x6 )
#endif // DEBUG_PRINTING

#ifdef SA_DEBUG
#include <assert.h>
#define ZEPTO_DEBUG_ASSERT( x ) assert( x )
#define ZEPTO_RUNTIME_CHECK( x ) assert( x )
#else
#define assert FORBIDDEN_CALL_OF_ASSERT
#define ZEPTO_DEBUG_ASSERT( x )
#define ZEPTO_RUNTIME_CHECK( x )  //TODO: define
#endif


#endif // __TIME_COMMON_H__
