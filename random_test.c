/** @file random_test.c
 *  @brief
 *
 *  @author turchenkov@gmail.com
 *  @bug
 *  @date 03-Feb-2019
 */

#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "mock.h"
#include "tiny-fs.h"
#include "ls.h"

#define CASES_ARR_LEN (DIR_ENTRIES / 2U)

#define PANIC                                                                                      \
	do {                                                                                       \
		printf("Panic at %s, %d \n", __FILE__, __LINE__);                                  \
		exit(-1);                                                                          \
	} while (0)

#ifdef DEBUG

static Media_Desc_t testMedia;

/** @brief expected result */
typedef enum {
	EXP_NO_RESULT = 0U,
	EXP_NEW_OK,
	EXP_NEW_ERR,
	EXP_CLOSE_OK,
	EXP_CLOSE_ERR,
	EXP_DEL_OK,
	EXP_DEL_ERR
} expected_result_t;

/**
 * @brief expected_result_String
 * @param res
 * @return pointer to const string
 */
const char *expected_result_String(expected_result_t res)
{
	const char *retVal;
	static const char *no_res = "EXP_NO_RESULT";
	static const char *new_ok = "EXP_NEW_OK";
	static const char *new_err = "EXP_NEW_ERR";
	static const char *cl_ok = "EXP_CLOSE_OK";
	static const char *cl_err = "EXP_CLOSE_ERR";
	static const char *del_ok = "EXP_DEL_OK";
	static const char *del_err = "EXP_DEL_ERR";
	static const char *bad = "<bad enum>";

	switch (res) {
	case (EXP_NO_RESULT): {
		retVal = no_res;
		break;
	}
	case (EXP_NEW_OK): {
		retVal = new_ok;
		break;
	}
	case (EXP_NEW_ERR): {
		retVal = new_err;
		break;
	}
	case (EXP_CLOSE_OK): {
		retVal = cl_ok;
		break;
	}
	case (EXP_CLOSE_ERR): {
		retVal = cl_err;
		break;
	}
	case (EXP_DEL_OK): {
		retVal = del_ok;
		break;
	}
	case (EXP_DEL_ERR): {
		retVal = del_err;
		break;
	}
	default: {
		retVal = bad;
		break;
	}
	}
	return retVal;
}

/**
 * @brief bool_String
 * @param res
 * @return
 */
const char *bool_String(bool res)
{
	static const char *tru = "TRUE";
	static const char *fal = "FALSE";

	if (res == true) {
		return tru;
	}
	return fal;
}

/** @brief what to do at the next step */
typedef enum { NOTHING = 0U, NEW, CLOSE, DEL } action_t;

/**
 * @brief test case
 */
typedef struct {
	char *name;
	size_t size;
	fHandle_t handle;
	expected_result_t expected;
	bool file_exists;
	bool file_opened;
} test_case_t;

typedef test_case_t *test_case_p;

/** @brief oracle */
typedef struct {
	size_t clusters_total;
	size_t clusters_left;
	size_t entries_left;
} oracle_t;

/* test cases array */
static test_case_t test_cases[CASES_ARR_LEN];

/* oracle */

static oracle_t oracle;

/**
 * @brief getRandomName
 * @param maxlen
 * @return
 */
static char *getRandomName(size_t maxlen)
{
	char *retVal = calloc(maxlen + 1U, sizeof(char));
	if (retVal == NULL) {
		PANIC;
	}
	for (size_t i = 0U; i < maxlen; i++) {
		retVal[i] = 0x40 + rand() % 27;
	}
	retVal[maxlen] = '\0';
	return retVal;
}

/**
 * @brief getAction returns random new action
 * @return
 */
static action_t getAction(void)
{
	uint8_t a = (uint8_t)((uint32_t)rand()) % 4U;
	if (a == 1U)
		return NEW;
	if (a == 2U)
		return CLOSE;
	if (a == 3U)
		return DEL;
	return NOTHING;
}

