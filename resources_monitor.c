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
    short ifaCnt, contCnt;
    IFASpeed* ifaSpeedBuf = NULL;
    DockerInfo* contBuf = NULL;
    get_Date();
    // for (int i = 0; i < contCnt; i++) {
    //     for (int j = 0; j < contBuf[i].ifaCount; j++) { 
    //         printf("%s %s | ", contBuf[i].vethName[j], contBuf[i].ipv4_addr[j]);
    //         // printf("%s %s | ", contBuf[i].vethName[j], contBuf[i].ipv4_addr[j]);
    //     }
    //     printf("\n");
    // }


    while(1){
        get_IFA_Speed(&contBuf, &ifaSpeedBuf, &ifaCnt, &contCnt);
        for (int i = 0; i < ifaCnt; i++){
            printf("%s %s %.2f %.2f %ld %ld %ld %ld\n", ifaSpeedBuf[i].ifa_name, ifaSpeedBuf[i].ipv4_addr, ifaSpeedBuf[i].speedRX, ifaSpeedBuf[i].speedTX, ifaSpeedBuf[i].errorRX, ifaSpeedBuf[i].errorTX,
            ifaSpeedBuf[i].dropRX, ifaSpeedBuf[i].dropTX);
        }
        for (int i = 0; i < 0; i++){
            printf("\n");
        }
        sleep(1);
    }

    // if (containerInfo != NULL){ // After convert veth to container name, remove allocated array.
//         for (int i = 0; i < containerCnt; i++) {
//             for (int j = 0; j < containerInfo[i].ifaCount; j++){ // Free String array
//                 free_Array((void**)&(containerInfo[i].vethName[j])); 
//                 free_Array((void**)&(containerInfo[i].ipv4_addr[j])); 
//             }
//             free_Array((void**)&(containerInfo[i].checked));
//             free_Array((void**)&(containerInfo[i].vethName));
//             free_Array((void**)&(containerInfo[i].ipv4_addr));
//             free_Array((void**)&(containerInfo[i].ifa_index));
//         }
//         free_Array((void**)containerInfo);
//     }
}