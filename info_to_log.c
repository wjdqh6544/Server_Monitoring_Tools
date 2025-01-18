#include "0_usrDefine.h"
#include "common.h"
#include "info_to_log.h"
#include "zz_struct.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern DateInfo dateBuf;

void* write_Temperature_to_Log(){ // Write temperature information to Log file.
    int fd = -1;
    TempLog tempBuf;
    char fullpath[MAX_LOG_PATH_LEN] = { '\0' };

    while(1){
        get_Filename(fullpath, HISTORY_PATH, TEMP_LOG, &dateBuf); // Destination: /var/log/00_Server_Monitoring/00_history/temperature_history-YYYYMMDD
        if ((fd = open(fullpath, O_WRONLY | O_CREAT | O_APPEND, 0640)) == -1){
            exception(-1, "write_Temperature_to_Log", fullpath);
            return NULL;
        }

        tempBuf.date = dateBuf;
        get_Temperature(&(tempBuf.temp));

        if (write(fd, &tempBuf, sizeof(TempLog)) != sizeof(TempLog)) {
            exception(-3, "write_Temperature_to_Log", "<Writed data size> != sizeof(TempLog)");
        }
        close(fd); 
        sleep(1);   
    }
    return NULL;
}

void* write_Usage_to_Log(){ // Write usage information to Log file.
    int fd = -1;
    UsageLog usageBuf;
    char fullpath[MAX_LOG_PATH_LEN] = { '\0' };

    while(1){
        get_Filename(fullpath, HISTORY_PATH, USAGE_LOG, &dateBuf); // Destination: /var/log/00_Server_Monitoring/00_history/usage_history-YYYYMMDD
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
        sleep(1);
    }
    return NULL;
}

void* write_Warning_to_Log(){ // Write temperature and usage to Log If the value is more than or equal to the critical point.
    int fd = -1;
    WarningLog warningBuf;
    char fullpath[MAX_LOG_PATH_LEN] = { '\0' };

    while(1){
        get_Filename(fullpath, HISTORY_PATH, WARNING_LOG, &dateBuf); // Destination: /var/log/00_Server_Monitoring/00_history/warning_history-YYYYMM
        if ((fd = open(fullpath, O_WRONLY | O_CREAT | O_APPEND, 0640)) == -1) {
            exception(-1, "write_Warning_to_Log", fullpath);
            return NULL;
        }

        get_Temperature(&(warningBuf.temp));
        get_CPU_Usage(&(warningBuf.cpuUsage));
        get_Memory_Usage(&(warningBuf.memUsage));

        // Temperature and usage data is written to Log file If the value that is more than or equal to the critical point is exist.
        if (over_Critical_Point(&warningBuf) == 1) { 
            warningBuf.date = dateBuf;
            if (write(fd, &warningBuf, sizeof(WarningLog)) != sizeof(WarningLog)) {
                exception(-3, "write_Warning_to_Log", "<Writed data size> != sizeof(WarningLog)");
            }
        }
        close(fd);
        sleep(1);
    }
    return NULL;
}

void get_Temperature(TempInfo* tempBuf){ // For getting temperature, Initialize storage value.
    tempBuf->inlet = -100; // Initialization; -100 means "N/A"
    tempBuf->exhaust = -100;
    for (int i = 0; i < MAX_CPU_COUNT; tempBuf->cpu[i++] = -100);
    tempBuf->raidCore = -100;
    tempBuf->raidController = -100;
    tempBuf->storage_cnt = 0;
    for (int i = 0; i < MAX_STORAGE_COUNT; tempBuf->storage[i++] = -100);

    get_Temperature_Omreport(tempBuf);
    get_Temperature_Perccli(tempBuf);
}

void get_Temperature_Omreport(TempInfo* tempBuf){ // Get Temperature Information from omreport
    FILE* omreport_ptr = NULL;
    char lineBuf[TEMP_MAX_LINE] = { '\0' };
    char* targetPos = NULL;

    if ((omreport_ptr = popen(GET_TEMP_OMREPORT_COMMAND, "r")) == NULL) { // Get raw data from omreport
        exception(-2, "get_Temperature_Omreport", "omreport");
        return;
    }
    
    while (fgets(lineBuf, sizeof(lineBuf), omreport_ptr) != NULL){
        if ((targetPos = strstr(lineBuf, INLET_TEMP)) != NULL){ // Extract Inlet temperature
            fgets(lineBuf, sizeof(lineBuf), omreport_ptr);
            targetPos = strstr(lineBuf, ": ");
            sscanf(targetPos, ": %hd.0 C", &(tempBuf->inlet));
        } else if ((targetPos = strstr(lineBuf, EXHAUST_TEMP)) != NULL){ // Extract Exhaust temperature
            fgets(lineBuf, sizeof(lineBuf), omreport_ptr);
            targetPos = strstr(lineBuf, ": ");
            sscanf(targetPos, ": %hd.0 C", &(tempBuf->exhaust));
        } else if ((targetPos = strstr(lineBuf, CPU1_TEMP)) != NULL){ // Extract CPU1 Package temperature
            fgets(lineBuf, sizeof(lineBuf), omreport_ptr);
            targetPos = strstr(lineBuf, ": ");
            sscanf(targetPos, ": %hd.0 C", &(tempBuf->cpu[0]));
        } else if ((targetPos = strstr(lineBuf, CPU2_TEMP)) != NULL){ // Extract CPU2 Package temperature
            fgets(lineBuf, sizeof(lineBuf), omreport_ptr);
            targetPos = strstr(lineBuf, ": ");
            sscanf(targetPos, ": %hd.0 C", &(tempBuf->cpu[1]));
        } 
    }

    pclose(omreport_ptr);
}