/**
 * @brief init_testCase initializes test case
 * @param pCase
 */
static void init_testCase(test_case_p pCase)
{
	if (pCase->file_exists != true) {
		if (pCase->name != NULL) {
			free(pCase->name);
		}
		pCase->name = getRandomName(MAX_FILENAME_LEN);
		pCase->size = ((uint32_t)(rand()) % UINT16_MAX) >> 2U;
		pCase->expected = EXP_NO_RESULT;
		pCase->file_exists = false;
		pCase->handle.media = &testMedia;
		pCase->file_opened = false;
		pCase->handle.pmutex = NULL;
	}
}

/**
 * @brief findEmptySlot
 * @param case0 pointer to the first test case in the array
 * @return index of empty slot
 */
size_t findEmptySlot(test_case_p case0)
{
	size_t retVal;
	retVal = UINT32_MAX;

	for (size_t i = 0U; i < CASES_ARR_LEN; i++) {
		if (case0[i].file_exists == false) {
			memset(&case0[i], 0U, MAX_FILENAME_LEN);
			retVal = i;
			break;
		} else {
			continue;
		}
	}
	return retVal;
}

/**
 * @brief selectFileToClose randomly selects a file to be closed
 * @param case0
 * @return
 */
size_t selectFileToClose(test_case_p case0)
{
	size_t retVal;
	retVal = (uint32_t)rand() % CASES_ARR_LEN;
	if (case0[retVal].file_exists == false) {
		/* non-existing file */
		init_testCase(&case0[retVal]);
		case0->expected = EXP_CLOSE_ERR;
	}
	return retVal;
}

/**
 * @brief selectFileToDelete randomly selects a file to be deleted
 * @param case0
 * @return
 */
size_t selectFileToDelete(test_case_p case0)
{
	size_t retVal;
	retVal = (uint32_t)rand() % CASES_ARR_LEN;
	if (case0[retVal].file_exists == false) {
		/* non-existing file */
		init_testCase(&case0[retVal]);
		case0->expected = EXP_DEL_ERR;
	}
	return retVal;
}

/**
 * @brief init_oracle
 * @param media
 */
static void init_oracle(Media_Desc_p media)
{
	oracle.clusters_total = getNumClusters(media);
	oracle.clusters_total -= getClusterFileSize(getClusterTableSize(media)) / FS_CLUSTER_SIZE;
	oracle.clusters_left = oracle.clusters_total;
	oracle.entries_left = (DIR_ENTRIES - 1U);
}

/**
 * @brief run_oracle returns correct result of the next action
 * @param tcase
 * @param action
 * @return
 */
