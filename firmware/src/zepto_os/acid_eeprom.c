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

#include <simpleiot_hal/siot_eeprom.h>
#include <hal_eeprom.h>

#define EEPROM_CHECKSUM_SIZE 4
#define EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE 16

// slot structure: | data | checksum | data | checksum |

#define SLOT_SIZE_FROM_DATA_SIZE( x ) ( 2 * ( (x) + EEPROM_CHECKSUM_SIZE ) )


void update_fletcher_checksum_16( uint8_t bt, uint16_t* state )
{
	// quick and dirty solution
	// TODO: implement
	uint8_t tmp = (uint8_t)(*state);
	uint8_t tmp1 = (*state) >> 8;
	tmp += bt;
	if ( tmp < bt )
		tmp += 1;
	if ( tmp == 0xFF )
		tmp = 0;
	tmp1 += tmp;
	if ( tmp1 < tmp )
		tmp1 += 1;
	if ( tmp1 == 0xFF )
		tmp1 = 0;
	*state = tmp1;
	*state <<= 8;
	*state += tmp;
}

void update_fletcher_checksum_32( uint16_t wrd, uint16_t* state )
{
	// quick and dirty solution
	// TODO: efficient implementation
	state[0] += wrd;
	if ( state[0] < wrd )
		state[0] += 1;
	if ( state[0] == 0xFFFF )
		state[0] = 0;
	state[1] += state[0];
	if ( state[1] < state[0] )
		state[1] += 1;
	if ( state[1] == 0xFFFF )
		state[1] = 0;
}

void init_checksum( uint8_t* checksum )
{
	ZEPTO_MEMSET( checksum, 0, EEPROM_CHECKSUM_SIZE );
}

bool is_same_checksum( uint8_t* checksum1, uint8_t* checksum2 )
{
	return ZEPTO_MEMCMP( checksum1, checksum2, EEPROM_CHECKSUM_SIZE ) == 0;
}

void calculate_checksum( const uint8_t* buff, uint16_t sz, uint8_t* checksum )
{
	uint16_t i;
	init_checksum( checksum );
#ifdef EEPROM_CHECKSUM_SIZE
#if (EEPROM_CHECKSUM_SIZE == 2)
	for ( i=0; i<sz; i++ )
		update_fletcher_checksum_16( buff[i], checksum );
#elif (EEPROM_CHECKSUM_SIZE == 4)
	for ( i=0; i<(sz>>1); i++ )
		update_fletcher_checksum_32( ((uint16_t*)buff)[i], (uint16_t*)checksum );
	if ( sz & 1 )
	{
		uint16_t wrd = buff[sz-1];
		update_fletcher_checksum_32( wrd, (uint16_t*)checksum );
	}
#else
#error unexpected value of EEPROM_CHECKSUM_SIZE
#endif
#else
#error EEPROM_CHECKSUM_SIZE is undefined
#endif
}


typedef struct _eeprom_slot_descriptor
{
	uint16_t offset;
	uint16_t data_size;
} eeprom_slot_descriptor;

const eeprom_slot_descriptor eeprom_slots[] ZEPTO_PROG_CONSTANT_LOCATION =
{
	{DATA_REINCARNATION_ID_SIZE * 2, DATA_SASP_NONCE_LW_SIZE},
	{DATA_REINCARNATION_ID_SIZE * 2 + SLOT_SIZE_FROM_DATA_SIZE( DATA_SASP_NONCE_LW_SIZE ), DATA_SASP_NONCE_LS_SIZE },
};

bool init_eeprom_access()
{
	return hal_init_eeprom_access();
}

bool eeprom_verify_checksum( uint16_t offset, uint16_t sz )
{
	uint8_t buff[EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE];
	uint8_t checksum_calculated[ EEPROM_CHECKSUM_SIZE ];
	uint8_t checksum_read[ EEPROM_CHECKSUM_SIZE ];
	ZEPTO_DEBUG_ASSERT( sz <= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE );
	ZEPTO_DEBUG_ASSERT( sizeof( checksum_calculated) == EEPROM_CHECKSUM_SIZE );
	ZEPTO_DEBUG_ASSERT( sizeof( checksum_read) == EEPROM_CHECKSUM_SIZE );
/*	while ( sz >= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE )
	{
		hal_eeprom_read( buff, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, offset );
		offset += EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		sz -= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		update_checksum( buff, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, checksum_calculated );
	}*/
	hal_eeprom_read( buff, sz, offset );
	offset += sz;
	calculate_checksum( buff, sz, checksum_calculated );
	hal_eeprom_read( (uint8_t*)checksum_read, EEPROM_CHECKSUM_SIZE, offset );
	return is_same_checksum( checksum_read, checksum_calculated );
}