void get_Temperature_Perccli(TempInfo* tempBuf){
    FILE* perccli_ptr = NULL;
    char lineBuf[TEMP_MAX_LINE] = { '\0' };
    char* targetPos = NULL;

    if ((perccli_ptr = popen(GET_RAID_TEMP_PERCCLI_COMMAND, "r")) == NULL){ // Get raw data of a RAID Card from perccli
        exception(-2, "get_Temperature_Perccli", "Perccli");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) {
        if ((targetPos = strstr(lineBuf, RAID_CORE_TEMP)) != NULL) { // Extract RAID core temperature
            sscanf(targetPos + strlen(RAID_CORE_TEMP), "%hd", &(tempBuf->raidCore));
        } else if ((targetPos = strstr(lineBuf, RAID_CTRL_TEMP))) { // Extract RAID controller temperature
            sscanf(targetPos + strlen(RAID_CTRL_TEMP), "%hd", &(tempBuf->raidController));
        }
    }

    pclose(perccli_ptr);

    if ((perccli_ptr = popen(GET_STORAGE_TEMP_PERCCLI_COMMAND, "r")) == NULL){ // Get raw data of a RAID Card from perccli
        exception(-2, "get_Temperature_Perccli", "Perccli");
        return;
    }

    for (short i = 0; (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL); i++) {
        sscanf(lineBuf, STORAGE_TEMP_FORM, &(tempBuf->storage[i])); // Extract RAID core temperature
        tempBuf->storage_cnt = (short)(i + 1);
    }

    pclose(perccli_ptr);
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

int over_Critical_Point(WarningLog* warningBuf) { /* Check whether the data is more than or equal to the critical point.
    If both temperature and usage is more than or equal to the critical point, The usage is first.
    That is, when this situation, the value of "type" in struct variable is "1".
    To see the value of "type" in struct variable, See define macro. (In 0_usrDefine.h, Warning Log)
    */

    if (get_Memory_Usage_Percent(warningBuf->memUsage.memTotal, warningBuf->memUsage.memUse) >= MEM_USAGE_CRITICAL_PERCENT) {
        warningBuf->type = TYPE_MEM_USAGE;
        return 1;
    }

    if (warningBuf->cpuUsage.usage >= CPU_USAGE_CRITICAL_PERCENT) {
        warningBuf->type = TYPE_CPU_USAGE;
        return 1;
    }

    for (int i = 0; i < MAX_CPU_COUNT; i++){
        if (warningBuf->temp.cpu[i] >= CPU_TEMP_CRITICAL_POINT) {
            warningBuf->type = TYPE_CPU_TEMP;
            return 1;
        }
    }

    for (int i = 0; i < MAX_STORAGE_COUNT; i++){
        if (warningBuf->temp.storage[i] >= STORAGE_TEMP_CRITICAL_POINT) {
            warningBuf->type = TYPE_STORAGE_TEMP;
            return 1;
        }
    }

    if (warningBuf->temp.raidCore >= RAID_CORE_TEMP_CRITICAL_POINT) {
        warningBuf->type = TYPE_RAID_CORE_TEMP;
        return 1;
    }

    if (warningBuf->temp.raidController >= RAID_CTRL_TEMP_CRITICAL_POINT) {
        warningBuf->type = TYPE_RAID_CTRL_TEMP;
        return 1;
    }

    if (warningBuf->temp.exhaust >= EXHAUST_TEMP_CRITICAL_POINT) {
        warningBuf->type = TYPE_EXHAUST_TEMP;
        return 1;
    }

    if (warningBuf->temp.inlet >= INLET_TEMP_CRITICAL_POINT) {
        warningBuf->type = TYPE_INLET_TEMP;
        return 1;
    }

    return 0;
}