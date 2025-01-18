#ifndef STRUCT

#define STRUCT
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

/* Temp, Usage Information */
typedef struct TempInfo { // Celcius
    short inlet;
    short exhaust;
    short cpu[MAX_CPU_COUNT];
    short raidCore;
    short raidController;
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