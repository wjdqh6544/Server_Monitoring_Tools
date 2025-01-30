#ifndef HW_INFO_H

#define HW_INFO_H
#include "zz_struct.h"

/* Functions that get server information (System Parts include CPU, Memory, PSU, Network, and so on.) */
void get_System_Information_from_Omreport(SystemInfo* systemBuf);
void get_Server_Information_from_Omreport(char* hostname, char* serverModel, char* serviceTag, char* serviceCode);
void get_CPU_Information_from_Omreport(CPUInfo* cpuBuf);
void get_Memory_Information_from_Omreport(MEMInfo* memoryBuf);
void get_CMOS_Battery_Status_from_Omreport(CMOS_BAT_INFO* cmosBatteryBuf);
void get_Fan_Status_from_Omreport(FANInfo* fanBuf);
void get_Physical_IFA_Information_from_Omreport(PHYSICAL_IFA_Info** ifaBuf, short* ifaCount);
void get_Interface_Speed_from_Omreport(PHYSICAL_IFA_Info* ifaBuf, short ifaCount);
void get_PSU_Status_from_Omreport(short* psuStatusBuf);

/* Functions that get disk information */
short get_VDisk_Information_from_Perccli(VDInfo** vdBuf, short* virtualDriveCnt);
void get_VDisk_FileSystem_from_Perccli(VDInfo* vdBuf, short virtualDiskCnt);
short get_Disk_Information_from_Perccli(DiskInfo** diskBuf, short* diskCount);
void get_Disk_Product_Name_from_Perccli(DiskInfo* diskBuf, short diskCount);

/* Functions that get HBA Card information */
short get_HBA_Information_from_Perccli(HBAInfo* HBABuf);
void get_BBU_Information_from_Perccli(BBUInfo* BBUBuf);

/* Functions that get CPU information from Jiffies (/proc/cpuinfo) */
void get_CPU_Usage_Percent_of_each_Core(float** usage_buf);
short get_Physical_CPU_Count(short totalCore);
short get_CPU_Core_Count();

#endif