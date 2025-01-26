#include "0_usrDefine.h"
#include "common.h"
#include "hw_info.h"
#include "info_from_log.h"
#include "os_info.h"
#include "zz_struct.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

DateInfo dateBuf;
const Unit_Mapping unitMap[] = {
    { KB, "KB" },
    { MB, "MB" },
    { GB, "GB" },
    { TB, "TB" },
    { PB, "PB" },
    { EB, "EB" }
};

int main(void){
    VDInfo* vdBuf;
    int vdCnt; 
    get_Date();
    get_VDisk_Information_from_Perccli(&vdBuf, &vdCnt);
    for (int i = 0; i < vdCnt; i++) {
        printf("%hd %hd %s %hd %hd %s %s %s %hd\n", (vdBuf + i)->driveGroup, (vdBuf + i)->virtualDrive, (vdBuf + i)->type, (vdBuf + i)->status, (vdBuf + i)->access, (vdBuf + i)->capacity, 
        (vdBuf + i)->vdName, (vdBuf + i)->fileSystem, (vdBuf + i)->mountPathCnt);
        for (int j = 0; j < (vdBuf + i)->mountPathCnt; j++){
            printf("%s\n", (vdBuf + i)->mountPath[j]);
        }
    }
}