expected_result_t run_oracle(Media_Desc_p media, test_case_p tcase, action_t action)
{
	size_t fsize_in_clusters = tcase->size / FS_CLUSTER_SIZE;

	if ((tcase->size % FS_CLUSTER_SIZE) > 0U) {
		fsize_in_clusters++;
	}

	expected_result_t result = EXP_NO_RESULT;
	switch (action) {
	case (NEW): {
		/* if there is already opened file we get an error */
		for (size_t i = 0U; i < CASES_ARR_LEN; i++) {
			if ((test_cases[i].file_exists == true) &&
			    (test_cases[i].file_opened == true)) {
				result = EXP_NEW_ERR;
				printf("EXP_NEW_ERR because of there is an already opened file\n");
				break;
			}
		}
		if (result == EXP_NEW_ERR) {
			break;
		}
		/*if file already exist we expect an error */
		for (size_t i = 0U; i < CASES_ARR_LEN; i++) {
			if (test_cases[i].file_exists == true) {
				if (strcmp(tcase->name, test_cases[i].name) == 0) {
					result = EXP_NEW_ERR;
					printf("EXP_NEW_ERR because of file already exists\n");
					break;
				}
			}
		}
		if (result == EXP_NEW_ERR) {
			break;
		}
		/* if file size in clusters > avail. clusters we expect error */
		if (fsize_in_clusters > oracle.clusters_left) {
			printf("EXP_NEW_ERR because of no more free clusters\n");
			result = EXP_NEW_ERR;
			break;
		}

		/* if file size in clusters > max avail contigous free space  we expect error */
		if ((fsize_in_clusters * FS_CLUSTER_SIZE) > findMaxFreeBlock(media)) {
			printf("EXP_NEW_ERR because of no free clusters chain with needed size\n");
			result = EXP_NEW_ERR;
			break;
		}

		/* if dir entries list is full we expect error */
		if (oracle.entries_left == 0U) {
			printf("EXP_NEW_ERR because of no more DIR entries\n");
			result = EXP_NEW_ERR;
			break;
		}

		/* otherwize we expect success */
		oracle.entries_left--;
		oracle.clusters_left -= fsize_in_clusters;
		printf("EXP_NEW_OK\n");
		result = EXP_NEW_OK;
		break;
	}
	case (CLOSE): {
		if ((tcase->file_exists == true) && (tcase->file_opened == true)) {
			if ((tcase->handle.fileDir.FileStatus == FStateOpenedR) ||
			    (tcase->handle.fileDir.FileStatus == FStateOpenedW)) {
				/* otherwize we expect success */
				printf("EXP_CLOSE_OK\n");
				result = EXP_CLOSE_OK;
				break;
			} else {
				printf("EXP_CLOSE_ERR because the file is not opened\n");
				result = EXP_CLOSE_ERR;
				break;
			}
		} else {
			printf("EXP_CLOSE_ERR because the file does not exist\n");
			result = EXP_CLOSE_ERR;
			break;
		}
	}
	case (DEL): {
		if (tcase->file_exists == true) {
			if (tcase->file_opened == false) {
				printf("EXP_DEL_OK\n");
				result = EXP_DEL_OK;
				oracle.entries_left++;
				oracle.clusters_left += fsize_in_clusters;
				break;
			} else {
				printf("EXP_DEL_ERR because the file is not closed\n");
				result = EXP_DEL_ERR;
				break;
			}
		} else {
			printf("EXP_DEL_ERR because the file does not exist\n");
			result = EXP_DEL_ERR;
			break;
		}
	}
	default:
		break;
	}
	return result;
}

/**
 * @brief printTestCases prints test cases table
 */
static void printTestCases(void)
{
	for (size_t i = 0U; i < CASES_ARR_LEN; i++) {
		printf("Test case %zu:\t%s\t%zu\t%s", i, test_cases[i].name, test_cases[i].size,
		       test_cases[i].handle.fileDir.FileName);
		printf("\texpected: %s", expected_result_String(test_cases[i].expected));
		printf("\tfile exists:  %s", bool_String(test_cases[i].file_exists));
		printf("\tfile opened: %s\n", bool_String(test_cases[i].file_opened));
	}
}

/**
 * @brief RandomTest performs random tests
 */