void eeprom_copy_block( uint16_t target_offset, uint16_t src_offset, uint16_t sz )
{
	uint8_t buff[EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE];
	ZEPTO_DEBUG_ASSERT( sz <= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE );
/*	while ( sz >= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE )
	{
		hal_eeprom_read( buff, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, src_offset );
		hal_eeprom_write( buff, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, target_offset );
		src_offset += EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		target_offset += EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		sz -= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
	}*/
	hal_eeprom_read( buff, sz, src_offset );
	hal_eeprom_write( buff, sz, target_offset );
}

bool eeprom_are_blocks_same( uint16_t offset1, uint16_t offset2, uint16_t sz )
{
	uint8_t buff1[EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE];
	uint8_t buff2[EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE];
	ZEPTO_DEBUG_ASSERT( sz <= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE );
/*	while ( sz >= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE )
	{
		hal_eeprom_read( buff1, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, offset1 );
		hal_eeprom_read( buff2, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, offset2 );
		if ( ZEPTO_MEMCMP( buff1, buff2, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE ) != 0 )
			return false;
		offset1 += EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		offset2 += EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		sz -= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
	}*/
	hal_eeprom_read( buff1, sz, offset1 );
	hal_eeprom_read( buff2, sz, offset2 );
//	return ZEPTO_MEMCMP( buff1, buff2, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE ) == 0;
	return ZEPTO_MEMCMP( buff1, buff2, sz ) == 0;
}

bool eeprom_check_restore_slot( uint8_t id )
{
	uint16_t offset = eeprom_slots[id].offset;
	uint16_t data_sz = eeprom_slots[id].data_size;
	bool ok1, ok2;
	ZEPTO_DEBUG_ASSERT( id < EEPROM_SLOT_MAX );

	ok1 = eeprom_verify_checksum( offset, data_sz );
	offset += data_sz + EEPROM_CHECKSUM_SIZE;
	ok2 = eeprom_verify_checksum( offset, data_sz );

	offset = eeprom_slots[id].offset;

	if ( ok1 && ok2 )
	{
		if ( eeprom_are_blocks_same( offset, offset + data_sz + EEPROM_CHECKSUM_SIZE, data_sz + EEPROM_CHECKSUM_SIZE ) )
			return true;
		eeprom_copy_block( offset +  data_sz + EEPROM_CHECKSUM_SIZE, offset, data_sz + EEPROM_CHECKSUM_SIZE );
		return true;
	}

	if ( (!ok1) && (!ok2) )
		return false;

	if ( !ok1 )
	{
		ZEPTO_DEBUG_ASSERT( ok2 );
		eeprom_copy_block( offset, offset +  data_sz + EEPROM_CHECKSUM_SIZE, data_sz + EEPROM_CHECKSUM_SIZE );
		return true;
	}

	ZEPTO_DEBUG_ASSERT( ok1 );
	eeprom_copy_block( offset +  data_sz + EEPROM_CHECKSUM_SIZE, offset, data_sz + EEPROM_CHECKSUM_SIZE );
	return true;
}

bool eeprom_is_same_in_eeprom( uint16_t offset, uint8_t* mem, uint16_t sz )
{
	uint8_t buff[EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE];
	ZEPTO_DEBUG_ASSERT( sz <= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE );
	hal_eeprom_read( buff, sz, offset );
	return ZEPTO_MEMCMP( buff, mem, sz ) == 0;
}

uint8_t eeprom_check_reincarnation( uint8_t* rid )
{
	bool ok1 = eeprom_is_same_in_eeprom( 0, rid, DATA_REINCARNATION_ID_SIZE );
	bool ok2 = eeprom_is_same_in_eeprom( DATA_REINCARNATION_ID_SIZE, rid, DATA_REINCARNATION_ID_SIZE );
	if ( ok1 && ok2 )
		return EEPROM_RET_REINCARNATION_ID_OK_BOTH_OK;
	return (ok1 || ok2) ? EEPROM_RET_REINCARNATION_ID_OK_ONE_OK : EEPROM_RET_REINCARNATION_ID_OLD;
}

