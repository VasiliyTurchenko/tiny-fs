/** @file test_f_checkfs.c
 *  @brief f_CheckFS test
 *  @author turchenkov@gmail.com
 *  @bug
 *  @date 08-Mar-2019
 */

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "testhelpers.h"
#include "mock.h"
#include "tiny-fs.h"
#include "ls.h"

#include "test_f_checkfs.h"

#ifdef DEBUG

static Media_Desc_t media = {
	.readFunc = Read_FRAM,
	.writeFunc = Write_FRAM,
	.MediaSize = FRAM_SIZE,
	.mode = MEDIA_RW,
};
#endif

void TEST_f_checkFS(void)
{
	Format(&media);
	char * testName = "TEST_f_checkFS()";
	TestHeader(testName);

	size_t testNum = 1U;

	/* good formatted media */

	FRESULT res = f_checkFS(&media);

	if (res == FR_OK) {
		TEST_PASSED(testNum);
	} else {
		TEST_FAILED(testNum, FRESULT_String(res));
	}

	/* spoiled $$FAT$$ name */
	testNum++;
	char * badname = "$$FOT$$";
	Write_FRAM((uint8_t*)badname, offsetof(FAT_Begin_t, entry0.FileName), (strlen(badname) + 1U));

	res = f_checkFS(&media);

	if (res == FR_NO_FILE) {
		TEST_PASSED(testNum);
	} else {
		TEST_FAILED(testNum, FRESULT_String(res));
	}

	Format(&media);
	/* spoiled $$FAT$$ file status */
	testNum++;
	fState_t badstate = FStateClosed;
	Write_FRAM((uint8_t*)&badstate, offsetof(FAT_Begin_t, entry0.FileStatus), sizeof (fState_t));

	res = f_checkFS(&media);

	if (res == FR_NO_FILE) {
		TEST_PASSED(testNum);
	} else {
		TEST_FAILED(testNum, FRESULT_String(res));
	}

	/* NULL pointer */
	testNum++;
	res = f_checkFS(NULL);

	if (res == FR_INVALID_OBJECT) {
		TEST_PASSED(testNum);
	} else {
		TEST_FAILED(testNum, FRESULT_String(res));
	}

	Format(&media);
	/* bad CRC */
	testNum++;

	uint32_t badCRC = 0U;
	Write_FRAM((uint8_t*)&badCRC, offsetof(FAT_Begin_t, h.DIR_CRC32), sizeof (uint32_t));

	res = f_checkFS(&media);

	if (res == FR_NO_FILESYSTEM) {
		TEST_PASSED(testNum);
	} else {
		TEST_FAILED(testNum, FRESULT_String(res));
	}

	TestFooter(testName);
}
