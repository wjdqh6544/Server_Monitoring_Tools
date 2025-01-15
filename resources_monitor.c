#include "common.h"
#include "info_to_log.h"
#include "zz_struct.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

DateInfo dateBuf;

int main(void){
    char buf[1000];
    int fd1, fd2;
    TempLog tempBuf;
    UsageLog usageBuf;
    get_Date();
    strcpy(buf, HISTORY_PATH);
    get_Filename(buf, HISTORY_PATH, TEMP_LOG, &dateBuf);
    fd1 = open(buf, O_RDONLY);
    strcpy(buf, HISTORY_PATH);
    get_Filename(buf, HISTORY_PATH, USAGE_LOG, &dateBuf);
    fd2 = open(buf, O_RDONLY);

    while(1){
        lseek(fd1, -sizeof(TempLog), SEEK_END);
        lseek(fd2, -sizeof(UsageLog), SEEK_END);
        read(fd1, &tempBuf, sizeof(tempBuf));
        read(fd2, &usageBuf, sizeof(usageBuf));
        printf("%2d %2d %.2f %.2f %.2f %.2f %f %ld %ld %ld %ld\n\n", tempBuf.date.day, usageBuf.date.day, tempBuf.temp.inlet, tempBuf.temp.exhaust, tempBuf.temp.cpu[0], tempBuf.temp.cpu[1], usageBuf.cpu.usage, 
        usageBuf.mem.memTotal, usageBuf.mem.memUse, usageBuf.mem.swapTotal, usageBuf.mem.swapUse);
        sleep(1);
    }
    
}