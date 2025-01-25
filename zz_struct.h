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

/* Disk Information */
typedef struct PartitionInfo {
    char fileSystem[MAX_FILESYSTEM_LEN];
    char mountPath[MAX_MOUNTPATH_LEN];
    size_t spaceTotal;
    size_t spaceUse;
} PartitionInfo;

typedef struct DiskInfo {
    short enclosureNum;
    short slotNum;
    short deviceID;
    short driveGroup;
    short status;
    char modelName[MAX_DISK_MODELNAME_LEN];
    char capacity[MAX_DISK_CAPACITY_LEN];
    short capUnit;
    short mediaType;
    short interface;
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