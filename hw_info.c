#include "common.h"
#include "hw_info.h"
#include "os_info.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

/* List of functions of using to get server information (System Parts include CPU, Memory, PSU, Network, and so on.) */
void get_System_Information_from_Omreport(SystemInfo* systemBuf){
    // Start initializaiton
    STR_INIT(systemBuf->hostname);
    STR_INIT(systemBuf->serverModel);
    STR_INIT(systemBuf->serviceTag);
    STR_INIT(systemBuf->serviceCode);

    for (int i = 0; i < MAX_CPU_COUNT; i++){
        STR_INIT(systemBuf->cpu[i].name);
        systemBuf->cpu[i].status = -100;
        systemBuf->cpu[i].coreCnt = -100;
    }

    systemBuf->mem.slotsTotal = -100;
    systemBuf->mem.slotsUsed = -100;
    STR_INIT(systemBuf->mem.installedCapacity);
    STR_INIT(systemBuf->mem.errorCorrection);
    systemBuf->mem.unit = NULL;

    STR_INIT(systemBuf->cmosBattery.name);
    systemBuf->cmosBattery.status = -100;

    for (int i = 0; i < MAX_FAN_COUNT; i++){
        systemBuf->fan[i].status = -100;
        STR_INIT(systemBuf->fan[i].rpm);
        STR_INIT(systemBuf->fan[i].name);
    }

    systemBuf->ifaCount = -100;
    systemBuf->ifa = NULL;

    for (int i = 0; i < MAX_PSU_COUNT; i++){
        systemBuf->psuStatus[i] = -100;
    }
    // Finish initialization

    get_Server_Information_from_Omreport(systemBuf->hostname, systemBuf->serverModel, systemBuf->serviceTag, systemBuf->serviceCode);
    get_CPU_Information_from_Omreport(systemBuf->cpu);
    get_Memory_Information_from_Omreport(&(systemBuf->mem));
    get_CMOS_Battery_Status_from_Omreport(&(systemBuf->cmosBattery));
    get_Fan_Status_from_Omreport(systemBuf->fan);
    get_Physical_IFA_Information_from_Omreport(&(systemBuf->ifa), &(systemBuf->ifaCount));
    get_PSU_Status_from_Omreport(systemBuf->psuStatus);
}

