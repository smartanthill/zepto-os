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
#define EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE 4

// slot structure: | data | checksum | data | checksum |

#define SLOT_SIZE_FROM_DATA_SIZE( x ) ( 2 * ( (x) + EEPROM_CHECKSUM_SIZE ) )


// data offsets and sizes
#define DATA_SASP_NONCE_LW_SIZE 6 // Nonce Lower Watermark
#define DATA_SASP_NONCE_LS_SIZE 6 // Nonce to use For Sending

void update_checksum_16_single_byte( uint8_t bt, uint16_t* state )
{
	// quick and dirty solution
	// TODO: implement
	*state += bt;
}

void update_checksum_32_single_byte( uint8_t bt, uint32_t* state )
{
	// quick and dirty solution
	// TODO: implement
	*state += bt;
}

void update_checksum_16( const uint8_t* buff, uint16_t sz, uint16_t* state )
{
	uint16_t i;
	for ( i=0; i<sz; i++ )
		update_checksum_16_single_byte( buff[i], state );
}

void update_checksum_32( const uint8_t* buff, uint16_t sz, uint32_t* state )
{
	uint16_t i;
	for ( i=0; i<sz; i++ )
		update_checksum_32_single_byte( buff[i], state );
}

#ifdef EEPROM_CHECKSUM_SIZE
#if (EEPROM_CHECKSUM_SIZE == 2)
#define update_checksum update_checksum_16
#elif (EEPROM_CHECKSUM_SIZE == 4)
#define update_checksum update_checksum_32
#else
#error unexpected value of EEPROM_CHECKSUM_SIZE
#endif
#else
#error EEPROM_CHECKSUM_SIZE is undefined
#endif

typedef struct _eeprom_slot_descriptor
{
	uint16_t offset;
	uint16_t data_size;
} eeprom_slot_descriptor;

const eeprom_slot_descriptor eeprom_slots[] ZEPTO_PROG_CONSTANT_LOCATION =
{
	{0, DATA_SASP_NONCE_LW_SIZE},
	{SLOT_SIZE_FROM_DATA_SIZE( DATA_SASP_NONCE_LW_SIZE ), DATA_SASP_NONCE_LS_SIZE },
};

bool init_eeprom_access()
{
	return hal_init_eeprom_access();
}

bool eeprom_verify_checksum( uint16_t offset, uint16_t sz )
{
	uint8_t buff[EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE];
	uint32_t checksum_calculated = 0;
	uint32_t checksum_read;
	ZEPTO_DEBUG_ASSERT( sizeof( checksum_calculated) == EEPROM_CHECKSUM_SIZE );
	ZEPTO_DEBUG_ASSERT( sizeof( checksum_read) == EEPROM_CHECKSUM_SIZE );
	while ( sz >= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE )
	{
		hal_eeprom_read( buff, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, offset );
		offset += EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		sz -= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		update_checksum( buff, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, &checksum_calculated );
	}
	hal_eeprom_read( buff, sz, offset );
	offset += sz;
	update_checksum( buff, sz, &checksum_calculated );
	hal_eeprom_read( (uint8_t*)&checksum_read, EEPROM_CHECKSUM_SIZE, offset );
	return checksum_read == checksum_calculated;
}

void eeprom_copy_block( uint16_t target_offset, uint16_t src_offset, uint16_t sz )
{
	uint8_t buff[EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE];
	while ( sz >= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE )
	{
		hal_eeprom_read( buff, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, src_offset );
		hal_eeprom_write( buff, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, target_offset );
		src_offset += EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		target_offset += EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		sz -= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
	}
	hal_eeprom_read( buff, sz, src_offset );
	hal_eeprom_write( buff, sz, target_offset );
}

bool eeprom_are_blocks_same( uint16_t offset1, uint16_t offset2, uint16_t sz )
{
	uint8_t buff1[EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE];
	uint8_t buff2[EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE];
	while ( sz >= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE )
	{
		hal_eeprom_read( buff1, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, offset1 );
		hal_eeprom_read( buff2, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE, offset2 );
		if ( ZEPTO_MEMCMP( buff1, buff2, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE ) != 0 )
			return false;
		offset1 += EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		offset2 += EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
		sz -= EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE;
	}
	hal_eeprom_read( buff1, sz, offset1 );
	hal_eeprom_read( buff2, sz, offset2 );
	return ZEPTO_MEMCMP( buff1, buff2, EEPROM_LOCAL_BUFF_FOR_CHECKUP_SIZE ) == 0;
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

bool eeprom_check_at_start()
{
	uint8_t i;
	bool ok;
	for ( i=0; i<EEPROM_SLOT_MAX; i++ )
	{
		ok = eeprom_check_restore_slot( i );
		if (!ok )
			return false;
	}
	return true;
}

void eeprom_write( uint8_t id, uint8_t* data)
{
	uint32_t checksum = 0;
	uint16_t offset = eeprom_slots[id].offset;
	ZEPTO_DEBUG_ASSERT( id < EEPROM_SLOT_MAX );
	ZEPTO_DEBUG_ASSERT( sizeof( checksum) == EEPROM_CHECKSUM_SIZE );
	update_checksum( data, eeprom_slots[id].data_size, &checksum );

	hal_eeprom_write( data, eeprom_slots[id].data_size, offset );
	offset += eeprom_slots[id].data_size;
	hal_eeprom_write( (uint8_t*)&checksum, EEPROM_CHECKSUM_SIZE, offset );
	offset += EEPROM_CHECKSUM_SIZE;
	hal_eeprom_flush();

	hal_eeprom_write( data, eeprom_slots[id].data_size, offset );
	offset += eeprom_slots[id].data_size;
	hal_eeprom_write( (uint8_t*)&checksum, EEPROM_CHECKSUM_SIZE, offset );
	offset += EEPROM_CHECKSUM_SIZE;
	hal_eeprom_flush();
}

void eeprom_read( uint8_t id, uint8_t* data)
{
	uint32_t checksum_calculated;
	uint32_t checksum_read;
	uint16_t offset = eeprom_slots[id].offset;
	ZEPTO_DEBUG_ASSERT( sizeof( checksum_calculated) == EEPROM_CHECKSUM_SIZE );
	ZEPTO_DEBUG_ASSERT( sizeof( checksum_read) == EEPROM_CHECKSUM_SIZE );
	ZEPTO_DEBUG_ASSERT( id < EEPROM_SLOT_MAX );

	hal_eeprom_read( data, eeprom_slots[id].data_size, offset );
	offset += eeprom_slots[id].data_size;
	hal_eeprom_read( (uint8_t*)&checksum_read, EEPROM_CHECKSUM_SIZE, offset );
	offset += EEPROM_CHECKSUM_SIZE;
	checksum_calculated = 0;
	update_checksum( data, eeprom_slots[id].data_size, &checksum_calculated );
	if ( checksum_read == checksum_calculated )
		return;

	ZEPTO_DEBUG_ASSERT( 0 == "eeprom slot might be corrupted" );

	hal_eeprom_read( data, eeprom_slots[id].data_size, offset );
	offset += eeprom_slots[id].data_size;
	hal_eeprom_read( (uint8_t*)&checksum_read, EEPROM_CHECKSUM_SIZE, offset );
	offset += EEPROM_CHECKSUM_SIZE;
	checksum_calculated = 0;
	update_checksum( data, eeprom_slots[id].data_size, &checksum_calculated );
	if ( checksum_read == checksum_calculated )
		return;

	ZEPTO_DEBUG_ASSERT( 0 == "eeprom slot is corrupted" );
}