void RandomTest(void)
{
	testMedia.readFunc = Read_FRAM;
	testMedia.writeFunc = Write_FRAM;
	testMedia.MediaSize = FRAM_SIZE;
	testMedia.mode = MEDIA_RW;
	if (Format(&testMedia) == ERROR) {
		PANIC;
	}
	//ls(&testMedia);
	init_oracle(&testMedia);
	time_t t;
	srand((uint32_t)time(&t));
	size_t tcase;
	size_t count = 0U;
	size_t newcount = 0;
	while (1) {
		expected_result_t oracle_result = EXP_NO_RESULT;
		switch (getAction()) {
		case (NEW): {
			tcase = findEmptySlot(
				&test_cases[0]); /* may return the slot with existing file! */
			if (tcase == UINT32_MAX) {
				PANIC;
			}
			init_testCase(&test_cases[tcase]);
			printf("RandomTest: trying to create new file \"%s\" with size %zu\n",
			       test_cases[tcase].name, test_cases[tcase].size);

			oracle_result = run_oracle(&testMedia, &test_cases[tcase], NEW);
			FRESULT res;
			if (oracle_result == EXP_NEW_OK) {
				newcount++;
				fflush(stdout);
			}
			res = NewFile(&test_cases[tcase].handle, test_cases[tcase].name,
				      test_cases[tcase].size, FModeRead);
			printf("NewFile result = %s\n", FRESULT_String(res));
			if (res != FR_OK) {
				/* no file was created or opened */
				if (oracle_result != EXP_NEW_ERR) {
					/* error */
					printf("%s, %zu\n", test_cases[tcase].name,
					       test_cases[tcase].size);
					printf("Current state of the filesystem (ls output):");
					ls(&testMedia);
					printf("total runs count = %zu\n", count);
					printf("NewFile() count= %zu\n", newcount);

					printf("Test cases array :\n");
					printTestCases();
					printf("\n");

					PANIC;
				} else {
					/*no error */
					printf("Test passed\n");

					if (test_cases[tcase].file_exists == false) {
						test_cases[tcase].expected = oracle_result;
					} else {
						//
					}
				}
			} else {
				/* file was opened */
				printf("OK for tcase = %zu\n", tcase);
				test_cases[tcase].expected = oracle_result;
				test_cases[tcase].file_exists = true;
				test_cases[tcase].file_opened = true;
			}
			//ls(&testMedia);
			break;
		}
		case (CLOSE): {
			tcase = selectFileToClose(&test_cases[0]);
			printf("RandomTest: trying to close file \"%s\" with size %zu\n",
			       test_cases[tcase].name, test_cases[tcase].size);
			oracle_result = run_oracle(&testMedia, &test_cases[tcase], CLOSE);
			fflush(stdout);
			FRESULT res = CloseFile(&test_cases[tcase].handle);
			printf("CloseFile result = %s\n", FRESULT_String(res));
			if ((res == FR_OK) && (oracle_result == EXP_CLOSE_OK)) {
				printf("Test passed\n");
				test_cases[tcase].expected = oracle_result;
				test_cases[tcase].file_exists = true;
				test_cases[tcase].file_opened = false;
				break;
			}

			if ((res != FR_OK) && (oracle_result == EXP_CLOSE_ERR)) {
				printf("Test passed\n");
			} else {
				printf("Test failed\n");
				printf("\n");
				printTestCases();
				printf("\n");
				PANIC;
			}
			//ls(&testMedia);
			break;
		}
		case (DEL): {
			tcase = selectFileToDelete(&test_cases[0]);
			printf("RandomTest: trying to delete file \"%s\" with size %zu\n",
			       test_cases[tcase].name, test_cases[tcase].size);
			oracle_result = run_oracle(&testMedia, &test_cases[tcase], DEL);
			if (oracle_result == EXP_DEL_OK) {
				printf("trap!\n");
				fflush(stdout);
			}
			FRESULT res = DeleteFile(&testMedia, test_cases[tcase].name);
			printf("DeleteFile result = %s\n", FRESULT_String(res));
			if ((res == FR_OK) && (oracle_result = EXP_DEL_OK)) {
				printf("Test passed\n");
				test_cases[tcase].expected = oracle_result;
				test_cases[tcase].file_exists = false;
				test_cases[tcase].file_opened = false;
				break;
			}
			if ((res != FR_OK) && (oracle_result == EXP_DEL_ERR)) {
				printf("Test passed\n");
			} else {
				printf("Test failed\n");
				printf("\n");
				printTestCases();
				printf("\n");
				PANIC;
			}
			//ls(&testMedia);
			break;
		}
		default:
			break;
		}
		count++;
		fflush(stdout);

		if (count > 1000U) {
			printf("Current state of the filesystem (ls output):");
			ls(&testMedia);
			printf("total runs count = %zu\n", count);
			printf("NewFile() count= %zu\n", newcount);
			break;
		}
	}
	return;
}

#endif
