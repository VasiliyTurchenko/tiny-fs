/** @file test_macros.h
 *  @brief
 *
 *  @author
 *  @bug
 *  @date 17-Feb-2019
 */

#ifndef TEST_MACROS_H
#define TEST_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdio.h>

#define TEST_FAILED(X, Y)                                                                          \
	do {                                                                                       \
		printf("Test #%zu failed at line %d, file %s with RESULT = %s.\n", (X),       \
		       __LINE__, __FILE__, (Y));                                                        \
	} while (0);
#define TEST_PASSED(X)                                                                             \
	do {                                                                                       \
		printf("Test #%zu passed.\n", (X));                                                 \
	} while (0);

#ifdef __cplusplus
}
#endif

#endif // TEST_MACROS_H
