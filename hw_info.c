#include "common.h"
#include "hw_info.h"
#include "os_info.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

extern Unit_Mapping unitMap[];

/* Functions that get disk information from perccli */
int get_VDisk_Information_from_Perccli(VDInfo** vdBuf, int* virtualDriveCnt) {
    // vdBuf -> Pointer of float Array. Array is made in this function.
    FILE* perccli_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' }, statusBuf[BUF_MAX_LINE], accessBuf[BUF_MAX_LINE], capUnitBuf[BUF_MAX_LINE];
    char* targetPos = NULL;

    if ((perccli_ptr = popen(GET_DISK_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_Disk_Information_from_Perccli", "Perccli - Disk");
        return -100;
    }

    while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) {
        if ((targetPos = strstr(lineBuf, VD_CNT)) != NULL) { // Extract the number of virtual disk
            sscanf(targetPos += strlen(VD_CNT), "%d", virtualDriveCnt);
            break;
        }
    }

    pclose(perccli_ptr);

    *vdBuf = (VDInfo*)malloc((*virtualDriveCnt) * sizeof(VDInfo));
    for (int i = 0; i < *virtualDriveCnt; i++) { // Initialization
        (*vdBuf)[i].driveGroup = -100;
        (*vdBuf)[i].virtualDrive = -100;
        (*vdBuf)[i].type[0] = '\0';
        (*vdBuf)[i].status = -100;
        (*vdBuf)[i].access = -100;
        (*vdBuf)[i].capacity[0] = '\0';
        (*vdBuf)[i].vdName[0] = '\0';
        (*vdBuf)[i].fileSystem[0] = '\0';
        (*vdBuf)[i].mountPathCnt = -100;
        (*vdBuf)[i].mountPath = NULL;
    }

    if ((perccli_ptr = popen(GET_VDISK_LIST, "r")) == NULL) {
        exception(-2, "get_VDisk_OSDrive_Filesystem_from_Perccli", "Perccli - VDisk");
        return -100;
    }

    while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) {
        if ((targetPos = strstr(lineBuf, VD_LIST)) != NULL) { // Extract the number of virtual disk
            fgets(lineBuf, sizeof(lineBuf), perccli_ptr); // =========
            fgets(lineBuf, sizeof(lineBuf), perccli_ptr); // \n
            fgets(lineBuf, sizeof(lineBuf), perccli_ptr); // ----------------------------------------------------------------
            fgets(lineBuf, sizeof(lineBuf), perccli_ptr); // DG/VD TYPE  State Access Consist Cache Cac sCC       Size Name
            fgets(lineBuf, sizeof(lineBuf), perccli_ptr); // ----------------------------------------------------------------
            for (int i = 0; i < *virtualDriveCnt; i++) {
                fgets(lineBuf, sizeof(lineBuf), perccli_ptr);
                sscanf(lineBuf, VD_LIST_FORM, &((*vdBuf)[i].driveGroup), &((*vdBuf)[i].virtualDrive), (*vdBuf)[i].type, statusBuf, accessBuf, 
                (*vdBuf)[i].capacity, capUnitBuf, (*vdBuf)[i].vdName);

                strcat((*vdBuf)[i].capacity, " "); // Union capacity and its Unit.
                strcat((*vdBuf)[i].capacity, capUnitBuf);

                if (strcmp(statusBuf, VD_STATE_OPTIMAL) == 0) { // Convert Status String to numeric code.
                    (*vdBuf)[i].status = TYPE_VD_STATE_OPTIMAL;
                } else if (strcmp(statusBuf, VD_STATE_DEGRADED) == 0) {
                    (*vdBuf)[i].status = TYPE_VD_STATE_DEGRADED;
                } else if (strcmp(statusBuf, VD_STATE_OFFLINE) == 0) {
                    (*vdBuf)[i].status = TYPE_VD_STATE_OFFLINE;
                } else if (strcmp(statusBuf, VD_STATE_RECOVERY) == 0) {
                    (*vdBuf)[i].status = TYPE_VD_STATE_RECOVERY;
                } else if (strcmp(statusBuf, VD_STATE_PARTIALLY_DEGRADED) == 0) {
                    (*vdBuf)[i].status = TYPE_VD_STATE_PARTIALLY_DEGRADED;
                } else if (strcmp(statusBuf, VD_STATE_HIDDEN) == 0) {
                    (*vdBuf)[i].status = TYPE_VD_STATE_HIDDEN;
                } else if (strcmp(statusBuf, VD_STATE_TRANSPORTREADY) == 0) {
                    (*vdBuf)[i].status = TYPE_VD_STATE_TRANSPORTREADY;
                } else {
                    (*vdBuf)[i].status = -100;
                }

                if (strcmp(accessBuf, VD_ACCESS_READ_ONLY) == 0) { // Convert Access String to numeric code.
                    (*vdBuf)[i].access = TYPE_VD_ACCESS_READ_ONLY;
                } else if (strcmp(accessBuf, VD_ACCESS_READ_WRITE) == 0) {
                    (*vdBuf)[i].access = TYPE_VD_ACCESS_READ_WRITE;
                } else if (strcmp(accessBuf, VD_ACCESS_BLOCKED) == 0) {
                    (*vdBuf)[i].access = TYPE_VD_ACCESS_BLOCKED;
                } else {
                    (*vdBuf)[i].access = -100;
                }
            }
        }
    }

    pclose(perccli_ptr);

    get_VDisk_FileSystem_from_Perccli(*vdBuf, *virtualDriveCnt);
    for (int i = 0 ; i < *virtualDriveCnt; i++){
        get_MountPath_from_FileSystem((*vdBuf)[i].fileSystem, &((*vdBuf)[i].mountPath), &((*vdBuf)[i].mountPathCnt));
    }

    return 0;
}

