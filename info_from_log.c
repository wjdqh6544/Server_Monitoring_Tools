#include "common.h"
#include "info_from_log.h"
#include "zz_struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

extern const DateInfo dateBuf;

int remove_History_Log(int type){ // Remove log file
    FILE* ls_ptr = NULL;
    DateInfo preserveDate[2]; // [0]: Yesterday's Date, [1]: Date 2 days ago
    char pathBuf[BUF_MAX_LINE] = { '\0' }, errorBuf[BUF_MAX_LINE] = { '\0' }, filenameBuf[WARNING_LOG_FORM_LEN] = { '\0' }, dateStr[3][9]; // [0]: today, [1]: yesterday, [2]: 2 days ago
    int res = 0;

    if (type == LOG_TYPE_WARNING) { // Remove warning log
        sprintf(pathBuf, GET_LOG_LIST, HISTORY_PATH, WARNING_LOG); // Get list of warning logs
    } else { // Remove information (Temperature, Usage) log
        sprintf(pathBuf, GET_LOG_LIST, HISTORY_PATH, INFO_LOG);
    }

    if ((ls_ptr = popen(pathBuf, "r")) == NULL) {
        strcpy(errorBuf, "popen() - ");
        strcat(errorBuf, pathBuf);
        exception(-2, "remove_All_Warning_History_File", errorBuf);
        return -100;
    }

    while(fscanf(ls_ptr, "%s", filenameBuf) != EOF) {
        if (type == LOG_TYPE_INFO) { // // Remove Information Log
            get_Before_day(preserveDate, NULL);
            get_Before_day(preserveDate + 1, preserveDate);
            sprintf(dateStr[0], "%04d%02d%02d", dateBuf.year, dateBuf.month, dateBuf.day);
            sprintf(dateStr[1], "%04d%02d%02d", preserveDate->year, preserveDate->month, preserveDate->day);
            sprintf(dateStr[2], "%04d%02d%02d", (preserveDate + 1)->year, (preserveDate + 1)->month, (preserveDate + 1)->day);
            if ((strstr(filenameBuf, dateStr[0]) != NULL) || (strstr(filenameBuf, dateStr[1]) != NULL) || (strstr(filenameBuf, dateStr[2]) != NULL))  { 
                // Date of today, yesterday or 2 days ago -> Skip. (Not remove)
                continue;
            }
        }
        sprintf(pathBuf, TMP_LOCATION_FORM, HISTORY_PATH, filenameBuf); // Get absolute path of a log file.
        if (remove(pathBuf) != 0) {
            exception(-4, "remove_All_Warning_History_File", pathBuf);
            res = -1;
            continue;
        }
    }

    return res;
}

int get_Warning_History_from_Log(WarningLog** warningList, int* warningCnt){ // Get Warning History
    FILE* ls_ptr = NULL;
    int idx = 0, fd = -1;
    char errorBuf[BUF_MAX_LINE] = { '\0' }, pathBuf[BUF_MAX_LINE] = { '\0' }, filenameBuf[WARNING_LOG_FORM_LEN] = { '\0' }, commandBuf[BUF_MAX_LINE] = { '\0' };
    
    *warningCnt = get_WarningLog_Line_Cnt();

    if (*warningList == NULL) {
        if ((*warningList = (WarningLog*)malloc((*warningCnt) * sizeof(WarningLog))) == NULL) {
            exception(-4, "get_Warning_History_from_Log", "malloc() - warningList");
            return -100;
        }
    }

    sprintf(commandBuf, GET_LOG_LIST, HISTORY_PATH, WARNING_LOG);
    if ((ls_ptr = popen(commandBuf, "r")) == NULL) { // Get List of btmp
        strcpy(errorBuf, "popen() - (for fileList) %s");
        strcat(errorBuf, commandBuf);
        exception(-2, "get_Warning_History_from_Log", errorBuf);
        return -100;
    }
    
    for (idx = 0; idx < *warningCnt; idx++) { // ls_ptr -> list of btmp filename 
        fscanf(ls_ptr, "%s", filenameBuf); // Get filename in the list.
        sprintf(pathBuf, TMP_LOCATION_FORM, HISTORY_PATH, filenameBuf);
        if ((fd = open(pathBuf, O_RDONLY)) == -1 ) { // Open target Warning file.
            exception(-1, "get_Warning_History_from_Log", pathBuf);
            return -100;
        }
        
        while(read(fd, &((*warningList)[idx++]), sizeof(WarningLog)) == sizeof(WarningLog));
    }
    return 0;
}

