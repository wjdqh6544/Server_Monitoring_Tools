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
    FileInfo* fileInfo = NULL;
    get_Date();
    int cnt = 0;
    get_File_Information(&fileInfo, &cnt);
    
    for (int i = 0; i < cnt; i++) {
        printf("%s (%d, %d, %d) (%d, %d, %d) (%d, %o, %o)\n", fileInfo[i].path, fileInfo[i].changed[0], fileInfo[i].ownerUID[0], fileInfo[i].ownerUID[1],
        fileInfo[i].changed[1], fileInfo[i].groupGID[0], fileInfo[i].groupGID[1], fileInfo[i].changed[2], fileInfo[i].permission[0], fileInfo[i].permission[1]);
    }

}