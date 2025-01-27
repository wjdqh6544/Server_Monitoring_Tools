#include "0_usrDefine.h"
#include "common.h"
#include "hw_info.h"
#include "info_from_log.h"
#include "os_info.h"
#include "zz_struct.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

DateInfo dateBuf;

int main(void){
    SystemInfo systemBuf;
    get_Date();
    get_System_Information_from_Omreport(&systemBuf);
    for (int i = 0; i < MAX_PSU_COUNT; i++){
        printf("%hd ", systemBuf.psuStatus[i]);
    }
    
}