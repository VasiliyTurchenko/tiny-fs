/** @file testhelpers.c
 *  @brief some test helpers
 *  @author turchenkov@gmail.com
 *  @bug
 *  @date 08-Mar-2019
 */

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "testhelpers.h"

/**
 * @brief TestHeader prints out the test name
 * @param testName
 */
void TestHeader(char * testName)
{
	printf("\nStarting test %s ...\n", testName);
}

/**
 * @brief TestFooter prints out the test name
 * @param testName
 */
void TestFooter(char * testName)
{
	printf("Test %s finished\n", testName);
}