void get_VDisk_FileSystem_from_Perccli(VDInfo* vdBuf, int virtualDiskCnt) {
    FILE* perccli_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' }, vdProperties[BUF_MAX_LINE] = { '\0' };
    char* targetPos = NULL;

    if ((perccli_ptr = popen(GET_VDISK_FILESYSTEM, "r")) == NULL) {
        exception(-2, "get_VDisk_FileSystem_from_Perccli", "Perccli - VDisk");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) {
        for (int i = 0; i < virtualDiskCnt; i++) {
            sprintf(vdProperties, VD_PROPERTIES_FORM, i);
            if ((targetPos = strstr(lineBuf, vdProperties)) != NULL) {
                while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) {
                    if ((targetPos = strstr(lineBuf, VD_FILESYSTEM_FORM)) != NULL) {
                        sscanf(targetPos + strlen(VD_FILESYSTEM_FORM), "%s", vdBuf[i].fileSystem);
                        break;
                    }
                }
            }
        }
    }

    pclose(perccli_ptr);
}

int get_Disk_Information_from_Perccli(DiskInfo** diskBuf, int* diskCount){
    // diskBuf -> Pointer of float Array. Array is made in this function.
    FILE* perccli_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' }, capUnitBuf[BUF_MAX_LINE], statusBuf[BUF_MAX_LINE], driveGroupBuf[BUF_MAX_LINE];
    char* targetPos = NULL;

    if ((perccli_ptr = popen(GET_DISK_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_Disk_Information_from_Perccli", "Perccli - Disk");
        return -100;
    }

    while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) {
        if ((targetPos = strstr(lineBuf, DISK_PHYSICAL_COUNT)) != NULL) { // Extract the number of physical disk
            sscanf(targetPos += strlen(DISK_PHYSICAL_COUNT), "%d", diskCount);
            break;
        }
    }

    *diskBuf = (DiskInfo*)malloc((*diskCount) * sizeof(DiskInfo));

    for (int i = 0; i < *diskCount; i++) { // Initialization
        (*diskBuf)[i].enclosureNum = -100;
        (*diskBuf)[i].slotNum = -100; 
        (*diskBuf)[i].deviceID = -100;
        (*diskBuf)[i].driveGroup = -100;
        (*diskBuf)[i].status = -100;
        (*diskBuf)[i].modelName[0] = '\0';
        (*diskBuf)[i].capacity[0] = '\0';
        (*diskBuf)[i].mediaType[0] = '\0';
        (*diskBuf)[i].interface[0] = '\0';
        (*diskBuf)[i].mappedPartition[0] = '\0';
    }

    while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) { // Extract disk information
        if (strstr(lineBuf, DISK_PD_LIST) != NULL) {
            fgets(lineBuf, sizeof(lineBuf), perccli_ptr); // =========
            fgets(lineBuf, sizeof(lineBuf), perccli_ptr); // \n
            fgets(lineBuf, sizeof(lineBuf), perccli_ptr); // ----------------------------------------------------------------
            fgets(lineBuf, sizeof(lineBuf), perccli_ptr); // EID:Slt DID State DG    Size Intf Med SED PI SeSz Model  Sp Type
            fgets(lineBuf, sizeof(lineBuf), perccli_ptr); // ----------------------------------------------------------------
            for (int i = 0; i < *diskCount; i++){ // Extract disk information (Content)
                fgets(lineBuf, sizeof(lineBuf), perccli_ptr);
                sscanf(lineBuf, "%hd:%hd %hd %s %s %s %s %s %s", &((*diskBuf)[i].enclosureNum), &((*diskBuf)[i].slotNum), &((*diskBuf)[i].deviceID), statusBuf, driveGroupBuf, 
                (*diskBuf)[i].capacity, capUnitBuf, (*diskBuf)[i].interface, (*diskBuf)[i].mediaType);

                if (strcmp(statusBuf, DISK_STATE_ONLINE) == 0) { // Convert "state" String to numeric code.
                    (*diskBuf)[i].status = TYPE_DISK_STATE_ONLINE;
                } else if (strcmp(statusBuf, DISK_STATE_OFFLINE) == 0) {
                    (*diskBuf)[i].status = TYPE_DISK_STATE_OFFLINE;
                } else if (strcmp(statusBuf, DISK_STATE_GHS) == 0) {
                    (*diskBuf)[i].status = TYPE_DISK_STATE_GHS;
                } else if (strcmp(statusBuf, DISK_STATE_DHS) == 0) {
                    (*diskBuf)[i].status = TYPE_DISK_STATE_DHS;
                } else if (strcmp(statusBuf, DISK_STATE_UGOOD) == 0) {
                    (*diskBuf)[i].status = TYPE_DISK_STATE_UGOOD;
                } else if (strcmp(statusBuf, DISK_STATE_UBAD) == 0) {
                    (*diskBuf)[i].status = TYPE_DISK_STATE_UBAD;
                } else if (strcmp(statusBuf, DISK_STATE_SANI) == 0) {
                    (*diskBuf)[i].status = TYPE_DISK_STATE_SANI;
                } else {
                    (*diskBuf)[i].status = -100;
                }

                strcat((*diskBuf)[i].capacity, " "); // Union capacity and its Unit.
                strcat((*diskBuf)[i].capacity, capUnitBuf); // Union capacity and its Unit.

                (*diskBuf)[i].driveGroup = ((strcmp(driveGroupBuf, "-") == 0) ? -100 : atoi(driveGroupBuf)); // Convert "driveGroup" String to numeric code.

            }
        }
    }

    pclose(perccli_ptr);

    get_Disk_Product_Name_from_Perccli(*diskBuf, *diskCount);

    return 0;
}

