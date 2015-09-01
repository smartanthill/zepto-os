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


/*******************************************************************************
  CONTENT OF THIS FILE IS INTENDED FOR DEBUG PURPOSES ONLY
  and implements a highly simplified siot/gdp.
  
  This implementation is symmetric to that on the Device side and just generates
  a simple packet with status "is-first" that is expected to be returned as-is.
*******************************************************************************/

#if !defined VERY_DEBUG
#error This file is intended for DEBUG PURPOSES ONLY and cannot be used when VERY_DEBUG is not defined
#else // VERY_DEBUG
#if (!defined VERY_DEBUG_SIOT_CCP)
#error This file is intended for DEBUG PURPOSES ONLY and can be used when one of VERY_DEBUG_SIOT_CCP is defined
#else // VERY_DEBUG_SIOT_CCP


#include "saccp_protocol_client_side_cu.h"


uint8_t handler_saccp_prepare_to_send( MEMORY_HANDLE mem_h )
{
	zepto_parser_free_memory( mem_h );
	zepto_write_uint8( mem_h, 1 ); // "is-first"
	zepto_write_block( mem_h, ">>>>>> PACKET <<<<<<", 20 );
	ZEPTO_DEBUG_PRINTF_1( "handler_saccp_prepare_to_send(): is-first, \">>>>>> PACKET <<<<<<\"\n" );
	return SACCP_RET_PASS_LOWER;
}

uint8_t handler_saccp_receive( MEMORY_HANDLE mem_h, sasp_nonce_type chain_id, DefaultTestingControlProgramState* DefaultTestingControlProgramState_struct )
{
	parser_obj po;
	zepto_parser_init( &po, mem_h );
	uint8_t first_byte = zepto_parse_uint8( &po );
	uint16_t sz = zepto_parsing_remaining_bytes( &po );
	uint16_t i;
	ZEPTO_DEBUG_PRINTF_3( "handler_saccp_receive(): %d, [%d] \"", first_byte, sz );
	for ( i=0; i<sz; i++)
		ZEPTO_DEBUG_PRINTF_2( "%c", (char)(zepto_parse_uint8( &po )) );
	ZEPTO_DEBUG_PRINTF_1( "\"\n" );

	handler_saccp_prepare_to_send( mem_h );

	return CONTROL_PROG_CHAIN_CONTINUE;
}



#endif // VERY_DEBUG_SIOT_CCP
#endif // VERY_DEBUG

