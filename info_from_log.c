#include "common.h"
#include "info_from_log.h"
#include "zz_struct.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

extern DateInfo dateBuf;

// int get_Warning_History_from_Log(){ // Get Warning History

// }

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
            get_Before_day(&dateTmp);
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
            get_Before_day(&dateTmp);
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

void get_Before_day(DateInfo* buf){ // Get before day.
    int daysInMonth[] = { 31, (dateBuf.year % 4 == 0 && dateBuf.year % 100 != 0) || dateBuf.year % 400 == 0 ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    buf->year = dateBuf.year;
    buf->month = dateBuf.month;
    buf->day = dateBuf.day - 1;
    if (buf->day < 1) { // If first day.
        buf->month -= 1;
        if (buf->month < 1) { // If first month.
            buf->month = 12;
            buf->year -= 1;
        }
        buf->day = daysInMonth[buf->month - 1]; // Last day of before month.
    }
}