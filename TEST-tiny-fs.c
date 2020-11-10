/** @file TEST-tiny-fs.c
 *  @brief
 *
 *  @author turchenkov@gmail.com
 *  @bug
 *  @date 26-Jan-2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "testhelpers.h"

#include "TEST-tiny-fs.h"
#include "TEST_mediaIO.h"
#include "random_test.h"

#include "test_f_checkfs.h"

#include "test_ascii_helpers.h"

#include "ls.h"
#include "tiny-fs.h"
#include "mock.h"

#ifdef DEBUG

static ErrorStatus TEST_FRAM_Write(void)
{
	ErrorStatus retVal = SUCCESS;

	static uint8_t DTA[FRAM_SIZE];  /* data transfer area for test */
	static uint8_t DTA1[FRAM_SIZE]; /* data transfer area for test */
	time_t t;

	srand((uint32_t)time(&t));

	/* test bad args */

	if (Read_FRAM(DTA, 0U, 0U) != ERROR) {
		printf("Test failed: frlen = 0 passed at line %d\n", __LINE__);
	}

	if (Write_FRAM(DTA, 0U, 0U) != ERROR) {
		printf("Test failed: frlen = 0 passed at line %d\n", __LINE__);
	}

	if (Read_FRAM(NULL, 0U, 1U) != ERROR) {
		printf("Test failed: NULL pointer passed at line %d\n",
		       __LINE__);
	}

	if (Write_FRAM(NULL, 0U, 1U) != ERROR) {
		printf("Test failed: NULL pointer passed at line %d\n",
		       __LINE__);
	}

	if (Read_FRAM(DTA, FRAM_SIZE - 1, 2U) != ERROR) {
		printf("Test failed: FRAM location above FRAM size passedat line %d\n",
		       __LINE__);
	}

	if (Write_FRAM(DTA, FRAM_SIZE - 1, 2U) != ERROR) {
		printf("Test failed: FRAM location above FRAM size passedat line %d\n",
		       __LINE__);
	}

	for (size_t i = 0U; i < FRAM_SIZE; i++) {
		DTA[i] = (rand() % 0xFF);
		DTA1[i] = DTA[i];
	}

	if (Write_FRAM(DTA, 0U, FRAM_SIZE) != SUCCESS) {
		printf("Test failed at line %d\n", __LINE__);
	}

	for (size_t i = 0U; i < FRAM_SIZE; i++) {
		DTA[i] = 0U;
	}

	if (Read_FRAM(DTA, 0U, FRAM_SIZE) != SUCCESS) {
		printf("Test failed at line %d\n", __LINE__);
	}

	for (size_t i = 0U; i < FRAM_SIZE; i++) {
		if (DTA[i] != DTA1[i]) {
			printf("Test failed at FRAM addr.: %zu (line %d)\n", i,
			       __LINE__);
		};
	}

	return retVal;
}

static ErrorStatus TEST_Format(void)
{
	Media_Desc_t testMedia;
	testMedia.readFunc = Read_FRAM;
	testMedia.writeFunc = Write_FRAM;
	testMedia.MediaSize = FRAM_SIZE;
	return (Format(&testMedia));
}

static void TEST_FindEntry()
{
	Media_Desc_t testMedia;
	testMedia.readFunc = Read_FRAM;
	testMedia.writeFunc = Write_FRAM;
	testMedia.MediaSize = FRAM_SIZE;

	DIR_Entry_t entry;
	const char *f = "FILE00";
	strcpy(entry.FileName, f);

	uint32_t findResult;
	findResult = findEntry(&testMedia, &entry);
}

#if(0)
static void TEST_getBitMask(void)
{
	for (uint8_t i = 0U; i < 10U; i++) {
		printf("TEST_getBitMask %d -> %d \n", i, getBitMask(i));
	}
}
#endif

typedef struct {
	size_t fatSize;
	uint8_t fillPattern;
	size_t holeSize;
	size_t holeOffset;
	size_t requested;
	size_t correctAnswer;

} alloc_testCase_t;

