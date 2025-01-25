#ifndef HW_INFO_H

#define HW_INFO_H
#include "zz_struct.h"

int get_Disk_Information_from_Perccli(DiskInfo** diskBuf, int* diskCount);
void get_Disk_Product_Name_from_Perccli(DiskInfo* diskBuf, int diskCount);
int get_HBA_Information_from_Perccli(HBAInfo* HBABuf);
void get_BBU_Information_from_Perccli(BBUInfo* BBUBuf);
void get_CPU_Usage_Percent_of_each_Core(float** usage_buf);
int get_Physical_CPU_Count(int totalCore);
int get_CPU_Core_Count();

#endif