void get_Server_Information_from_Omreport(char* hostname, char* serverModel, char* serviceTag, char* serviceCode){ // Get Server Information: Hostname, Service Tag, Service Code (for DELL)
    FILE* omreport_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' };
    char* targetPos = NULL;

    if ((omreport_ptr = popen(GET_SERVER_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_Server_Information_from_Omreport", "Omreport - Server Info.");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), omreport_ptr) != NULL) { // Extract Hostname, Server Model, Service Tag, Service Code
        if ((targetPos = strstr(lineBuf, INFO_HOSTNAME)) != NULL) {
            sscanf(targetPos + strlen(INFO_HOSTNAME), INFO_FORM_STR, hostname);
        } else if ((targetPos = strstr(lineBuf, INFO_SERVER_MODEL)) != NULL) {
            sscanf(targetPos + strlen(INFO_SERVER_MODEL), INFO_FORM_STR, serverModel);
        } else if ((targetPos = strstr(lineBuf, INFO_SERVICE_TAG)) != NULL) {
            sscanf(targetPos + strlen(INFO_SERVICE_TAG), INFO_FORM_STR, serviceTag);
        } else if ((targetPos = strstr(lineBuf, INFO_SERVICE_CODE)) != NULL) {
            sscanf(targetPos + strlen(INFO_SERVICE_CODE), INFO_FORM_STR, serviceCode);
        }
    }

    pclose(omreport_ptr);
}

void get_CPU_Information_from_Omreport(CPUInfo* cpuBuf){ // Get CPU Information: CPU Name, Status, Core Count
    FILE* omreport_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' }, valueBuf[BUF_MAX_LINE];
    char* targetPos = NULL;
    int index = 0, existCPU = 1;

    if ((omreport_ptr = popen(GET_CPU_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_CPU_Information_from_Omreport", "Omreport - CPU");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), omreport_ptr) != NULL) { 
        if ((targetPos = strstr(lineBuf, INFO_INDEX)) != NULL) { // Extract index of CPU
            sscanf(targetPos + strlen(INFO_INDEX), " : %d", &index);
        } else if ((targetPos = strstr(lineBuf, INFO_STATUS)) != NULL) { // Check whether CPU is occupied. Status is Unknown (CPU is not occupied)
            sscanf(targetPos + strlen(INFO_STATUS), INFO_FORM_STR, valueBuf);
            existCPU = ((strcmp(valueBuf, INFO_STATUS_UNKNOWN) == 0) ? 0 : 1);
        }
        
        if (existCPU == 0) { // CPU is not occupied.
            continue;
        }
        
        if ((targetPos = strstr(lineBuf, INFO_CPU_PROCESSOR_NAME)) != NULL) { // Extract CPU Name
            sscanf(targetPos + strlen(INFO_CPU_PROCESSOR_NAME), INFO_FORM_STR, cpuBuf[index].name);
        } else if ((targetPos = strstr(lineBuf, INFO_CPU_STATE)) != NULL) { // Extract CPU State
            sscanf(targetPos + strlen(INFO_CPU_STATE), INFO_FORM_STR, valueBuf);
            cpuBuf[index].status = ((strcmp(valueBuf, INFO_STATE_PRESENT) == 0) ? TYPE_STATUS_OK : TYPE_STATUS_CRITICAL); // CPU State: OK or Not OK
        } else if ((targetPos = strstr(lineBuf, INFO_CPU_CORE_CNT)) != NULL) { // Extract the number of CPU Core.
            sscanf(targetPos + strlen(INFO_CPU_CORE_CNT), INFO_FORM_NUM_SHORT, &(cpuBuf[index].coreCnt));
        }
    }

    pclose(omreport_ptr);
}

void get_Memory_Information_from_Omreport(MEMInfo* memoryBuf){ // Get Memory Information: RAM Slot (Available / Used), RAM Size (installed), ErrorCollection, Each unit information
    FILE* omreport_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' }, valueBuf[BUF_MAX_LINE];
    char* targetPos = NULL;
    int detail = 0, idx = 0;

    if ((omreport_ptr = popen(GET_MEM_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_Memory_Information_from_Omreport", "Omreport - Memory");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), omreport_ptr) != NULL) {
        if (detail == 0) {
            if ((targetPos = strstr(lineBuf, INFO_MEM_INSTALLED_CAP)) != NULL) { // Extract installed memory capacity
                sscanf(targetPos + strlen(INFO_MEM_INSTALLED_CAP), INFO_FORM_STR, memoryBuf->installedCapacity);
            } else if ((targetPos = strstr(lineBuf, INFO_MEM_SLOT_TOTAL)) != NULL) { // Extract the number of total memory slots 
                sscanf(targetPos + strlen(INFO_MEM_SLOT_TOTAL), INFO_FORM_NUM_SHORT, &(memoryBuf->slotsTotal));
            } else if ((targetPos = strstr(lineBuf, INFO_MEM_SLOT_USED)) != NULL) { // Extract the number of used memory slots
                sscanf(targetPos + strlen(INFO_MEM_SLOT_USED), INFO_FORM_NUM_SHORT, &(memoryBuf->slotsUsed));

                free_Array((void**)memoryBuf->unit); // If allocated Array exists, Free the memory allocated for the existing array.
                if ((memoryBuf->unit = (MEM_UNIT_INFO*)malloc((memoryBuf->slotsUsed) * sizeof(MEM_UNIT_INFO))) == NULL) {
                    exception(-4, "get_Memory_Information_from_Omreport", "malloc() - memoryBuf->unit");
                    return;
                }

                for (int i = 0; i < memoryBuf->slotsUsed; i++){ // Initialization
                    memoryBuf->unit[i].status = -100;
                    STR_INIT(memoryBuf->unit[i].connectorName);
                    STR_INIT(memoryBuf->unit[i].type);
                    STR_INIT(memoryBuf->unit[i].capacity);
                }
            } else if ((targetPos = strstr(lineBuf, INFO_MEM_ERROR_COLLECTION)) != NULL) { // Extract error correction method
                sscanf(targetPos + strlen(INFO_MEM_ERROR_COLLECTION), INFO_FORM_STR, memoryBuf->errorCorrection);
                detail = 1;
            }
        } else { // detail != 0
            if (idx < memoryBuf->slotsUsed) {
                if ((targetPos = strstr(lineBuf, INFO_INDEX)) != NULL) { // Extract memory unit index
                    sscanf(targetPos + strlen(INFO_INDEX), INFO_FORM_NUM, &idx);
                } else if ((targetPos = strstr(lineBuf, INFO_STATUS)) != NULL) { // Extract memroy unit status
                    sscanf(targetPos + strlen(INFO_STATUS), INFO_FORM_STR, valueBuf);
                    memoryBuf->unit[idx].status = ((strcmp(valueBuf, INFO_STATUS_OK) == 0) ? TYPE_STATUS_OK : TYPE_STATUS_CRITICAL); // Memory Unit status: OK or not OK
                } else if ((targetPos = strstr(lineBuf, INFO_MEM_UNIT_CONNECTOR)) != NULL) { // Extract connector name embedded memory unit 
                    sscanf(targetPos + strlen(INFO_MEM_UNIT_CONNECTOR), INFO_FORM_STR, memoryBuf->unit[idx].connectorName);
                } else if ((targetPos = strstr(lineBuf, INFO_MEM_UNIT_TYPE)) != NULL) { // Extract memory unit type (ex. DDR4)
                    sscanf(targetPos + strlen(INFO_MEM_UNIT_TYPE), INFO_FORM_STR, memoryBuf->unit[idx].type);
                } else if ((targetPos = strstr(lineBuf, INFO_MEM_UNIT_SIZE)) != NULL) { // Extract memory unit size
                    sscanf(targetPos + strlen(INFO_MEM_UNIT_SIZE), INFO_FORM_STR, memoryBuf->unit[idx].capacity);
                    idx = ((idx == 3) ? 4 : 3);
                }
            } else {
                break;
            }
        }
    }

    pclose(omreport_ptr);
}

void get_CMOS_Battery_Status_from_Omreport(CMOS_BAT_INFO* cmosBatteryBuf){ // Get CMOS Battery status: Good or else
    FILE* omreport_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' };
    char* targetPos = NULL;

    if ((omreport_ptr = popen(GET_CMOS_BATTERY_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_CMOS_Battery_Status_from_Omreport", "Omreport - Batteries");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), omreport_ptr) != NULL){
        if ((targetPos = strstr(lineBuf, INFO_NAME)) != NULL) { // Extract battery name
            sscanf(targetPos + strlen(INFO_NAME), INFO_FORM_STR, cmosBatteryBuf->name);
        } else if ((targetPos = strstr(lineBuf, INFO_READING)) != NULL) { // Extract battery status
            sscanf(targetPos + strlen(INFO_READING), INFO_FORM_STR, lineBuf);
            cmosBatteryBuf->status = ((strcmp(lineBuf, INFO_CMOS_BATTERY_GOOD) == 0) ? TYPE_STATUS_OK : TYPE_STATUS_CRITICAL); // Battery status: OK or not
        }
    }

    pclose(omreport_ptr);
}

void get_Fan_Status_from_Omreport(FANInfo* fanBuf){ // Get fan status: Fan name, rpm, status
    FILE* omreport_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' };
    char* targetPos = NULL;
    int idx = 0;

    if ((omreport_ptr = popen(GET_FAN_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_FAN_Status_from_Omreport", "Omreport - Fans");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), omreport_ptr) != NULL){
        if ((targetPos = strstr(lineBuf, INFO_INDEX)) != NULL) { // Extract index name
            sscanf(targetPos + strlen(INFO_INDEX), INFO_FORM_NUM, &idx);
        } else if ((targetPos = strstr(lineBuf, INFO_STATUS)) != NULL) { // Extract fan status
            sscanf(targetPos + strlen(INFO_STATUS), INFO_FORM_STR, lineBuf);
            fanBuf[idx].status = ((strcmp(lineBuf, INFO_STATUS_OK) == 0) ? TYPE_STATUS_OK : TYPE_STATUS_CRITICAL); // Fan status: OK or not
        } else if ((targetPos = strstr(lineBuf, INFO_NAME)) != NULL) { // Extract fan name
            sscanf(targetPos + strlen(INFO_NAME), INFO_FORM_STR, fanBuf[idx].name);
        } else if ((targetPos = strstr(lineBuf, INFO_READING)) != NULL) { // Extract fan RPM
            sscanf(targetPos + strlen(INFO_READING), INFO_FORM_STR, fanBuf[idx].rpm);
        }
    }

    pclose(omreport_ptr);
}

void get_Physical_IFA_Information_from_Omreport(PHYSICAL_IFA_Info** ifaBuf, int* ifaCount){ // Get network interface information: interface name (Linux, Card model name), speed, connected / disconnected
    FILE* omreport_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' };
    char* targetPos = NULL;
    int idx = 0;

    *ifaCount = -100;

    if ((omreport_ptr = popen(GET_NICS_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_Physical_IFA_Information_from_Omreport", "Omreport - nics, count");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), omreport_ptr) != NULL){ // Get the number of physical network interface
        if ((targetPos = strstr(lineBuf, INFO_INDEX)) != NULL) { 
            sscanf(targetPos + strlen(INFO_INDEX), INFO_FORM_NUM, ifaCount); // *ifaCount -> Index
        } else if (strstr(lineBuf, INFO_NICS_TEAM_INTERFACES) != NULL) {
            break;
        }
    }

    (*ifaCount)++;

    pclose(omreport_ptr);

    free_Array((void**)ifaBuf); // If allocated Array exists, Free the memory allocated for the existing array.
    if ((*ifaBuf = (PHYSICAL_IFA_Info*)malloc((*ifaCount) * sizeof(PHYSICAL_IFA_Info))) == NULL) { // Create Array of interface
        exception(-4, "get_Physical_IFA_Information_from_Omreport", "malloc() - ifaBuf");
        return;
    }

    for (int i = 0; i < *ifaCount; i++){ // Initialization
        STR_INIT((*ifaBuf)[i].name);
        STR_INIT((*ifaBuf)[i].ifName);
        STR_INIT((*ifaBuf)[i].speed);
        (*ifaBuf)[i].connected = -100;
    }

    if ((omreport_ptr = popen(GET_NICS_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_Physical_IFA_Information_from_Omreport", "Omreport - nics, info");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), omreport_ptr) != NULL){ 
        if ((targetPos = strstr(lineBuf, INFO_INDEX)) != NULL) { // Extract index of each network interface.
            sscanf(targetPos + strlen(INFO_INDEX), INFO_FORM_NUM, &idx);
        } else if ((targetPos = strstr(lineBuf, INFO_NICS_INTERFACE_NAME)) != NULL) { // Extract interface name in Linux
            sscanf(targetPos + strlen(INFO_NICS_INTERFACE_NAME), INFO_FORM_STR, (*ifaBuf)[idx].name);
        } else if ((targetPos = strstr(lineBuf, INFO_NICS_DESCRIPTION)) != NULL) { // Extract network card name
            sscanf(targetPos + strlen(INFO_NICS_DESCRIPTION), INFO_FORM_STR, (*ifaBuf)[idx].ifName);
        } else if ((targetPos = strstr(lineBuf, INFO_NICS_CON_STATUS)) != NULL) { // Extract connection status. (Connected or Disconnected)
            sscanf(targetPos + strlen(INFO_NICS_CON_STATUS), INFO_FORM_STR, lineBuf);
            (*ifaBuf)[idx].connected = ((strcmp(lineBuf, INFO_NICS_CONNECT) == 0) ? TYPE_NICS_CONNECT : TYPE_NICS_DISCONNECT); // convert status to numerical code.
        }

        if (strstr(lineBuf, INFO_NICS_TEAM_INTERFACES) != NULL) {
            break;
        }
    }

    pclose(omreport_ptr);

    get_Interface_Speed_from_Omreport(*ifaBuf, *ifaCount);
}

void get_Interface_Speed_from_Omreport(PHYSICAL_IFA_Info* ifaBuf, int ifaCount){ // Get speed of interface
    FILE* omreport_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' }, searchBuf[BUF_MAX_LINE];
    char* targetPos = NULL;
    int physical_ifa = 0;

    for (int i = 0; i < (int)ifaCount; i++){
        sprintf(lineBuf, GET_NICS_INFO_COMMAND_DETAIL, i); // Get command (Detail information of each network interface)

        if ((omreport_ptr = popen(lineBuf, "r")) == NULL){
            exception(-2, "get_Interface_Speed_from_Omreport", "Omreport - nics");
            return;
        }

        while (fgets(lineBuf, sizeof(lineBuf), omreport_ptr) != NULL){
            if (physical_ifa == 0) {
                sprintf(searchBuf, INFO_NICS_DETAIL_INTERFACE, ifaBuf[i].name); // Get physical interface name (in Linux, ex. eno1)
                physical_ifa = ((strstr(lineBuf, searchBuf) != NULL) ? 1 : 0); // Find physical interface section.
            } else { // physical_ifa != 0 -> Success to find physical interface section.
                if ((targetPos = strstr(lineBuf, INFO_NICS_DETAIL_SPEED)) != NULL) {
                    fgets(lineBuf, sizeof(lineBuf), omreport_ptr);
                    sscanf(lineBuf + strlen(INFO_NICS_DETAIL_VALUE), INFO_FORM_STR, ifaBuf[i].speed);
                    break;
                }
            }
        }

        pclose(omreport_ptr);
    }
}

void get_PSU_Status_from_Omreport(int* psuStatusBuf){ // Get status of Power Supplies.
    FILE* omreport_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' };
    char* targetPos = NULL;
    int idx = 0;

    if ((omreport_ptr = popen(GET_PSU_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_PSU_Status_from_Omreport", "Omreport - Pwrsupplies");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), omreport_ptr) != NULL){
        if ((targetPos = strstr(lineBuf, INFO_INDEX)) != NULL) { // Extract index
            sscanf(targetPos + strlen(INFO_INDEX), INFO_FORM_NUM, &idx);
            fgets(lineBuf, sizeof(lineBuf), omreport_ptr); // Extract PSU Status
            sscanf(targetPos + strlen(INFO_STATUS), INFO_FORM_STR, lineBuf);
            psuStatusBuf[idx] = ((strcmp(lineBuf, INFO_STATUS_OK) == 0) ? TYPE_STATUS_OK : TYPE_STATUS_CRITICAL); // Battery status: OK or not
        }
    }

    pclose(omreport_ptr);
}

/* List of functions of using to get disk information */
int get_VDisk_Information_from_Perccli(VDInfo** vdBuf, int* virtualDriveCnt){ // Get virtual disks information from Perccli command.
    // vdBuf -> Pointer of float Array. Array is made in this function.
    FILE* perccli_ptr = NULL;
    static int priv_vdCnt = 0;
    char lineBuf[BUF_MAX_LINE] = { '\0' }, statusBuf[BUF_MAX_LINE], accessBuf[BUF_MAX_LINE], capUnitBuf[BUF_MAX_LINE];
    char* targetPos = NULL;

    if ((perccli_ptr = popen(GET_DISK_INFO_COMMAND, "r")) == NULL) {
        exception(-2, "get_VDisk_Information_from_Perccli", "Perccli - Disk");
        return -100;
    }

    while (fgets(lineBuf, sizeof(lineBuf), perccli_ptr) != NULL) {
        if ((targetPos = strstr(lineBuf, VD_CNT)) != NULL) { // Extract the number of virtual disk
            sscanf(targetPos += strlen(VD_CNT), "%d", virtualDriveCnt);
            break;
        }
    }

    pclose(perccli_ptr);

    if (*vdBuf != NULL){ // If allocated Array exists.
        for (int i = 0; i < priv_vdCnt; i++) { // Free mountPath string
            for (int j = 0; j < (*vdBuf)[i].mountPathCnt; j++){
                free_Array((void**)&((*vdBuf)[i].mountPath[j])); 
            }
            free_Array((void**)&((*vdBuf)[i].mountPath));
        }
        free_Array((void**)vdBuf); // Free the memory allocated for the existing array.
    }

    if ((*vdBuf = (VDInfo*)malloc((*virtualDriveCnt) * sizeof(VDInfo))) == NULL) {
        exception(-4, "get_VDisk_Information_from_Perccli", "malloc() - vdBuf");
        return -100;
    }

    for (int i = 0; i < *virtualDriveCnt; i++) { // Initialization
        (*vdBuf)[i].driveGroup = -100;
        (*vdBuf)[i].virtualDrive = -100;
        (*vdBuf)[i].status = -100;
        (*vdBuf)[i].access = -100;
        STR_INIT((*vdBuf)[i].type);
        STR_INIT((*vdBuf)[i].capacity);
        STR_INIT((*vdBuf)[i].vdName);
        STR_INIT((*vdBuf)[i].fileSystem);
        (*vdBuf)[i].mountPathCnt = -100;
        (*vdBuf)[i].mountPath = NULL;
    }

    priv_vdCnt = *virtualDriveCnt;

    if ((perccli_ptr = popen(GET_VDISK_LIST, "r")) == NULL) {
        exception(-2, "get_VDisk_Information_from_Perccli", "Perccli - VDisk");
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

void get_VDisk_FileSystem_from_Perccli(VDInfo* vdBuf, int virtualDiskCnt){ // Get the fileSystem path connected to Virtual disk.
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

int get_Disk_Information_from_Perccli(DiskInfo** diskBuf, int* diskCount){ // Get disk information: Capacity, Disk ID, Status, and so on.
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

    free_Array((void**)diskBuf); // If allocated Array exists, Free the memory allocated for the existing array.
    if ((*diskBuf = (DiskInfo*)malloc((*diskCount) * sizeof(DiskInfo))) == NULL) {
        exception(-4, "get_Disk_Information_from_Perccli", "malloc() - diskBuf");
        return -100;
    }
    for (int i = 0; i < *diskCount; i++) { // Initialization
        (*diskBuf)[i].enclosureNum = -100;
        (*diskBuf)[i].slotNum = -100; 
        (*diskBuf)[i].deviceID = -100;
        (*diskBuf)[i].driveGroup = -100;
        (*diskBuf)[i].status = -100;
        STR_INIT((*diskBuf)[i].modelName);
        STR_INIT((*diskBuf)[i].capacity);
        STR_INIT((*diskBuf)[i].mediaType);
        STR_INIT((*diskBuf)[i].interface);
        STR_INIT((*diskBuf)[i].mappedPartition);
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
                sscanf(lineBuf, DISK_INFO_FORM, &((*diskBuf)[i].enclosureNum), &((*diskBuf)[i].slotNum), &((*diskBuf)[i].deviceID), statusBuf, driveGroupBuf, 
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

/* List of functions of using to get HBA Card information */
int get_HBA_Information_from_Perccli(HBAInfo* HBABuf){ // Get HBA Card information from Perccli command.
    FILE* perccli_ptr = NULL;
    char lineBuf[BUF_MAX_LINE];
    char* targetPos = NULL;

    STR_INIT(HBABuf->HBA_Name); // Initialization
    STR_INIT(HBABuf->HBA_Bios_Ver);
    STR_INIT(HBABuf->HBA_Serial_Num);
    STR_INIT(HBABuf->HBA_Firmware_Ver);
    STR_INIT(HBABuf->HBA_Driver_Name);
    STR_INIT(HBABuf->HBA_Driver_Ver);
    STR_INIT(HBABuf->bbuStatus.voltage);
    STR_INIT(HBABuf->bbuStatus.designCapacity);
    STR_INIT(HBABuf->bbuStatus.fullCapacity);
    STR_INIT(HBABuf->bbuStatus.remainCapacity);
    HBABuf->status = -100;
    HBABuf->HBA_Cur_Personality = -100;
    HBABuf->HBA_Drive_Groups_Cnt = -100;
    HBABuf->bbuStatus.status = -100;

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
            sscanf(targetPos + strlen(HBA_DRIVE_GROUPS), "%d", &(HBABuf->HBA_Drive_Groups_Cnt));
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

/* List of functions of using to get CPU information from Jiffies (/proc/cpuinfo) */
void get_CPU_Usage_Percent_of_each_Core(float** usage_buf_ptr){ // Get CPU usage percent of each Core from Jiffies.
    // usage_buf_ptr -> Pointer of float Array. Array is made in this function.
    size_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guest_nice = 0;
    static size_t* priv_total = NULL, *priv_idle = NULL;
    FILE *fp = NULL;
    
    if (priv_total == NULL){
        if ((priv_total = (size_t*)calloc(get_CPU_Core_Count(), sizeof(size_t))) == NULL) {
            exception(-4, "get_CPU_Usage_Percent_of_each_Core", "calloc() - priv_total");
            return;
        }
    }

    if (priv_idle == NULL){
        if ((priv_idle = (size_t*)calloc(get_CPU_Core_Count(), sizeof(size_t))) == NULL) {
            exception(-4, "get_CPU_Usage_Percent_of_each_Core", "calloc() - priv_idle");
            return;
        }
    }

    if (*usage_buf_ptr == NULL) {
        if ((*usage_buf_ptr = (float*)malloc(get_CPU_Core_Count() * sizeof(float))) == NULL) { // Allocate new memory.
            exception(-4, "get_CPU_Usage_Percent_of_each_Core", "malloc() - usage_buf_ptr");
            return;
        }
    }

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

int get_Physical_CPU_Count(int totalCore){ // Get the number of physical CPU installed to server. 
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