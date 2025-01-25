#ifndef COMMON_H

#define COMMON_H
#include "zz_struct.h"
#include <sys/types.h>

void get_Date();
int check_Package_Installed(char* type);
void exception(short code, char *func_name, char *detail);
void get_Filename(char* fullpathBuf, char* path, char* filename, DateInfo* targetDate);
short check_Log_Directory(char* fullpath, mode_t permissions);
float get_Capacity_Percent(size_t total, size_t use);
double convert_Size_Unit(size_t size, int autoUnit, UNIT* unit);
void remove_Space_of_Head(char* ptr);
void remove_Space_of_Tail(char* ptr);

#endif