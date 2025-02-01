#include "0_usrDefine.h"
#include "common.h"
#include "hw_info.h"
#include "info_from_log.h"
#include "os_info.h"
#include "zz_struct.h"
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern const DateInfo dateBuf;

int main(void){
    ProcessInfo* psBuf = NULL;
    int cnt = 10;
    get_Date();
    get_Process_Status(&psBuf, cnt);
    for (int i = 0; i < cnt; i++) {
        printf("%s %d %4.1f %4.1f %ld  %s  %s  %s  %s\n",
        psBuf[i].userName, psBuf[i].pid, psBuf[i].cpu, psBuf[i].mem, psBuf[i].memUseSize, psBuf[i].tty, psBuf[i].start, psBuf[i].time, psBuf[i].command);
    }
}