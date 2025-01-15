#ifndef COMMON_H

#define COMMON_H
#include "zz_struct.h"
#include <sys/types.h>

void get_Date();
void exception(short code, char *func_name, char *detail);
double convert_Size_Unit(size_t size, UNIT unit);
void get_Filename(char* path, char* filename, char* type, DateInfo* targetDate);
short check_Log_Directory(char* fullpath, mode_t permissions);

#endif