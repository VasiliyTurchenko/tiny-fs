/** @file ls.C
 *  @brief prints out a directory content
 *
 *  @author turchenkov@gmail.com
 *  @bug
 *  @date 03-Feb-2019
 */

#include <stdio.h>
#include <stdlib.h>

#include "tiny-fs.h"


/**
 * @brief ls prints directory content
 */
void ls(Media_Desc_p media)
{
#ifdef DEBUG
	FAT_Header_t header;
	DIR_Entry_t dirEntry;
	if (Read_FRAM((uint8_t *)&header, offsetof(FAT_Begin_t, h),
		      sizeof(FAT_Header_t)) != SUCCESS) {
		printf("Test failed at line %d\n", __LINE__);
		exit(-1);
	}
	printf("\nVolume FS version:%d\tHeader CRC32:%08x\tCluster Table Size:%zu\n",
	       header.FS_version, header.DIR_CRC32,
	       header.FAT_ClusterTableSize);

	size_t busy_clust = 0U;

	printf("FILE\tSTATE\t\tSIZE\tADDRESS\t\tCRC32\n");
	for (size_t i = 0U; i < DIR_ENTRIES; i++) {
		size_t mediaAddr;
		if (i == 0U) {
			mediaAddr = offsetof(FAT_Begin_t, entry0);
		} else {
			mediaAddr = offsetof(FAT_Begin_t, entries) +
				    sizeof(DIR_Entry_t) * (i - 1U);
		}
		if (Read_FRAM((uint8_t *)&dirEntry, mediaAddr,
			      sizeof(DIR_Entry_t)) != SUCCESS) {
			printf("Test failed at line %d\n", __LINE__);
			exit(-1);
		}
		if (dirEntry.FileStatus == FStateNoFile) {
#if(0)
			printf("<empty>\t-----\t----\t--------\t\t--------\n");
#endif
		} else {
			printf("%s\t", dirEntry.FileName);

			switch (dirEntry.FileStatus) {
			case FStateFAT: {
				printf("FAT file\t");
				break;
			}
			case FStateClosed: {
				printf("closed\t\t");
				break;
			}
			case FStateOpenedR: {
				printf("openedR\t\t");
				break;
			}
			case FStateOpenedW: {
				printf("openedW\t\t");
				break;
			}
			default: {
				break;
			}
			}
			printf("%d\t", dirEntry.FileSize);

			size_t clusters_occu;
			clusters_occu = dirEntry.FileSize / FS_CLUSTER_SIZE;
			if ((dirEntry.FileSize % FS_CLUSTER_SIZE) != 0U) {
				clusters_occu++;
			}

			busy_clust += clusters_occu;

			printf("%zu\t\t", dirEntry.FileAddress);
			printf("%08x\n", dirEntry.FileCRC32);
		}
	}

	size_t free_clust;

	free_clust = getNumClusters(media);
	free_clust = free_clust - busy_clust;

	printf("Free space: %zu clusters, %lu bytes\n\n", free_clust, free_clust * FS_CLUSTER_SIZE);
#else
	(void)(media);
#endif
}
