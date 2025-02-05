#ifndef HW_INFO_H

#define HW_INFO_H
#include "zz_struct.h"

/* List of functions that get server information (System Parts include CPU, Memory, PSU, Network, and so on.) */
void get_System_Information_from_Omreport(SystemInfo* systemBuf);
void get_Server_Information_from_Omreport(char* hostname, char* serverModel, char* serviceTag, char* serviceCode);
void get_CPU_Information_from_Omreport(CPUInfo* cpuBuf);
void get_Memory_Information_from_Omreport(MEMInfo* memoryBuf);
void get_CMOS_Battery_Status_from_Omreport(CMOS_BAT_INFO* cmosBatteryBuf);
void get_Fan_Status_from_Omreport(FANInfo* fanBuf);
void get_PSU_Status_from_Omreport(int* psuStatusBuf);

/* List of functions of using to get physical network interface information */
void get_Physical_IFA_Information_from_Omreport(PHYSICAL_IFA_Info** ifaBuf, int* ifaCount);
void get_Interface_Speed_from_Omreport(PHYSICAL_IFA_Info* ifaBuf, int ifaCount);

/* List of functions that get disk information */
int get_VDisk_Information_from_Perccli(VDInfo** vdBuf, int* virtualDriveCnt);
void get_VDisk_FileSystem_from_Perccli(VDInfo* vdBuf, int virtualDiskCnt);
int get_Disk_Information_from_Perccli(DiskInfo** diskBuf, int* diskCount);
void get_Disk_Product_Name_from_Perccli(DiskInfo* diskBuf, int diskCount);

/* List of functions that get HBA Card information */
int get_HBA_Information_from_Perccli(HBAInfo* HBABuf);
void get_BBU_Information_from_Perccli(BBUInfo* BBUBuf);

/* List of functions that get CPU information from Jiffies (/proc/cpuinfo) */
void get_CPU_Usage_Percent_of_each_Core(float** usage_buf);
int get_Physical_CPU_Count(int totalCore);
int get_CPU_Core_Count();

#endif