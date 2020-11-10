/** @file main.c
 *  @brief
 *
 *  @author turchenkov@gmail.com
 *  @bug
 *  @date 26-Jan-2019
 */


#include <stdio.h>

#include "mock.h"
#include "tiny-fs.h"

#include "TEST-tiny-fs.h"

int main()
{
	printf("Tests started.\n");
	Run_TEST();

	return 0;
}