void get_Disk_Product_Name_from_Perccli(DiskInfo* diskBuf, int diskCount){ // Get disk product name
    FILE* perccli_ptr = NULL;
    char lineBuf[BUF_MAX_LINE], searchStr[BUF_MAX_LINE];
    char* targetPos = NULL;

    if ((perccli_ptr = popen(GET_DISK_NAME, "r")) == NULL) {
        exception(-2, "get_Disk_Product_Name_from_Percli", "Perccli - Disk");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) {
        if (strstr(lineBuf, DISK_NAME_FILTER) != NULL){
            for (int i = 0; i < diskCount; i++){
                sprintf(searchStr, DISK_NAME_FORM, diskBuf[i].enclosureNum, diskBuf[i].slotNum);
                if (strstr(lineBuf, searchStr) != NULL) { // Extract the product name of disk
                    while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) {
                        if ((targetPos = strstr(lineBuf, DISK_NAME)) != NULL){
                            sscanf(targetPos += strlen(DISK_NAME), "%[^\n]s", diskBuf[i].modelName);
                            remove_Space_of_Tail(diskBuf[i].modelName);
                            break;
                        }
                    }
                }
            }
        }
    }

    pclose(perccli_ptr);
}

/* Functions that get HBA Card information from perccli */
int get_HBA_Information_from_Perccli(HBAInfo* HBABuf){ // Get HBA Card information from Perccli command.
    FILE* perccli_ptr = NULL;
    char lineBuf[BUF_MAX_LINE];
    char* targetPos = NULL;

    HBABuf->HBA_Name[0] = '\0'; // Initialization
    HBABuf->HBA_Bios_Ver[0] = '\0';
    HBABuf->HBA_Serial_Num[0] = '\0';
    HBABuf->HBA_Firmware_Ver[0] = '\0';
    HBABuf->HBA_Driver_Name[0] = '\0';
    HBABuf->HBA_Driver_Ver[0] = '\0';
    HBABuf->status = -100;
    HBABuf->HBA_Cur_Personality = -100;
    HBABuf->HBA_Drive_Groups_Cnt = -100;
    HBABuf->bbuStatus.status = -100;
    HBABuf->bbuStatus.voltage[0] = '\0';
    HBABuf->bbuStatus.designCapacity[0] = '\0';
    HBABuf->bbuStatus.fullCapacity[0] = '\0';
    HBABuf->bbuStatus.remainCapacity[0] = '\0';

    if ((perccli_ptr = popen(GET_DISK_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_HBA_Information_from_Perccli", "Perccli - HBA Card");
        return -100;
    }

    while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) {
        if (((targetPos = strstr(lineBuf, HBA_PRODUCTNAME)) != NULL) && (strlen(HBABuf->HBA_Name) == 0)) { // Extract HBA Card model name
            sscanf(targetPos + strlen(HBA_PRODUCTNAME), "%[^\n]s", HBABuf->HBA_Name);

        } else if ((targetPos = strstr(lineBuf, HBA_BIOS_VER)) != NULL) { // Extract HBA Card BIOS Version
            sscanf(targetPos + strlen(HBA_BIOS_VER), "%[^\n]s", HBABuf->HBA_Bios_Ver);

        } else if ((targetPos = strstr(lineBuf, HBA_SERIAL_NUMBER)) != NULL) { // Extract HBA Card serial number
            sscanf(targetPos + strlen(HBA_SERIAL_NUMBER), "%[^\n]s", HBABuf->HBA_Serial_Num);

        } else if ((targetPos = strstr(lineBuf, HBA_FIRMWARE_VER)) != NULL) { // Extract HBA Card firmware version
            sscanf(targetPos + strlen(HBA_FIRMWARE_VER), "%[^\n]s", HBABuf->HBA_Firmware_Ver);

        } else if ((targetPos = strstr(lineBuf, HBA_DRIVER_NAME)) != NULL){ // Extract HBA Card driver name
            sscanf(targetPos + strlen(HBA_DRIVER_NAME), "%[^\n]s", HBABuf->HBA_Driver_Name);

        } else if ((targetPos = strstr(lineBuf, HBA_DRIVER_VER)) != NULL) { // Extract HBA Card driver version
            sscanf(targetPos + strlen(HBA_DRIVER_VER), "%[^\n]s", HBABuf->HBA_Driver_Ver);

        } else if ((targetPos = strstr(lineBuf, HBA_CONTROLLER_STATUS)) != NULL) { // Extract HBA Card Status
            sscanf(targetPos + strlen(HBA_CONTROLLER_STATUS), "%[^\n]s", lineBuf);
            if (strcmp(lineBuf, HBA_CONTROLLER_STATUS_OPTIMAL) == 0) {
                HBABuf->status = TYPE_STATUS_OPTIMAL;
            } else if (strcmp(lineBuf, HBA_CONTROLLER_STATUS_DEGRADED) == 0) {
                HBABuf->status = TYPE_STATUS_DEGRADED;
            } else if (strcmp(lineBuf, HBA_CONTROLLER_STATUS_FAILED) == 0){
                HBABuf->status = TYPE_STATUS_FAILED;
            } else {
                HBABuf->status = -100;
            }

        } else if ((targetPos = strstr(lineBuf, HBA_CURRENT_PERSONALITY)) != NULL) { // Extract current mode of HBA Card (RAID or HBA)
            sscanf(targetPos + strlen(HBA_CURRENT_PERSONALITY), "%[^\n]s", lineBuf);
            if (strncmp(lineBuf, HBA_CURRENT_PERSONALITY_HBA, strlen(HBA_CURRENT_PERSONALITY_HBA)) == 0) {
                HBABuf->HBA_Cur_Personality = TYPE_HBA_CUR_HBA;
            } else if (strncmp(lineBuf, HBA_CURRENT_PERSONALITY_RAID, strlen(HBA_CURRENT_PERSONALITY_RAID)) == 0) {
                HBABuf->HBA_Cur_Personality = TYPE_HBA_CUR_RAID;
            } else {
                HBABuf->HBA_Cur_Personality = -100;
            }

        } else if ((targetPos = strstr(lineBuf, HBA_DRIVE_GROUPS)) != NULL) { // Extract the number of drive groups
            sscanf(targetPos + strlen(HBA_DRIVE_GROUPS), "%hd", &(HBABuf->HBA_Drive_Groups_Cnt));
        }
    }

    pclose(perccli_ptr);

    get_BBU_Information_from_Perccli(&(HBABuf->bbuStatus));

    return 0;
}

void get_BBU_Information_from_Perccli(BBUInfo* BBUBuf){ // Get BBU(Backup Battery Unit) information from Perccli command.
    FILE* perccli_ptr = NULL;
    char lineBuf[BUF_MAX_LINE];
    char* targetPos = NULL;

    if ((perccli_ptr = popen(GET_BBU_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_BBU_Information_from_Perccli", "Perccli - BBU");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) {
        if ((targetPos = strstr(lineBuf, BBU_STATUS)) != NULL) { // Extract BBU Status
            targetPos += strlen(BBU_STATUS);
            remove_Space_of_Head(targetPos);
            sscanf(targetPos, "%[^\n]s", lineBuf);
            if (strncmp(lineBuf, BBU_STATUS_OPTIMAL, strlen(BBU_STATUS_OPTIMAL)) == 0) {
                BBUBuf->status = TYPE_STATUS_OPTIMAL;
            } else if (strncmp(lineBuf, BBU_STATUS_DEGRADED, strlen(BBU_STATUS_DEGRADED)) == 0) {
                BBUBuf->status = TYPE_STATUS_DEGRADED;
            } else if (strncmp(lineBuf, BBU_STATUS_FAILED, strlen(BBU_STATUS_FAILED)) == 0) {
                BBUBuf->status = TYPE_STATUS_FAILED;
            }

        } else if (((targetPos = strstr(lineBuf, BBU_VOLTAGE)) != NULL) && (strstr(lineBuf, BBU_VOLTAGE_UNIT) != NULL) && (strlen(BBUBuf->voltage) == 0)) { // Extract BBU Voltage
            targetPos += strlen(BBU_VOLTAGE);
            remove_Space_of_Head(targetPos); // Remove the space at the head of String.
            sscanf(targetPos, "%[^\n]s", lineBuf);
            remove_Space_of_Tail(lineBuf); // Remove the space at the tail of String.
            strcpy(BBUBuf->voltage, lineBuf);

        } else if (((targetPos = strstr(lineBuf, BBU_DESIGN_CAPACITY)) != NULL) && (strstr(lineBuf, BBU_CAPACITY_UNIT) != NULL)) { // Extract design capacity of BBU (mAh)
            targetPos += strlen(BBU_DESIGN_CAPACITY);
            remove_Space_of_Head(targetPos); // Remove the space at the head of String.
            sscanf(targetPos, "%[^\n]s", lineBuf);
            remove_Space_of_Tail(lineBuf); // Remove the space at the tail of String.
            strcpy(BBUBuf->designCapacity, lineBuf);

        } else if (((targetPos = strstr(lineBuf, BBU_FULL_CHARGE_CAPACITY)) != NULL) && (strstr(lineBuf, BBU_CAPACITY_UNIT) != NULL)) { // Extract full charged capacity of BBU (mAh)
            targetPos += strlen(BBU_FULL_CHARGE_CAPACITY);
            remove_Space_of_Head(targetPos); // Remove the space at the head of String.
            sscanf(targetPos, "%[^\n]s", lineBuf);
            remove_Space_of_Tail(lineBuf); // Remove the space at the tail of String.
            strcpy(BBUBuf->fullCapacity, lineBuf);

        } else if (((targetPos = strstr(lineBuf, BBU_REMAINING_CAPACITY)) != NULL) && (strstr(lineBuf, BBU_CAPACITY_UNIT) != NULL) && (strstr(lineBuf, "Alarm") == NULL)) { // Extract remaining capacity of BBU (mAh)
            targetPos += strlen(BBU_REMAINING_CAPACITY);
            remove_Space_of_Head(targetPos); // Remove the space at the head of String.
            sscanf(targetPos, "%[^\n]s", lineBuf);
            remove_Space_of_Tail(lineBuf); // Remove the space at the tail of String.
            strcpy(BBUBuf->remainCapacity, lineBuf);
        }
    }
    
    pclose(perccli_ptr);
}

/* Functions that get CPU information from Jiffies (/proc/cpuinfo) */
void get_CPU_Usage_Percent_of_each_Core(float** usage_buf_ptr){ // Get CPU usage percent of each Core from Jiffies.
    // usage_buf_ptr -> Pointer of float Array. Array is made in this function.
    unsigned long user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guest_nice = 0;
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