/** @file mock.c
 *  @brief mock file with read and write functions
 *
 *  @author turchenkov@gmail.com
 *  @bug
 *  @date 26-Jan-2019
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "mock.h"

#define NVMEM_SIZE	FRAM_SIZE

static uint32_t rc_crc32(uint32_t crc, const char *buf, size_t len);

/* emulates FRAM */
static uint8_t	FRAM[FRAM_SIZE];

/**
 * @brief  Read_FRAM reads bytes from non-volatile FRAM memory
 * @note
 * @param  addr_to is a pointer to the RAM buffer
 * @param  fram_addr is address of data in the FRAM chip
 * @param  frlen is length of the data to be read
 * @note   if (fram_addr+frlen) >= chip memory size, the function returns ERROR
 * @retval ERROR or SUCCESS
 */
ErrorStatus Read_FRAM(uint8_t *addr_to, uint32_t fram_addr, size_t frlen)
{
	ErrorStatus result;
	result = ERROR;

	if (addr_to == NULL) {
		goto fExit;
	}

	if ((fram_addr + frlen) > NVMEM_SIZE) {
		goto fExit;
	}

	for (size_t i = 0U; i < frlen; i++) {
		*addr_to = FRAM[fram_addr + i];
		addr_to++;
	}
	result = SUCCESS;

fExit:
	return result;
}

/**
 * @brief  Write_FRAM writes bytes to non-volatile FRAM memory
 * @note
 * @param  addr_from is a pointer to the RAM buffer
 * @param  fram_addr is a destination address in the FRAM chip
 * @param  frlen is length of the data to be written
 * @note   if (fram_addr+frlen) >= chip memory size, the function returns ERROR
 * @retval ERROR or SUCCESS
 */
ErrorStatus Write_FRAM(uint8_t *addr_from, uint32_t fram_addr, size_t frlen)
{
	ErrorStatus result;
	result = ERROR;

	if (addr_from == NULL) {
		goto fExit;
	}

	if ((fram_addr + frlen) > NVMEM_SIZE) {
		goto fExit;
	}

	for (size_t i = 0U; i < frlen; i++) {
		FRAM[fram_addr + i] = *addr_from;
		addr_from++;
	}

	result = SUCCESS;
fExit:
	return result;
}

/**
 * @brief CRC32 calculates CRC32
 * @param addr_from pointer to the data buffer
 * @param len length of the buffer
 * @param initCRC initial CRC32 value
 * @return CRC32
 */
uint32_t CRC32(uint8_t * addr_from, size_t len, uint32_t initCRC)
{
	return rc_crc32(initCRC, (const char *)addr_from, len);
}


uint32_t
rc_crc32(uint32_t crc, const char *buf, size_t len)
{
	static uint32_t table[256];
	static int have_table = 0;
	uint32_t rem;
	uint8_t octet;
	int i, j;
	const char *p, *q;

	/* This check is not thread safe; there is no mutex. */
	if (have_table == 0) {
		/* Calculate CRC table. */
		for (i = 0; i < 256; i++) {
			rem = i;  /* remainder from polynomial division */
			for (j = 0; j < 8; j++) {
				if (rem & 1) {
					rem >>= 1;
					rem ^= 0xedb88320;
				} else
					rem >>= 1;
			}
			table[i] = rem;
		}
		have_table = 1;
	}

	crc = ~crc;
	q = buf + len;
	for (p = buf; p < q; p++) {
		octet = *p;  /* Cast to unsigned octet. */
		crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
	}
	return ~crc;
}

