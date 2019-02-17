/** @file TEST_mediaIO.c
 *  @brief
 *
 *  @author turchenkov@gmail.com
 *  @bug
 *  @date 17-Feb-2019
 */

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#include "test_macros.h"
#include "tiny-fs.h"
#include "TEST_mediaIO.h"
#include "ls.h"

/* work buffers */
static uint8_t b1[FRAM_SIZE];
static uint8_t b2[FRAM_SIZE];

typedef struct testCase_mediaIO {
	char *fName;
	size_t fSize;
	size_t dataSize;
	uint8_t *workBuf;
	uint8_t *correctBuf;
	FRESULT expectedNew;
	FRESULT expectedW;
	FRESULT expectedR;
	FRESULT expectedC;
} testCase_mediaIO_t;

/* test file by file */
static testCase_mediaIO_t *newCase(void);
static void prepareBuffers(size_t dataSize);

void ProcTestResult(char * filename, int linenum, bool res, const char * msg, size_t testnum);

static testCase_mediaIO_t cases[] = {

	{ .fName = "0001", .fSize = 1000U, .dataSize = 1000U, (uint8_t *)&b1, (uint8_t *)&b2, FR_OK, FR_OK, FR_OK, FR_OK },
	{ .fName = "0000", .fSize = 0000U, .dataSize = 0000U, (uint8_t *)&b1, (uint8_t *)&b2, FR_OK, FR_OK, FR_OK, FR_OK },
//	{ .fName = "0000", .fSize = 2000U, .dataSize = 16U, (uint8_t *)&b1, (uint8_t *)&b2, FR_OK, FR_OK, FR_OK, FR_OK },
//	{ .fName = "0002", .fSize = 100U, .dataSize = 16U, (uint8_t *)&b1, (uint8_t *)&b2, FR_OK, FR_OK, FR_OK, FR_OK },
//	{ .fName = "5320", .fSize = 3320, .dataSize = 3320U, (uint8_t *)&b1, (uint8_t *)&b2, FR_OK, FR_OK, FR_OK, FR_OK },
//	{ .fName = "993", .fSize = 993, .dataSize = 993U, (uint8_t *)&b1, (uint8_t *)&b2, FR_OK, FR_OK, FR_OK, FR_OK },
//	{ .fName = "10", .fSize = 10, .dataSize = 10U, (uint8_t *)&b1, (uint8_t *)&b2, FR_OK, FR_OK, FR_OK, FR_OK },
//	{ .fName = "11", .fSize = 40, .dataSize = 40U, (uint8_t *)&b1, (uint8_t *)&b2, FR_OK, FR_OK, FR_OK, FR_OK },
};

/**
 * @brief TEST_fileIO
 * @param media
 */
void TEST_fileIO(Media_Desc_t *media)
{
	time_t t;
	srand((uint32_t)time(&t));


	fHandle_t filehandle;
	testCase_mediaIO_t *p;
	size_t i = 0U;
	const char * badDataSize = "# of bytes is wrong\n";
	const char * memcmpErr = "data comparison error!\n";
	const char * ftellErr = "f_tell() error.\n";

	while ((p = newCase()) != NULL) {
		memset(&filehandle, 0, sizeof(fHandle_t));
		filehandle.media = media;
		prepareBuffers(p->dataSize);

		FRESULT res = FR_OK;

		res = NewFile(&filehandle, p->fName, p->fSize, FModeWrite);
		ProcTestResult(__FILE__, __LINE__, (res == p->expectedNew), FRESULT_String(res), i);

		size_t bw = 0U;
		res = f_write(&filehandle, &b1, p->dataSize, &bw);
		ProcTestResult(__FILE__, __LINE__, (res == p->expectedW), FRESULT_String(res), i);
		ProcTestResult(__FILE__, __LINE__, (bw == p->dataSize), badDataSize, i);

		/* lseek and f_tell */
		ProcTestResult(__FILE__, __LINE__, (f_tell(&filehandle) == p->dataSize ), ftellErr, i);

		res = f_rewind(&filehandle);
		ProcTestResult(__FILE__, __LINE__, (f_tell(&filehandle) == 0U ), ftellErr, i);

		res = CloseFile(&filehandle);
		ProcTestResult(__FILE__, __LINE__, (res == p->expectedC), FRESULT_String(res), i);

		memset(&filehandle, 0, sizeof(fHandle_t));
		filehandle.media = media;
		res = NewFile(&filehandle, p->fName, p->fSize, FModeRead);
		ProcTestResult(__FILE__, __LINE__, (res == p->expectedNew), FRESULT_String(res), i);

		size_t br = 0U;
		res = f_read(&filehandle, &b2, p->dataSize, &br);
		ProcTestResult(__FILE__, __LINE__, (res == p->expectedR), FRESULT_String(res), i);
		ProcTestResult(__FILE__, __LINE__, (br == p->dataSize), badDataSize, i);

		/* read behind the file end */

		res = f_read(&filehandle, &b2, 0U, &br);
		ProcTestResult(__FILE__, __LINE__, (res == FR_OK), FRESULT_String(res), i);
		ProcTestResult(__FILE__, __LINE__, (br == 0), badDataSize, i);

		int cres = memcmp(&b1, &b2, p->dataSize);
		ProcTestResult(__FILE__, __LINE__, (cres == 0), memcmpErr, i);

		res = CloseFile(&filehandle);
		ProcTestResult(__FILE__, __LINE__, (res == p->expectedC), FRESULT_String(res), i);

		i++;
	}
}

/**
 * @brief ProcTestResult
 * @param filename
 * @param linenum
 * @param res
 * @param msg
 * @param testnum
 */
void ProcTestResult(char * filename, int linenum, bool res, const char * msg, size_t testnum)
{
	if (res == false) {
		printf("Test #%d failed at line %d, file %s with result = %s.\n", testnum, linenum, filename, msg);
		exit(-1);
	} else {
		printf("Test #%d passed at line %d, file %s.\n", testnum, linenum, filename);
	}
}


/**
 * @brief newCase
 * @return
 */
static testCase_mediaIO_t *newCase(void)
{
	static size_t c = 0U;
	while (c < (sizeof(cases) / sizeof(cases[0]))) {
		c++;
		return &cases[c - 1U];
	}
	return NULL;
}

/**
 * @brief prepareBuffers
 * @param dataSize
 */
static void prepareBuffers(size_t dataSize)
{
	memset(&b1, 0, dataSize);
	for (size_t i = 0U; (i < dataSize) && (i < FRAM_SIZE); i++) {
		b1[i] = (uint8_t)(rand() % UINT8_MAX);
	}
	memset(&b2, 0xffU, dataSize);
}