static alloc_testCase_t testCases[] = {
	{ .fatSize = 10U, // too big
	  .fillPattern = 0xFFU,
	  .holeSize = 0U,
	  .holeOffset = 1U,
	  .requested = 10U * FS_CLUSTER_SIZE,
	  .correctAnswer = UINT32_MAX },
	{ .fatSize = 10U, // zero byte rquest
	  .fillPattern = 0xFFU,
	  .holeSize = 0U,
	  .holeOffset = 1U,
	  .requested = 0U * FS_CLUSTER_SIZE,
	  .correctAnswer = 0U },
	{ .fatSize = 10U, // 64 bytes request
	  .fillPattern = 0x0FU,
	  .holeSize = 0U,
	  .holeOffset = 0U,
	  .requested = 4U * FS_CLUSTER_SIZE,
	  .correctAnswer = 4U * FS_CLUSTER_SIZE },
	{ .fatSize = 100U, // 127 bytes request
	  .fillPattern = 0xAAU,
	  .holeSize = 1U,
	  .holeOffset = 98U,
	  .requested = (8U * FS_CLUSTER_SIZE) - 1U,
	  .correctAnswer = 98U * FS_CLUSTER_SIZE * CHAR_BIT },
	{ .fatSize = 100U, // 129 bytes request
	  .fillPattern = 0xFFU,
	  .holeSize = 1U,
	  .holeOffset = 99U,
	  .requested = (8U * FS_CLUSTER_SIZE) + 1U,
	  .correctAnswer = UINT32_MAX },

};

static void TEST_allocateClusters(void)
{
	const size_t nTests = sizeof(testCases) / sizeof(testCases[0]);
	for (size_t i = 0U; i < nTests; i++) {
		uint8_t *fat;
		uint8_t *fat_etalon;

		fat = malloc(testCases[i].fatSize);
		fat_etalon = malloc(testCases[i].fatSize);

		memset(fat, testCases[i].fillPattern, testCases[i].fatSize);
		memset(&fat[testCases[i].holeOffset], 0x00U,
		       testCases[i].holeSize);

		memcpy(fat_etalon, fat, testCases[i].fatSize);

		printf("allocateClusters test #%zu ...", i);

		size_t res = allocateClusters(fat, testCases[i].fatSize,
					      testCases[i].requested);
		if (res == testCases[i].correctAnswer) {
			printf("\tpassed.");
		} else {
			printf("\tfailed. Got %zu instead of %zu", res,
			       testCases[i].correctAnswer);
		}
		if (memcmp(fat_etalon, fat, testCases[i].fatSize) == 0) {
			printf("\tClusters were allocated correctly.\n");
		} else {
			printf("\tClusters were allocated with error.\n");
		}

		free(fat);
		free(fat_etalon);
	}
}



static	Media_Desc_t media = {
	.readFunc = Read_FRAM,
	.writeFunc = Write_FRAM,
	.MediaSize = FRAM_SIZE,
	.mode = MEDIA_RW,
	};


#endif

void Run_TEST(void)
{
	InitFS();
#ifdef DEBUG
#if(0)
	printf("\nTEST FRAM_Write()/FRAM_Read() started....\n");
	printf("\t");
	if (TEST_FRAM_Write() == ERROR) {
		printf("...failed\n");
	} else {
		printf("...passed\n");
	}

	printf("\nTEST getBitMask() started ....\n");
	TEST_getBitMask();

	printf("\nTEST allocateClusters() started ....\n");
	TEST_allocateClusters();
#endif

	printf("\nTEST Format() started....\n");

	if (TEST_Format() == ERROR) {
		printf("...failed\n");
	} else {
		printf("...passed\n");
	}
	ls(&media);

	TEST_fileIO(&media);

	ls(&media);

	TEST_f_checkFS();

#if(1)
	RandomTest();
#endif

#if(0)
	printf("\nTEST findEntry() started....\n");
	TEST_FindEntry();
#endif

#endif

	TEST_ascii_helpers();

}
