#include "zz_struct.h"
#include "common.h"
#include "info_to_log.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

DateInfo dateBuf;

int main(void){
    get_Date();

    if (check_Log_Directory(HISTORY_PATH, 0750) == -1){ // Check the presense of "/var/log/00_Server_Monitoring/00_history" directory. (History file is saved to this.)
        exception(-4, "check_Log_Directory", "ERROR Creating Log Directory");
        return -1;
    }
    
    write_Temperature_to_Log();

}