int get_WarningLog_Line_Cnt(){ // Get the number of Warning history 
    FILE* ls_ptr = NULL;
    char pathBuf[BUF_MAX_LINE] = { '\0' }, errorBuf[BUF_MAX_LINE] = { '\0' };
    size_t bytes = 0, sumBytes = 0;

    sprintf(pathBuf, GET_LOG_SIZE, HISTORY_PATH, WARNING_LOG); // Get list of warning logs
    if ((ls_ptr = popen(pathBuf, "r")) == NULL) {
        strcpy(errorBuf, "popen() - ");
        strcat(errorBuf, pathBuf);
        exception(-2, "get_WarningLog_Line_Cnt", errorBuf);
        return -100;
    }

    for (int i = 0; fscanf(ls_ptr, "%ld", &bytes) != EOF; i++) { // Get size of each logfile, and Calculate accumulated sum.
        sumBytes += bytes;
    }

    return sumBytes / sizeof(WarningLog); // Calculate the number of history log content.
}

int get_Memory_Usage_from_Log(MemUsage* memBuf){ // Get Memory (Physical, SWAP) Usage (Capacity) from log file.
    int fd = -1;
    UsageLog logBuf;
    char fullpath[MAX_LOG_PATH_LEN] = { '\0' };

    get_Filename(fullpath, HISTORY_PATH, USAGE_LOG, &dateBuf);
    if ((fd = open(fullpath, O_RDONLY)) == -1) {
        exception(-1, "get_Memory_Usage_from_Log", fullpath);
        return -100;
    }

    lseek(fd, -sizeof(UsageLog), SEEK_END); // Move a pointer to read a last data.
    if (read(fd, &logBuf, sizeof(UsageLog)) != sizeof(UsageLog)) {
        exception(-2, "get_Memory_Usage_from_Log", "<Read data size> != sizeof(UsageLog)");
        return -100;
    }

    memBuf->memTotal = logBuf.mem.memTotal; // Extract and copy capacity data.
    memBuf->memUse = logBuf.mem.memUse;
    memBuf->swapTotal = logBuf.mem.swapTotal;
    memBuf->swapUse = logBuf.mem.swapUse;

    close(fd);

    return 0;
}

