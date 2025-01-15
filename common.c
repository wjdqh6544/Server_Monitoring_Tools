#include "0_usrDefine.h"
#include "zz_struct.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

extern DateInfo dateBuf;

void get_Date(){
    time_t current_time;
    struct tm *tm;
    current_time = time(NULL);
    tm = localtime(&current_time);
    
    dateBuf.year = (short)((tm->tm_year) + 1900);
    dateBuf.month = (short)((tm->tm_mon) + 1);
    dateBuf.day = (short)(tm->tm_mday);
    dateBuf.hrs = (short)(tm->tm_hour);
    dateBuf.min = (short)(tm->tm_min);
    dateBuf.sec = (short)(tm->tm_sec);
}

void exception(short code, char *func_name, char *detail){
    char type[ERROR_MSG_LEN];
    char *detail_str = (detail != NULL) ? detail : "";
    switch (code) {
        case -1:
            strcpy(type, "Open File");
            break;
        case -2:
            strcpy(type, "Read Data");
            break;
        case -3:
            strcpy(type, "Write Data");
            break;
        case -4:
            strcpy(type, "Invoke System Call");
            break;
    }
    printf("%04d-%02d-%02d %02d:%02d:%02d (func. - %s) Cannot %s: %s\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, func_name, type, detail_str);
}

double convert_Size_Unit(size_t size, UNIT unit){ // Convert to the largest capacity unit. (Minimum: KB / ex. 1,048,576 KB -> 1 GB)
    double res = (double)size;
    for (int i = 0; i < (int)unit; i++){
        res /= 1024;
    }
    return res;
}

void get_Filename(char* path, char* filename, char* type, DateInfo* targetDate){ // Create a full path of Log file.
    if (path[strlen(path) - 1] == '/'){ // filename form: <type>-YYYYMMDD
        sprintf(filename, "%s%s-%04d%02d%02d", path, type, targetDate->year, targetDate->month, targetDate->day); // <path(Directory)>/<type>-YYYYMMDD
    } else {
        sprintf(filename, "%s/%s-%04d%02d%02d", path, type, targetDate->year, targetDate->month, targetDate->day);
    }
    
}

short check_Log_Directory(char* fullpath, mode_t permissions){ // Check the presence of directory, and Create it if not exists.
    DIR* dir_ptr = NULL;
    char path[MAX_LOG_PATH_LEN] = { '\0' };
    char* path_ptr = path + 1;
    if ((dir_ptr = opendir(fullpath)) == NULL){ // If directory does not exist.
        strcpy(path, fullpath);
        while(*path_ptr){ // To create the directory recursively. (Child directory exists.)
            if((*path_ptr) == '/') {
                (*path_ptr) = '\0';
                if ((dir_ptr = opendir(path)) == NULL) { // Check whether parent directory exists.
                    if (mkdir(path, permissions) == -1){
                        return -1;
                    }
                } else {
                    closedir(dir_ptr);
                }
                (*path_ptr) = '/';
            }
            path_ptr++;
        }
        if (mkdir(path, permissions) == -1){ // Create last child directory. 
            return -1;
        }
    } else {
        closedir(dir_ptr);
    }
    return 0;
}

