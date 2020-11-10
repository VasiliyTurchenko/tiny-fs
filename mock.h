/** @file mock.h
 *  @brief mock header file with read and write functions
 *
 *  @author turchenkov@gmail.com
 *  @bug
 *  @date 26-Jan-2019
 */

#ifndef _MOCK_H
#define _MOCK_H

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#include "port.h"

#define LOCK
#define UNLOCK

#define	FRAM_SIZE	8192U


ErrorStatus Read_FRAM(uint8_t *addr_to, uint32_t fram_addr, size_t frlen);
ErrorStatus Write_FRAM(uint8_t *addr_from, uint32_t fram_addr, size_t frlen);
uint32_t CRC32(uint8_t * addr_from, size_t len, uint32_t initCRC);

#endif
