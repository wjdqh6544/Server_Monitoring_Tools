#ifndef RESOURCES_MONITOR_H

#define RESOURCES_MONITOR_H
#include "zz_struct.h"

/* List of functinos relative to 1st features (Display System Information)*/
void one_System_Information();

/* List of functions relative to 2st feature (Hardware Temperature) */
void two_Hardware_Temperature();

/* List of functions relative to 3rd feature (Display CPU / Memory Usage)*/
void three_CPU_Memory_Usage();
int get_Maximum_Length_of_ProcessInfo(ProcessInfo* psList, int psCnt, int type);

/* List of functions relative to 4th features (Display Disk Information)*/
void four_Disk_Information();
int get_Maximum_Length_of_DiskInfo(DiskInfo* diskList, int diskCnt, int type);
void display_Partition_Information();
int get_Maximum_Length_of_PartitionInfo(PartitionInfo* partList, int partCnt, int type);
void get_Percent_Bar(char* barBuf, float percent, int length);

/* List of functions relative to 5th features (Display Network (IFA) Information) */
void five_Network_Information();
int get_Maximum_Length_of_Physical_IFAInfo(PHYSICAL_IFA_Info* ifaList, int ifaCount, int type);
void display_IFA_Speed(DockerInfo** containerList, IFASpeed* ifaList, int ifaCount);
int get_Maximum_Length_of_Physical_IFASpeed(IFASpeed* ifaList, int ifaCount);

#endif