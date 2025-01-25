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
    DiskInfo* diskBuf = NULL;
    int diskCount; 
    get_Date();
    // get_HBA_Information_from_Perccli(&HBABuf);
    // printf("Status: %hd | Voltage: %s | design: %s | ", HBABuf.bbuStatus.status, HBABuf.bbuStatus.voltage, HBABuf.bbuStatus.designCapacity);
    // printf("remain: %s | full: %s", HBABuf.bbuStatus.remainCapacity, HBABuf.bbuStatus.fullCapacity);
    get_Disk_Information_from_Perccli(&diskBuf, &diskCount);
    for (int i = 0; i < diskCount; i++){
        printf("%hd %hd %hd %hd %hd %s %s %hd %hd %hd\n", diskBuf[i].enclosureNum,   diskBuf[i].slotNum, diskBuf[i].deviceID, diskBuf[i].driveGroup, 
        diskBuf[i].status, diskBuf[i].modelName, diskBuf[i].capacity, diskBuf[i].capUnit, diskBuf[i].mediaType, diskBuf[i].interface);
    }
}