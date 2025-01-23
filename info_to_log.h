#ifndef INFO_TO_LOG_H

#define INFO_TO_LOG_H
#include "zz_struct.h"

void* write_Temperature_to_Log();
void* write_Usage_to_Log();
void* write_Warning_to_Log();
void get_Temperature_from_System(TempInfo* tempBuf);
void get_Temperature_Omreport(TempInfo* tempBuf);
void get_Temperature_Perccli(TempInfo* tempBuf);
void get_CPU_Usage_Percent_of_All_Core(CpuUsage* cpuUsageBuf);
void get_Memory_Usage(MemUsage* memUsageBuf);
int over_Critical_Point(WarningLog* warningBuf);

#endif