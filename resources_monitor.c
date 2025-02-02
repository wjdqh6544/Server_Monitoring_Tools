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
    WarningLog* warningList = NULL;
    int cnt = 0;
    get_Date();
    get_Warning_History_from_Log(&warningList, &cnt);
    for (int i = 0; i < cnt; i++) {
        printf("%d %04hd-%02hd-%02hd %02hd:%02hd:%02hd\n",
        i + 1, warningList[i].date.year, warningList[i].date.month, warningList[i].date.day, warningList[i].date.hrs, warningList[i].date.min, warningList[i].date.sec);
    }

    printf("%d\n", remove_History_Log(LOG_TYPE_WARNING));
}