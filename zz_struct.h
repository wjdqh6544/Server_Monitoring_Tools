#ifndef STRUCT_H

#define STRUCT_H
#include "0_usrDefine.h"
#include <grp.h>
#include <net/if.h>
#include <utmp.h>
#include <pwd.h>

/* Main-Program */
typedef struct MenuItem { // Main Menu
    const char* menuStr;
    void(*function)();
} MenuItem;

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

typedef struct Unit_Mapping { // for Displaying Unit
    UNIT unit;
    char* str;
} Unit_Mapping;

typedef struct ProcessInfo { // CPU / Memory Usage
    char userName[UT_NAMESIZE + 1];
    pid_t pid;
    float cpu;
    float mem;
    size_t memUseSize;
    char tty[UT_LINESIZE + 1];
    char start[TIME_LEN];
    char time[TIME_LEN];
    char* command;
} ProcessInfo;

/* File Permissions */
typedef struct FileInfo { // Core file status
    char fullPath[MAX_MOUNTPATH_LEN];
    char path[MAX_PATH_LEN];
    int changed[3]; // [0]: uid / [1]: gid / [2]: permission | -1: ERROR, 0: Do not need to be changed, 1: Need to be changed.
    uid_t ownerUID[2]; // [0]: Before, [1]: After
    gid_t groupGID[2]; // [0]: Before, [1]: After
    mode_t permission[2]; // [0]: Before, [1]: After
} FileInfo;

/* Linux Users */
typedef struct UserInfo { // User Information + Last PW Change date
    char* userName;
    uid_t uid;
    int grpCnt;
    gid_t* gid;
    DateInfo lastLogin;
    char loginIP[UT_LINESIZE + 8];
    DateInfo lastChangePW;
} UserInfo;

typedef struct LoginInfo { // Login History
    char userName[UT_NAMESIZE + 1];
    uid_t uid;
    int status; // Success or Failed
    DateInfo logDate;
    char deviceName[UT_LINESIZE + 1];
    char loginIP[UT_LINESIZE + 1];
} LoginInfo;

/* Network Interface */
typedef struct DockerInfo { // Network Information
    short* checked;
    char containerName[MAX_DOCKER_CONTAINER_NAME_LEN];
    pid_t pid;
    short ifaCount;
    char** vethName;
    char** ipv4_addr;
    short* ifa_index;
} DockerInfo;

typedef struct IFASpeed { // Network Information
    char ipv4_addr[IPV4_LEN];
    char ifa_name[MAX_DOCKER_CONTAINER_NAME_LEN];
    float speedRX;
    float speedTX;
    size_t errorRX;
    size_t errorTX;
    size_t dropRX;
    size_t dropTX;
} IFASpeed;

/* Parts Information */
typedef struct CPUInfo { // System Information
    char name[MAX_PARTS_NAME_LEN];
    short status;
    short coreCnt;
} CPUInfo;

typedef struct MEM_UNIT_INFO { // System Information
    int status;
    char connectorName[4];
    char type[MAX_PARTS_NAME_LEN];
    char capacity[MAX_CAPACITY_LEN];
} MEM_UNIT_INFO;

typedef struct MEMInfo { // System Information
    short slotsTotal;
    short slotsUsed;
    char installedCapacity[MAX_CAPACITY_LEN];
    char errorCorrection[MAX_PARTS_NAME_LEN];
    MEM_UNIT_INFO* unit;
} MEMInfo;

typedef struct CMOS_BAT_INFO { // System Information
    char name[MAX_PARTS_NAME_LEN];
    int status;
} CMOS_BAT_INFO;

typedef struct FANInfo { // System Information
    int status;
    char rpm[MAX_CAPACITY_LEN];
    char name[MAX_PARTS_NAME_LEN];
} FANInfo;

typedef struct PHYSICAL_IFA_Info { // System Information
    char name[MAX_PARTS_NAME_LEN];
    char ifName[MAX_PARTS_NAME_LEN];
    int connected;
    char speed[MAX_CAPACITY_LEN];
} PHYSICAL_IFA_Info;

typedef struct SystemInfo { // System Information
    char hostname[MAX_PARTS_NAME_LEN];
    char serverModel[MAX_PARTS_NAME_LEN];
    char serviceTag[MAX_PARTS_NAME_LEN];
    char serviceCode[MAX_PARTS_NAME_LEN];
    CPUInfo cpu[MAX_CPU_COUNT];
    MEMInfo mem;
    CMOS_BAT_INFO cmosBattery;
    FANInfo fan[MAX_FAN_COUNT];
    int psuStatus[MAX_PSU_COUNT];
} SystemInfo;

/* Disk Information */
typedef struct PartitionInfo { // DIsk Information
    char fileSystem[MAX_FILESYSTEM_LEN];
    char mountPath[MAX_MOUNTPATH_LEN];
    size_t spaceTotal;
    size_t spaceUse;
} PartitionInfo;

typedef struct VDInfo { // DIsk Information
    short driveGroup;
    short virtualDrive;
    char type[MAX_VDISK_TYPE_LEN];
    short status;
    short access;
    char capacity[MAX_CAPACITY_LEN];
    char vdName[MAX_VDISK_NAME_LEN];
    char fileSystem[MAX_FILESYSTEM_LEN];
    int mountPathCnt;
    char** mountPath;
} VDInfo;

typedef struct DiskInfo { // DIsk Information
    short enclosureNum;
    short slotNum;
    short driveGroup;
    short status;
    int deviceID;
    char modelName[MAX_DISK_MODELNAME_LEN];
    char capacity[MAX_CAPACITY_LEN];
    char mediaType[MAX_DISK_MEDIATYPE_LEN];
    char interface[MAX_DISK_INTERFACE_LEN];
    char mappedPartition[MAX_MOUNTPATH_LEN];
} DiskInfo;

typedef struct BBUInfo { // DIsk Information
    int status;
    char voltage[MAX_BBU_VOLTAGE_LEN];
    char designCapacity[MAX_BBU_CAPACITY_LEN];
    char fullCapacity[MAX_BBU_CAPACITY_LEN];
    char remainCapacity[MAX_BBU_CAPACITY_LEN];
} BBUInfo;

typedef struct HBAInfo { // DIsk Information
    char HBA_Name[MAX_HBA_NAME_LEN];
    char HBA_Bios_Ver[MAX_HBA_BIOS_VER_LEN];
    char HBA_Serial_Num[MAX_HBA_SERIAL_LEN];
    char HBA_Firmware_Ver[MAX_HBA_FIRMWARE_VER_LEN];
    char HBA_Driver_Name[MAX_HBA_DRIVER_NAME_LEN];
    char HBA_Driver_Ver[MAX_HBA_DRIVER_VER_LEN];
    short status;
    short HBA_Cur_Personality; // Mode
    int HBA_Drive_Groups_Cnt;
    BBUInfo bbuStatus;
} HBAInfo;

/* Temp Information */
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

/* Usage Information */
typedef struct CpuUsage { // %
    float usage;    
} CpuUsage;

typedef struct MemUsage { // KB; Capacity
    size_t memTotal; 
    size_t memUse;
    size_t swapTotal;
    size_t swapUse;
} MemUsage;

/* saved struct to log */
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