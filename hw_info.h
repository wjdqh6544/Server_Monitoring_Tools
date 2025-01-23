#ifndef HW_INFO_H

#define HW_INFO_H

void get_CPU_Usage_Percent_of_each_Core(float** usage_buf);
int get_Physical_CPU_Count(int totalCore);
int get_CPU_Core_Count();

#endif