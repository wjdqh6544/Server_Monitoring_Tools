#ifndef STRUCT_H

#define STRUCT_H
#include "0_usrDefine.h"
#include <stdlib.h>

/* commom */
typedef enum UNIT { KB, MB, GB, TB, PB, EB } UNIT;

typedef struct DateInfo {
    short year;
    short month;
    short day;
    short hrs;
    short min;
    short sec;
} DateInfo;

typedef struct Unit_Mapping {
    UNIT unit;
    char* str;
} Unit_Mapping;

/* Parts Information */
typedef struct CPUInfo {
    char name[MAX_PARTS_NAME_LEN];
    short status;
    short coreCnt;
} CPUInfo;

typedef struct MEM_UNIT_INFO {
    short status;
    char connectorName[4];
    char type[MAX_PARTS_NAME_LEN];
    char capacity[MAX_CAPACITY_LEN];
} MEM_UNIT_INFO;

typedef struct MEMInfo {
    short slotsTotal;
    short slotsUsed;
    char installedCapacity[MAX_CAPACITY_LEN];
    char errorCorrection[MAX_PARTS_NAME_LEN];
    MEM_UNIT_INFO* unit;
} MEMInfo;

typedef struct CMOS_BAT_INFO {
    char name[MAX_PARTS_NAME_LEN];
    short status;
} CMOS_BAT_INFO;


typedef struct FANInfo {
    short status;
    char rpm[MAX_CAPACITY_LEN];
    char name[MAX_PARTS_NAME_LEN];
} FANInfo;

typedef struct IFAInfo {
    char name[MAX_PARTS_NAME_LEN];
    char ifName[MAX_PARTS_NAME_LEN];
    short connected;
    char speed[MAX_CAPACITY_LEN];
} IFAInfo;

typedef struct SystemInfo {
    char hostname[MAX_PARTS_NAME_LEN];
    char serverModel[MAX_PARTS_NAME_LEN];
    char serviceTag[MAX_PARTS_NAME_LEN];
    char serviceCode[MAX_PARTS_NAME_LEN];
    CPUInfo cpu[MAX_CPU_COUNT];
    MEMInfo mem;
    CMOS_BAT_INFO cmosBattery;
    FANInfo fan[MAX_FAN_COUNT];
    short ifaCount;
    IFAInfo* ifa;
    short psuStatus[MAX_PSU_COUNT];
} SystemInfo;

/* Disk Information */
typedef struct PartitionInfo {
    char fileSystem[MAX_FILESYSTEM_LEN];
    char mountPath[MAX_MOUNTPATH_LEN];
    size_t spaceTotal;
    size_t spaceUse;
} PartitionInfo;

typedef struct VDInfo {
    short driveGroup;
    short virtualDrive;
    char type[MAX_VDISK_TYPE_LEN];
    short status;
    short access;
    char capacity[MAX_CAPACITY_LEN];
    char vdName[MAX_VDISK_NAME_LEN];
    char fileSystem[MAX_FILESYSTEM_LEN];
    short mountPathCnt;
    char** mountPath;
} VDInfo;

typedef struct DiskInfo {
    short enclosureNum;
    short slotNum;
    short deviceID;
    short driveGroup;
    short status;
    char modelName[MAX_DISK_MODELNAME_LEN];
    char capacity[MAX_CAPACITY_LEN];
    char mediaType[MAX_DISK_MEDIATYPE_LEN];
    char interface[MAX_DISK_INTERFACE_LEN];
    char mappedPartition[MAX_MOUNTPATH_LEN];
} DiskInfo;

typedef struct BBUInfo {
    short status;
    char voltage[MAX_BBU_VOLTAGE_LEN];
    char designCapacity[MAX_BBU_CAPACITY_LEN];
    char fullCapacity[MAX_BBU_CAPACITY_LEN];
    char remainCapacity[MAX_BBU_CAPACITY_LEN];
} BBUInfo;

typedef struct HBAInfo {
    char HBA_Name[MAX_HBA_NAME_LEN];
    char HBA_Bios_Ver[MAX_HBA_BIOS_VER_LEN];
    char HBA_Serial_Num[MAX_HBA_SERIAL_LEN];
    char HBA_Firmware_Ver[MAX_HBA_FIRMWARE_VER_LEN];
    char HBA_Driver_Name[MAX_HBA_DRIVER_NAME_LEN];
    char HBA_Driver_Ver[MAX_HBA_DRIVER_VER_LEN];
    short status;
    short HBA_Cur_Personality;
    short HBA_Drive_Groups_Cnt; 
    BBUInfo bbuStatus;
} HBAInfo;

/* Temp, Usage Information */
typedef struct TempInfo { // Celcius
    short inlet;
    short exhaust;
    short cpu[MAX_CPU_COUNT];
    short raidCore;
    short raidController;
    short bbu;
    short storage_cnt;
    short storage[MAX_STORAGE_COUNT];
} TempInfo;

typedef struct CpuUsage { // %
    float usage;    
} CpuUsage;

typedef struct MemUsage { // KB; Capacity
    size_t memTotal; 
    size_t memUse;
    size_t swapTotal;
    size_t swapUse;
} MemUsage;

/* log content */
typedef struct TempLog {
    DateInfo date;
    TempInfo temp;
} TempLog;

typedef struct UsageLog {
    DateInfo date;
    CpuUsage cpu;
    MemUsage mem;
} UsageLog;

typedef struct WarningLog {
    DateInfo date;
    TempInfo temp;
    CpuUsage cpuUsage;
    MemUsage memUsage;
    short type;
} WarningLog;

#endif