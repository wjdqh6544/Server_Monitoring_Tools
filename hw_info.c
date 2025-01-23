#include "0_usrDefine.h"
#include "common.h"
#include "hw_info.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

// int get_Disk_Information_from_Perccli(){

// }

void get_CPU_Usage_Percent_of_each_Core(float** usage_buf_ptr){ // Get CPU usage percent of each Cor from Jiffies.
    // usage_buf_ptr -> Pointer of float Array. Array is made in this function.
    unsigned long user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guest_nice = 0; // 10코어에 대한 priv 값 저장필요.
    static unsigned long* priv_total = NULL;
    static unsigned long* priv_idle = NULL;
    FILE *fp = NULL;
    
    if (priv_total == NULL){
        priv_total = (unsigned long*)calloc(get_CPU_Core_Count(), sizeof(unsigned long));
    }

    if (priv_idle == NULL){
        priv_idle = (unsigned long*)calloc(get_CPU_Core_Count(), sizeof(unsigned long));
    }

    if (*usage_buf_ptr != NULL) { // If array is already created.
        free(*usage_buf_ptr); // Free the memory allocated for the existing array.
        *usage_buf_ptr = NULL;
    }

    *usage_buf_ptr = (float*)malloc(get_CPU_Core_Count() * sizeof(float)); // Allocate new memory.

    for (int i = 0; i < get_CPU_Core_Count(); (*usage_buf_ptr)[i++] = -100); // Initialization

    if ((fp = fopen(STAT_LOCATION, "r")) == NULL) {
        exception(-1, "get_CPU_Usage_Percent_of_each_Core", STAT_LOCATION);
        return;
    }

    fscanf(fp, STAT_FORM, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice); // Jiffies of total core
    for (int i = 0; i < get_CPU_Core_Count(); i++){
        fscanf(fp, STAT_FORM, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice); // Jiffies of each core
        // Calculate CPU Usage (now - priv jiffies)
        (*usage_buf_ptr)[i] = 100 * (1 - ((idle - priv_idle[i]) / (float)((user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice) - priv_total[i]))); 
        priv_total[i] = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice; // Save now jiffies
        priv_idle[i] = idle;
    }

    fclose(fp);
}

int get_Physical_CPU_Count(int totalCore) { // Get the number of physical CPU installed to server. 
    FILE* fp = NULL;
    int id = 0, tmp = 0;
    char fullpath[MAX_PHYSICAL_CPU_PATH_LEN] = { '\0' };
    for (int i = 0; i < totalCore; i++){
        sprintf(fullpath, PHYSICAL_CPU_PATH_FORM, i);
        if ((fp = fopen(fullpath, "r")) == NULL) {
            exception(-1, "get_Physical_CPU_Count", fullpath);
            return -100;
        }

        fscanf(fp, "%d", &tmp);
        id = (id < tmp) ? tmp : id;
        
        fclose(fp);
    }
    return id + 1;
}

int get_CPU_Core_Count(){ /* Get the number of all CPU Cores. 
    For example, A server has 2 physical CPU and each CPU has 10 core, then this function returns "20")    */

    DIR *dir_ptr = NULL;
    int core_cnt = 0;
    char* token_ptr = NULL;
    struct dirent *dirent_ptr = NULL;

    if ((dir_ptr = opendir(PHYSICAL_CPU_PATH)) != NULL) { // Open "/sys/devices/system/cpu"
        while ((dirent_ptr = readdir(dir_ptr)) != NULL) {
            if (strncmp(dirent_ptr->d_name, "cpu", 3) == 0) { // Check cpu_ form (_ is CPU core.)
                token_ptr = (dirent_ptr->d_name) + 3;
                if (isdigit(*token_ptr) != 0) { // At cpu_, Check _ is number.
                    core_cnt = (core_cnt < atoi(token_ptr)) ? atoi(token_ptr) : core_cnt;
                }
            }
        }

        closedir(dir_ptr);

    } else {
        exception(-1, "get_CPU_Count", PHYSICAL_CPU_PATH);
        return -100;
    }

    return core_cnt + 1;
}