int get_Average_Usage_Percent_from_Log(float* avg_usage_buf, int inputInterval, int type){ // Get average usage information from log file.
    // avg_usage_buf: Must be allocated to memory. (At caller, Array / Vairable must be allocated in memory)
    //Average interval: inputInterval; MAX = 86400 (1 days)
    int fd = -1, change_file = 0, line_cnt = 0, interval = inputInterval;
    UsageLog usageBuf;
    DateInfo dateTmp;
    char fullpath[MAX_LOG_PATH_LEN] = { '\0' };

    switch (type) { // Initialization
        case TYPE_CPU_USAGE:
            avg_usage_buf[0] = 0; // CPU Usage
            break;
        case TYPE_MEM_USAGE:
            avg_usage_buf[0] = 0; // Physical Memory Usage
            avg_usage_buf[1] = 0; // SWAP Usage
            break;
        default:
            exception(-5, "get_Average_Usage_from_Log", "Wrong Type");
            return -100;
    }

    get_Filename(fullpath, HISTORY_PATH, USAGE_LOG, &dateBuf);
    if ((fd = open(fullpath, O_RDONLY)) == -1){
        exception(-1, "get_Average_Usage_from_Log", fullpath);
        return -100;
    }

    for (int line = 1; line <= interval; line++) { // Read data (# of data: intervalSec)
        if (change_file == 1) { // Change target file. -> read before day file.
            close(fd); // Close current opened file descriptor.
            
            dateTmp = dateBuf;
            get_Before_day(&dateTmp, NULL);
            get_Filename(fullpath, HISTORY_PATH, USAGE_LOG, &dateTmp); // Get full path of target file (before day)
            
            if ((fd = open(fullpath, O_RDONLY)) == -1){ // Open before day file.
                exception(-1, "get_Average_Usage_from_Log (at Before Day)", fullpath);
                break;
            }

            line = 1; // Re-Setting file pointer offset
            interval -= line_cnt;
            change_file = 0;
        }

        if (lseek(fd, (-sizeof(UsageLog) * line), SEEK_END) == 0) { // Move the pointer to the start of target data.
            change_file = 1; // Need to change target file. (because all of them is read, additional data does not exists anymore.)
        }

        if (read(fd, &usageBuf, sizeof(UsageLog)) != sizeof(UsageLog)) {
            exception(-2, "get_Average_Usage_from_Log", "<Read data size> != sizeof(UsageLog)");
            continue;
        }

        line_cnt++;

        switch (type) { // Sum Temperature data
            case TYPE_CPU_USAGE:
                avg_usage_buf[0] += usageBuf.cpu.usage;
                break;
            case TYPE_MEM_USAGE:
                avg_usage_buf[0] += get_Capacity_Percent(usageBuf.mem.memTotal, usageBuf.mem.memUse);
                avg_usage_buf[1] += ((usageBuf.mem.swapTotal != 0) ? get_Capacity_Percent(usageBuf.mem.swapTotal, usageBuf.mem.swapUse) : 0);
                break;
        }
    }

    close(fd);

    avg_usage_buf[0] /= (float)line_cnt; // Calculate average.
    (type == TYPE_MEM_USAGE) ? avg_usage_buf[1] /= (float)line_cnt : 0;

    return 0;
}

