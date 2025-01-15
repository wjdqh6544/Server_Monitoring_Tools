#include "zz_struct.h"
#include "0_usrDefine.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern DateInfo dateBuf;

void get_Temperature_Omreport(TempInfo* tempBuf){
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

void* write_Temperature_to_Log(){ // Write temperature information to Log file.
    int fd = -1;
    TempLog tempBuf;
    char fullpath[MAX_LOG_PATH_LEN] = { '\0' };

    get_Filename(HISTORY_PATH, fullpath, TEMP_LOG, &dateBuf); // destination: /var/log/00_Server_Monitoring/00_history/Temperature_history-YYYYMMDD
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