#include "0_usrDefine.h"
#include "common.h"
#include "info_to_log.h"
#include "zz_struct.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern DateInfo dateBuf;

void* write_Temperature_to_Log(){ // Write temperature information to Log file.
    int fd = -1;
    TempLog tempBuf;
    char fullpath[MAX_LOG_PATH_LEN] = { '\0' };

    get_Filename(fullpath, HISTORY_PATH, TEMP_LOG, &dateBuf); // Destination: /var/log/00_Server_Monitoring/00_history/Temperature_history-YYYYMMDD
    if ((fd = open(fullpath, O_WRONLY | O_CREAT | O_APPEND, 0640)) == -1){
        exception(-1, "write_Temperature_to_Log", fullpath);
        return NULL;
    }

    tempBuf.date = dateBuf;
    get_Temperature_Omreport(&(tempBuf.temp));

    if (write(fd, &tempBuf, sizeof(TempLog)) != sizeof(TempLog)) {
        exception(-3, "write_Temperature_to_Log", "<Writed data size> != sizeof(TempLog)");
    }
    close(fd);

    return NULL;
}

void* write_Usage_to_Log(){ // Write usage information to Log file.
    int fd = -1;
    UsageLog usageBuf;
    char fullpath[MAX_LOG_PATH_LEN] = { '\0' };

    get_Filename(fullpath, HISTORY_PATH, USAGE_LOG, &dateBuf); // Destination: /var/log/00_Server_Monitoring/00_history/Usage_history-YYYYMMDD
    if ((fd = open(fullpath, O_WRONLY | O_CREAT | O_APPEND, 0640)) == -1){
        exception(-1, "write_Usage_to_Log", fullpath);
        return NULL;
    }

    usageBuf.date = dateBuf;
    get_CPU_Usage(&(usageBuf.cpu));
    get_Memory_Usage(&(usageBuf.mem));

    if (write(fd, &usageBuf, sizeof(UsageLog)) != sizeof(UsageLog)) {
        exception(-3, "write_Usage_to_Log", "<Writed data size> != sizeof(UsageLog)");
    }
    close(fd);

    return NULL;
}

void get_Temperature_Omreport(TempInfo* tempBuf){ // Get Temperature Information from omreport
    FILE* omreport_ptr = NULL;
    char lineBuf[TEMP_MAX_LINE] = { 0 };
    char* targetPos = NULL;

    tempBuf->inlet = -100; // Initialization; -100 means "N/A"
    tempBuf->exhaust = -100;
    tempBuf->cpu[0] = -100;
    tempBuf->cpu[1] = -100;

    if ((omreport_ptr = popen(GET_TEMP_COMMAND, "r")) == NULL) { // Get raw data from omreport
        exception(-2, "get_Temperature", "omreport");
        return;
    }
    
    while (fgets(lineBuf, sizeof(lineBuf), omreport_ptr) != NULL){
        if ((targetPos = strstr(lineBuf, "Inlet")) != NULL){ // Extract Inlet temperature
            fgets(lineBuf, sizeof(lineBuf), omreport_ptr);
            targetPos = strstr(lineBuf, ": ");
            sscanf(targetPos, ": %f C", &(tempBuf->inlet));
        } else if ((targetPos = strstr(lineBuf, "Exhaust")) != NULL){ // Extract Exhaust temperature
            fgets(lineBuf, sizeof(lineBuf), omreport_ptr);
            targetPos = strstr(lineBuf, ": ");
            sscanf(targetPos, ": %f C", &(tempBuf->exhaust));
        } else if ((targetPos = strstr(lineBuf, "CPU1")) != NULL){ // Extract CPU1 Package temperature
            fgets(lineBuf, sizeof(lineBuf), omreport_ptr);
            targetPos = strstr(lineBuf, ": ");
            sscanf(targetPos, ": %f C", &(tempBuf->cpu[0]));
        } else if ((targetPos = strstr(lineBuf, "CPU2")) != NULL){ // Extract CPU2 Package temperature
            fgets(lineBuf, sizeof(lineBuf), omreport_ptr);
            targetPos = strstr(lineBuf, ": ");
            sscanf(targetPos, ": %f C", &(tempBuf->cpu[1]));
        } 
    }

    pclose(omreport_ptr);
}

void get_CPU_Usage(CpuUsage* cpuUsageBuf){ // Get CPU Usage(%) from Jiffies
    unsigned long user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guest_nice = 0;
    static unsigned long priv_total = 0, priv_idle = 0;
    FILE *fp = NULL;
    cpuUsageBuf->usage = -100; // Initialization; -100 means "N/A"

    if ((fp = fopen(STAT_LOCATION, "r")) == NULL) {
        exception(-1, "get_CPU_Usage", STAT_LOCATION);
        return;
    }

    fscanf(fp, STAT_FORM, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
    fclose(fp);

    // Calculate CPU Usage (now - priv jiffies)
    cpuUsageBuf->usage = 100 * (1 - ((idle - priv_idle) / (float)((user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice) - priv_total))); 

    priv_total = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice; // Save now jiffies
    priv_idle = idle;
}

void get_Memory_Usage(MemUsage* memUsageBuf){ // Get Memory (Physical / SWAP) Size from /proc/meminfo
    FILE* fp = NULL;
    char buf[MEMINFO_MAX_LINE] = { '\0' };
    size_t size_tmp_buf = 0;

    memUsageBuf->memUse = 0; // Initialization
    memUsageBuf->memTotal = 0;
    memUsageBuf->swapUse = 0;
    memUsageBuf->swapTotal = 0;

    if ((fp = fopen(MEMINFO_LOCATION, "r")) == NULL){
        exception(-1, "get_Memory_Usage", MEMINFO_LOCATION);
        return;
    }

    while (fscanf(fp, MEMINFO_FORM, buf, &size_tmp_buf) != EOF) { // Read and extract memory size
        if (strcmp(buf, MEMTOTAL) == 0) {
            memUsageBuf->memTotal = size_tmp_buf;
        } else if (strcmp(buf, MEMFREE) == 0) {
            memUsageBuf->memUse = size_tmp_buf;
        } else if (strcmp(buf, SWAPTOTAL) == 0) {
            memUsageBuf->swapTotal = size_tmp_buf;
        } else if (strcmp(buf, SWAPFREE) == 0) {
            memUsageBuf->swapUse = size_tmp_buf;
        } else {
            continue;
        }
    }
    memUsageBuf->memUse = memUsageBuf->memTotal - memUsageBuf->memUse;
    memUsageBuf->swapUse = memUsageBuf->swapTotal - memUsageBuf->swapUse;
    fclose(fp);

    if (memUsageBuf->memTotal <= 0){ // Memory Capacity (Free, Total) Reading ERROR
        exception(-2, "get_Memory_Usage", "Memory Size"); 
        memUsageBuf->memUse = 0; // Initialization (Reset)
        memUsageBuf->memTotal = 0;
        memUsageBuf->swapUse = 0;
        memUsageBuf->swapTotal = 0;
        return;
    }
}