int get_Average_Temperature_from_Log(float* avg_temp_buf, int inputInterval, int type){ // Get average temperature information from log file.
    // avg_temp_buf: Must be allocated to memory. (At caller, Array / Vairable must be allocated in memory)
    //Average interval: inputInterval; MAX = 86400 (1 days)
    int fd = -1, change_file = 0, line_cnt = 0, interval = inputInterval;
    TempLog tempBuf;
    DateInfo dateTmp;
    char fullpath[MAX_LOG_PATH_LEN] = { '\0' };

    switch (type) { // Initialization
        case TYPE_INLET_TEMP:
        case TYPE_EXHAUST_TEMP:
        case TYPE_RAID_CORE_TEMP:
        case TYPE_RAID_CTRL_TEMP:
        case TYPE_BBU_TEMP:
            avg_temp_buf[0] = -100;
            break;
        case TYPE_CPU_TEMP:
            avg_temp_buf[0] = -100;
            avg_temp_buf[1] = -100;
            break;
        case TYPE_STORAGE_TEMP:
            for (int storageIdx = 0; storageIdx < MAX_STORAGE_COUNT; storageIdx++){
                avg_temp_buf[storageIdx]  = -100;
            }
            break;
        default:
            exception(-5, "get_Average_Usage_from_Log", "Invalid function parameter");
            return -100;
    }

    get_Filename(fullpath, HISTORY_PATH, TEMP_LOG, &dateBuf);
    if ((fd = open(fullpath, O_RDONLY)) == -1){
        exception(-1, "get_Average_Temperature_from_Log", fullpath);
        return -100;
    }

    for (int line = 1; line <= interval; line++) { // Read data (# of data: intervalSec)
        if (change_file == 1) { // Change target file. -> read before day file.
            close(fd); // Close current opened file descriptor.
            
            dateTmp = dateBuf;
            get_Before_day(&dateTmp, NULL);
            get_Filename(fullpath, HISTORY_PATH, TEMP_LOG, &dateTmp); // Get full path of target file (before day)
            
            if ((fd = open(fullpath, O_RDONLY)) == -1){ // Open before day file.
                exception(-1, "get_Average_Temperature_from_Log (at Before Day)", fullpath);
                break;
            }

            line = 1; // Re-Setting file pointer offset
            interval -= line_cnt;
            change_file = 0;
        }

        if (lseek(fd, (-sizeof(TempLog) * line), SEEK_END) == 0) { // Move the pointer to the start of target data.
            change_file = 1; // Need to change target file. (because all of them is read, additional data does not exists anymore.)
        }

        if (read(fd, &tempBuf, sizeof(tempBuf)) != sizeof(tempBuf)) {
            exception(-2, "get_Average_Temperature_from_Log", "<Read data size> != sizeof(TempLog)");
            continue;
        }

        line_cnt++;

        switch (type) { // Sum Temperature data
            case TYPE_INLET_TEMP:
                if (avg_temp_buf[0] == -100) {
                    avg_temp_buf[0] = tempBuf.temp.inlet;
                } else {
                    avg_temp_buf[0] += tempBuf.temp.inlet;
                }
                break;
            case TYPE_EXHAUST_TEMP:
                if (avg_temp_buf[0] == -100) {
                    avg_temp_buf[0] = tempBuf.temp.exhaust;
                } else {
                    avg_temp_buf[0] += tempBuf.temp.exhaust;
                }
                break;
            case TYPE_CPU_TEMP:
                for (int i = 0; i < MAX_CPU_COUNT; i++){
                    if (avg_temp_buf[i] == -100) {
                        avg_temp_buf[i] = tempBuf.temp.cpu[i];
                    } else {
                        avg_temp_buf[i] += tempBuf.temp.cpu[i];
                    }
                }
                break;
            case TYPE_RAID_CORE_TEMP:
                if (avg_temp_buf[0] == -100) {
                    avg_temp_buf[0] = tempBuf.temp.raidCore;
                } else {
                    avg_temp_buf[0] += tempBuf.temp.raidCore;
                }
                break;
            case TYPE_RAID_CTRL_TEMP:
                if (avg_temp_buf[0] == -100) {
                    avg_temp_buf[0] = tempBuf.temp.raidController;
                } else {
                    avg_temp_buf[0] += tempBuf.temp.raidController;
                }
                break;
            case TYPE_BBU_TEMP:
                if (avg_temp_buf[0] == -100) {
                    avg_temp_buf[0] = tempBuf.temp.bbu;
                } else {
                    avg_temp_buf[0] += tempBuf.temp.bbu;
                }
                break;
            case TYPE_STORAGE_TEMP:
                for (int storageIdx = 0; storageIdx < MAX_STORAGE_COUNT; storageIdx++){
                    if (avg_temp_buf[storageIdx] == -100) {
                        avg_temp_buf[storageIdx] = tempBuf.temp.storage[storageIdx];
                    } else {
                        avg_temp_buf[storageIdx] += tempBuf.temp.storage[storageIdx];
                    }
                }
                break;
        }
    }

    close(fd);

    if (type == TYPE_STORAGE_TEMP) { // Calculate average.
        for (int idx = 0; idx < MAX_STORAGE_COUNT; avg_temp_buf[idx++] /= (float)line_cnt);
    } else if (type == TYPE_CPU_TEMP){
        for (int idx = 0; idx < MAX_CPU_COUNT; avg_temp_buf[idx++] /= (float)line_cnt);
    } else {
        avg_temp_buf[0] /= (float)line_cnt;
    }

    return 0;
}

void get_Before_day(DateInfo* buf, const DateInfo* pointDate){ // Get before day.
    // buf: Pointer that store the date, pointDate: Baseline date, Nullable.
    // If pointDate is null, Calculate with base of today.
    int daysInMonth[] = { 31, (dateBuf.year % 4 == 0 && dateBuf.year % 100 != 0) || dateBuf.year % 400 == 0 ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    DateInfo baseline;
    baseline = ((pointDate == NULL) ? dateBuf : (*pointDate));
    
    buf->year = baseline.year;
    buf->month = baseline.month;
    buf->day = baseline.day - 1;
    buf->hrs = baseline.hrs;
    buf->min = baseline.min;
    buf->sec = baseline.sec;

    if (buf->day < 1) { // If first day.
        buf->month -= 1;
        if (buf->month < 1) { // If first month.
            buf->month = 12;
            buf->year -= 1;
        }
        buf->day = daysInMonth[buf->month - 1]; // Last day of before month.
    }
}