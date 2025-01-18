#ifndef INFOTOLOG

#define INFOTOLOG
#include "zz_struct.h"

void* write_Temperature_to_Log();
void* write_Usage_to_Log();
void* write_Warning_to_Log();
void get_Temperature(TempInfo* tempBuf);
void get_Temperature_Omreport(TempInfo* tempBuf);
void get_Temperature_Perccli(TempInfo* tempBuf);
void get_CPU_Usage(CpuUsage* cpuUsageBuf);
void get_Memory_Usage(MemUsage* memUsageBuf);
int over_Critical_Point(WarningLog* warningBuf);

#endif