bool eeprom_check_at_start()
{
	uint8_t i;
	bool ok;
	ZEPTO_DEBUG_ASSERT( ( EEPROM_CHECKSUM_SIZE & 1 ) == 0 ); // even number
	for ( i=0; i<EEPROM_SLOT_MAX; i++ )
	{
		ok = eeprom_check_restore_slot( i );
		if (!ok )
			return false;
	}
	return true;
}

void eeprom_update_reincarnation_if_necessary( uint8_t* rid )
{
	if ( ! eeprom_is_same_in_eeprom( 0, rid, DATA_REINCARNATION_ID_SIZE ) )
		hal_eeprom_write( rid, DATA_REINCARNATION_ID_SIZE, 0 );
	if ( ! eeprom_is_same_in_eeprom( DATA_REINCARNATION_ID_SIZE, rid, DATA_REINCARNATION_ID_SIZE ) )
		hal_eeprom_write( rid, DATA_REINCARNATION_ID_SIZE, DATA_REINCARNATION_ID_SIZE );
}

void eeprom_write( uint8_t id, uint8_t* data)
{
	uint8_t checksum[ EEPROM_CHECKSUM_SIZE ]; init_checksum( checksum );
	uint16_t offset = eeprom_slots[id].offset;
	ZEPTO_DEBUG_ASSERT( id < EEPROM_SLOT_MAX );
	ZEPTO_DEBUG_ASSERT( sizeof( checksum) == EEPROM_CHECKSUM_SIZE );
	calculate_checksum( data, eeprom_slots[id].data_size, checksum );

	hal_eeprom_write( data, eeprom_slots[id].data_size, offset );
	offset += eeprom_slots[id].data_size;
	hal_eeprom_write( (uint8_t*)checksum, EEPROM_CHECKSUM_SIZE, offset );
	offset += EEPROM_CHECKSUM_SIZE;
	hal_eeprom_flush();

	hal_eeprom_write( data, eeprom_slots[id].data_size, offset );
	offset += eeprom_slots[id].data_size;
	hal_eeprom_write( (uint8_t*)checksum, EEPROM_CHECKSUM_SIZE, offset );
	offset += EEPROM_CHECKSUM_SIZE;
	hal_eeprom_flush();
}

void eeprom_read( uint8_t id, uint8_t* data)
{
	uint8_t checksum_calculated[ EEPROM_CHECKSUM_SIZE ];
	uint8_t checksum_read[ EEPROM_CHECKSUM_SIZE ];
	uint16_t offset = eeprom_slots[id].offset;
	ZEPTO_DEBUG_ASSERT( sizeof( checksum_calculated) == EEPROM_CHECKSUM_SIZE );
	ZEPTO_DEBUG_ASSERT( sizeof( checksum_read) == EEPROM_CHECKSUM_SIZE );
	ZEPTO_DEBUG_ASSERT( id < EEPROM_SLOT_MAX );

	hal_eeprom_read( data, eeprom_slots[id].data_size, offset );
	offset += eeprom_slots[id].data_size;
	hal_eeprom_read( (uint8_t*)checksum_read, EEPROM_CHECKSUM_SIZE, offset );
	offset += EEPROM_CHECKSUM_SIZE;
	init_checksum( checksum_calculated );
	calculate_checksum( data, eeprom_slots[id].data_size, checksum_calculated );
	if ( is_same_checksum( checksum_read, checksum_calculated ) )
		return;

	ZEPTO_DEBUG_ASSERT( 0 == "eeprom slot might be corrupted" );

	hal_eeprom_read( data, eeprom_slots[id].data_size, offset );
	offset += eeprom_slots[id].data_size;
	hal_eeprom_read( (uint8_t*)checksum_read, EEPROM_CHECKSUM_SIZE, offset );
	offset += EEPROM_CHECKSUM_SIZE;
	init_checksum( checksum_calculated );
	calculate_checksum( data, eeprom_slots[id].data_size, checksum_calculated );
	if ( is_same_checksum( checksum_read, checksum_calculated ) )
		return;

	ZEPTO_DEBUG_ASSERT( 0 == "eeprom slot is corrupted" );
}
