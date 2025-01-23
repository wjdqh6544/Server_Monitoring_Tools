#ifndef INFO_FROM_LOG_H

#define INFO_FROM_LOG_H
#include "zz_struct.h"

int get_Memory_Usage_from_Log(MemUsage* memBuf);
int get_Average_Usage_Percent_from_Log(float* avg_usage_buf, int inputInterval, int type);
int get_Average_Temperature_from_Log(float* avg_temp_buf, int inputInterval, int type);
void get_Before_day(DateInfo* buf);

#endif