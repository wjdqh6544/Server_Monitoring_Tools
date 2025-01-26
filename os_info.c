#include "common.h"
#include "os_info.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/statvfs.h>

void get_MountPath_from_FileSystem(const char* fileSystem, char*** mountPath, short* mountPathCnt) { // Get the mount path using fileSystem path of Disk.
    FILE* lsblk_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' }, originPath[5] = { '\0' }, mountBuf[MAX_MOUNTPATH_LEN] = { '\0' };
    int mountIdx = 0;
    // char* targetPos = NULL;

    for (int i = 5; fileSystem[i] != '\0'; i++) {
        originPath[i - 5] = fileSystem[i];
        originPath[i - 4] = '\0';
    }

    if ((lsblk_ptr = popen(GET_MOUNTPATH_LSBLK, "r")) == NULL) {
        exception(-2, "get_MountPath_from_FileSystem", "lsblk - mountPathCnt");
        return;
    }

    *mountPathCnt = 0;

    while (fgets(lineBuf, sizeof(lineBuf), lsblk_ptr) != NULL) { // Get the number of child filesystem. (= partition)
        if (strstr(lineBuf, originPath) != NULL){
            do {
                fgets(lineBuf, sizeof(lineBuf), lsblk_ptr);
                (*mountPathCnt)++;
            } while (isalpha(lineBuf[0]) == 0);
            (*mountPathCnt)--;
            break;
        }
    }

    pclose(lsblk_ptr);
    
    if (*mountPath != NULL) {
        free(*mountPath);
        *mountPath = NULL;
    }

    *mountPath = (char**)malloc((*mountPathCnt) * sizeof(char*)); // Allocate 2D-Array for mountPath

    if ((lsblk_ptr = popen(GET_MOUNTPATH_LSBLK, "r")) == NULL) {
        exception(-2, "get_MountPath_from_FileSystem", "lsblk - mountPath");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), lsblk_ptr) != NULL) {
        if (strstr(lineBuf, originPath) != NULL) {
            while (fgets(lineBuf, sizeof(lineBuf), lsblk_ptr) != NULL) {
                if (isalpha(lineBuf[0]) == 0) {
                    sscanf(lineBuf, "%*s %*s %*s %*s %*s %*s %s", mountBuf);
                    (*mountPath)[mountIdx] = (char*)malloc(strlen(mountBuf) * sizeof(char));
                    strcpy((*mountPath)[mountIdx++], mountBuf);
                }
                
                if (mountIdx == (*mountPathCnt)) {
                    break;
                }
            }    
        }

    }
    
    return;
}

int get_Partition_Information(PartitionInfo** partition_list_ptr, short* partition_cnt){ // Get the partition list and capacity of each partitions.
    // partition_list_ptr: Must be 'not' allocated to memory. (At this function, partition list is allocated to memory.)
    // So, Caller invokes this function with pointer that initialized with "NULL"
    FILE* fp = NULL;
    int cnt = 0;
    char fileSystem[MAX_FILESYSTEM_LEN] = { '\0' };
    char mountPath[MAX_MOUNTPATH_LEN] = { '\0' };

    if ((fp = fopen(MOUNTS_LOCATION, "r")) == NULL) { // Open /proc/mounts
        exception(-1, "get_Partition_List", MOUNTS_LOCATION);
        return -100;
    }

    while (fscanf(fp, MOUNTS_FORM, fileSystem, mountPath) != EOF) { // For counting the number of partitions.
        if ((strncmp("/dev", fileSystem, 4) != 0) && (strncmp("tmpfs", fileSystem, 5) != 0)) { // For filtering partitions
            continue;
        }

        if (strncmp("/dev/loop", fileSystem, strlen("/dev/loop")) == 0) {
            continue;
        }
        cnt++;
    }

    if (*partition_list_ptr != NULL){ // Create Array for partition information.
        free(*partition_list_ptr); // Free the memory allocated for the existing array.
        *partition_list_ptr = NULL;
    }

    *partition_list_ptr = (PartitionInfo*)calloc(cnt, sizeof(PartitionInfo)); // Allocate new memory.
    fseek(fp, 0, SEEK_SET); // Move pointer to head of file.

    for (int i = 0; fscanf(fp, MOUNTS_FORM, fileSystem, mountPath) != EOF; i++) { // For extracting fileSystem and mountPath of each partitions.
        if ((strncmp("/dev", fileSystem, 4) != 0) && (strncmp("tmpfs", fileSystem, 5) != 0)) { // For filtering partitions
            i--;
            continue;
        }

        if (strncmp("/dev/loop", fileSystem, strlen("/dev/loop")) == 0) {
            i--;
            continue;
        }
        
        strcpy((*partition_list_ptr)[i].fileSystem, fileSystem);
        strcpy((*partition_list_ptr)[i].mountPath, mountPath);
        get_Partition_Usage(&((*partition_list_ptr)[i]));
    }

    (*partition_cnt) = cnt;

    return 0;

}

void get_Partition_Usage(PartitionInfo* partitionBuf){
    struct statvfs statBuf;
    char errorBuf[ERROR_MSG_LEN] = { '\0' };
    
    if (statvfs(partitionBuf->mountPath, &statBuf) == -1) { // Get capacity information of partition
        partitionBuf->spaceTotal = 0;
        partitionBuf->spaceUse = 0;
        strcpy(errorBuf, "statvfs - ");
        strcat(errorBuf, partitionBuf->mountPath);
        exception(-4, "get_Partition_Usage", errorBuf);
        return;
    }

    partitionBuf->spaceTotal = (size_t)(statBuf.f_blocks * statBuf.f_frsize) / 1024; // Unit: KB
    partitionBuf->spaceUse = (size_t)(statBuf.f_bfree * statBuf.f_frsize) / 1024; // Unit: KB, Free space
    partitionBuf->spaceUse = (partitionBuf->spaceTotal) - (partitionBuf->spaceUse); // Calculate Used space
}