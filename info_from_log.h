#ifndef INFO_FROM_LOG_H

#define INFO_FROM_LOG_H
#include "zz_struct.h"

int remove_History_Log(int type);
int get_Warning_History_from_Log(WarningLog** warningList, int* warningCnt);
int get_WarningLog_Line_Cnt();
int get_Memory_Usage_from_Log(MemUsage* memBuf);
int get_Average_Usage_Percent_from_Log(float* avg_usage_buf, int inputInterval, int type);
int get_Average_Temperature_from_Log(float* avg_temp_buf, int inputInterval, int type);
void get_Before_day(DateInfo* buf, const DateInfo* pointDate);

#endif