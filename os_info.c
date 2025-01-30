#include "common.h"
#include "os_info.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

extern DateInfo dateBuf;

/* Functions that get network interface information (from /proc/net/dev) */
short get_IFA_Speed(DockerInfo** containerInfo, IFASpeed** ifa, short* ifaCount, short* containerCnt){ // Get receive and transmit speed of network interfaces.
    FILE *fp = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' };
    static size_t *priv_byteRX = NULL, *priv_byteTX = NULL;
    size_t byteRX = 0, byteTX = 0;
    
    *ifaCount = -100;

    if ((fp = fopen("/proc/net/dev", "r")) == NULL) {
        return -100;
    }

    *ifaCount = 0;
    fgets(lineBuf, sizeof(lineBuf), fp); // Inter-|   Receive                                                |  Transmit
    fgets(lineBuf, sizeof(lineBuf), fp); //  face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    for ((*ifaCount) = 0; fgets(lineBuf, sizeof(lineBuf), fp) != NULL; (*ifaCount)++); // Get the number of network interfaces.
    
    fseek(fp, 0, SEEK_SET);
    fgets(lineBuf, sizeof(lineBuf), fp); // Inter-|   Receive                                                |  Transmit
    fgets(lineBuf, sizeof(lineBuf), fp); //  face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed

    if (priv_byteRX == NULL){
        if ((priv_byteRX = (size_t*)calloc((*ifaCount), sizeof(size_t))) == NULL) {
            exception(-4, "get_IFA_Speed", "calloc() - priv_byteRX");
            return -100;
        }
    }

    if (priv_byteTX == NULL){
        if ((priv_byteTX = (size_t*)calloc((*ifaCount), sizeof(size_t))) == NULL) {
            exception(-4, "get_IFA_Speed", "calloc() - priv_byteTX");
            return -100;
        }
    }

    if (*ifa == NULL){ // If array not allocated, allocate array. (When first invoked)
        if ((*ifa = (IFASpeed*)malloc((*ifaCount) * sizeof(IFASpeed))) == NULL) {
            exception(-4, "get_IFA_Speed", "malloc() - ifaBuf");
            return -100;
        }
    }

    for (int i = 0; i < *ifaCount; i++) { // Initialization
        STR_INIT((*ifa)[i].ipv4_addr);
        STR_INIT((*ifa)[i].ifa_name);
        (*ifa)[i].speedRX = -100;
        (*ifa)[i].speedTX = -100;
        (*ifa)[i].errorRX = 0;
        (*ifa)[i].errorTX = 0;
        (*ifa)[i].dropRX = 0;
        (*ifa)[i].dropTX = 0;
    }

    get_Docker_Container_Information(containerInfo, containerCnt); // Get docker container information (container name, veth name and its IPv4 Address)

    for (int i = 0; fscanf(fp, NET_DEV_FORM, (*ifa)[i].ifa_name, &byteRX, &((*ifa)[i].errorRX), &((*ifa)[i].dropRX), &byteTX, &((*ifa)[i].errorTX), &((*ifa)[i].dropTX)) != EOF; i++) {
        (*ifa)[i].ifa_name[strlen((*ifa)[i].ifa_name) - 1] = '\0';
        (*ifa)[i].speedRX = (byteRX - priv_byteRX[i]) / (float)1024; // Calculate speed and Convert unit to "KB"
        (*ifa)[i].speedTX = (byteTX - priv_byteTX[i]) / (float)1024; // Calculate speed and Convert unit to "KB"

        get_IPv4_Addr((*ifa)[i].ifa_name, (*ifa)[i].ipv4_addr); // Get IPv4 Address

        if (STRN_CMP_EQUAL((*ifa)[i].ifa_name, CHECK_BRIDGE_FORM)) {
            convert_BridgeID_to_Name((*ifa)[i].ifa_name);
        }

        if (STRN_CMP_EQUAL((*ifa)[i].ifa_name, DOCKER_VETH_INTERFACE_PREFIX)) {
            convert_Veth_to_Container_Info(*containerInfo, &((*ifa)[i]), *containerCnt);
        }

        priv_byteRX[i] = byteRX;
        priv_byteTX[i] = byteTX;
    }

    fclose(fp);

    sort_Docker_Network(*ifa, *ifaCount);

    return 0;
}

void get_IPv4_Addr(const char* ifa_name, char* ipv4_addr){ // Get IPv4 address of a network interface.
    struct ifaddrs *ifa;
    struct sockaddr_in *addr = NULL;
    char errorBuf[ERROR_MSG_LEN] = { '\0' };
    if (getifaddrs(&ifa) == -1) { // Extract network interface list of System
        exception(-4, "get_IPv4_Addr", "getifaddrs()");
        return;
    }

    for (; ifa != NULL; ifa = ifa->ifa_next){
        if (ifa->ifa_addr == NULL){ // IPv4 Address is not specified.
            continue;
        }
        if ((strcmp(ifa->ifa_name, ifa_name) == 0) && (ifa->ifa_addr->sa_family == AF_INET)) { // sa_family == IPv4
            addr = (struct sockaddr_in *)ifa->ifa_addr; // socketaddr_in sturct: contain short sin_family(AF_INET), u_short sin_port, struct in_addr sin_addr->(u_long)s_addr;
            if (inet_ntop(AF_INET, &addr->sin_addr, ipv4_addr, IPV4_LEN) == NULL) { // Convert (u_long) s_addr Numeric information to IPv4 String
                sprintf(errorBuf, "%s - %s", "inet_ntop()", ifa->ifa_name);
                exception(-4, "get_IPv4_Addr", errorBuf);
                return;
            }
        }
    }
}

void get_Docker_Container_Information(DockerInfo** containerInfo, short* containerCnt){ // Get docker container information: name and ipv4 address corresponding to veth (vethxxxxxx: virtual interface of container)
    /* 
    containerInfo: Must be 'not' allocated to memory. (At this function, docker container information list is allocated to memory.)
    So, Caller invokes this function with pointer that initialized with "NULL"
    
    Working Flow
    1. Get Docker container name. (ex. example_Container)
    2. Get a pid of each docker container. (ex. 4000)
    3. Get network interface index of docker container pid. (ex. /proc/4000/net/igmp -> 17 eth0 ...)
    4. Get a virtual network interface (veth) name relative to extracted network interface index. (ex. vethxxxxxx@if17)
    5. Get ipv4 address of container.
    */
    FILE* docker_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' }, commandBuf[BUF_MAX_LINE];
    int tmpIndex = 0;

    (*containerCnt) = -100;

    if ((docker_ptr = popen(GET_DOCKER_CONTAINER_NAME, "r")) == NULL) { // Get the number of docker container
        exception(-2, "get_Docker_Container_Information", "docker ps");
        return;
    }

    (*containerCnt) = 0;

    while (fgets(lineBuf, sizeof(lineBuf), docker_ptr) != NULL){
        (*containerCnt)++;
    }

    if (*containerInfo == NULL) { // Allocate array for veth, container Name, and IPv4 information
        *containerInfo = (DockerInfo*)malloc((*containerCnt) * sizeof(DockerInfo));

        for (int i = 0; i < *containerCnt; i++) { // Initialization
            (*containerInfo)[i].checked = NULL;
            STR_INIT((*containerInfo)[i].containerName);
            (*containerInfo)[i].pid = 0;
            (*containerInfo)[i].ifaCount = -100;
            (*containerInfo)[i].vethName = NULL;
            (*containerInfo)[i].ipv4_addr = NULL;
            (*containerInfo)[i].ifa_index = NULL;
        }
    }

    if ((docker_ptr = popen(GET_DOCKER_CONTAINER_NAME, "r")) == NULL) { // 1. Get docker container name
        exception(-2, "get_Docker_Container_Information", "docker ps");
        return;
    }

    for (int i = 0; fgets((*containerInfo)[i].containerName, sizeof(((*containerInfo)[i].containerName)), docker_ptr) != NULL; i++); 

    pclose(docker_ptr);

    for (int i = 0; i < *containerCnt; i++){ // 2. Get pid of each docker container 
        (*containerInfo)[i].containerName[strlen((*containerInfo)[i].containerName) - 1] = '\0'; // remove "\n"
        sprintf(commandBuf, GET_DOCKER_INSPECT_CONTAINER, (*containerInfo)[i].containerName);
        if ((docker_ptr = popen(commandBuf, "r")) == NULL) { 
            sprintf(lineBuf, "docker inspect - %s", (*containerInfo)[i].containerName);
            exception(-2, "get_Docker_Container_Information", lineBuf);
            return;
        }

        fscanf(docker_ptr, "%d", &((*containerInfo)[i].pid));

        pclose(docker_ptr);
    }

    for (int i = 0; i < *containerCnt; i++) { // 3. Get veth ifa index at each container
        sprintf(commandBuf, IGMP_LOCATION, (*containerInfo)[i].pid);
        if ((docker_ptr = fopen(commandBuf, "r'")) == NULL) { 
            exception(-1, "get_Docker_Container_Information", commandBuf);
            return;
        }

        (*containerInfo)[i].ifaCount = 0;

        while (fgets(lineBuf, sizeof(lineBuf), docker_ptr) != NULL) { // Get the number of interface in /proc/${pid}/net/igmp
            (*containerInfo)[i].ifaCount++;
        }

        (*containerInfo)[i].ifaCount = (((*containerInfo)[i].ifaCount - 1) / 2) - 1; 

        (*containerInfo)[i].checked = (short*)calloc((*containerInfo)[i].ifaCount, sizeof(short)); // Create array for checking status
        (*containerInfo)[i].vethName = (char**)malloc(((*containerInfo)[i].ifaCount) * sizeof(char*)); // Create 2D array for veth name.
        (*containerInfo)[i].ipv4_addr = (char**)malloc(((*containerInfo)[i].ifaCount) * sizeof(char*)); // Create 2D array for IPv4 of veth.
        (*containerInfo)[i].ifa_index = (short*)malloc(((*containerInfo)[i].ifaCount) * sizeof(short)); // Create array for veth interface index.
        for (int j = 0; j < (*containerInfo)[i].ifaCount; j++) { // Initialization
            (*containerInfo)[i].vethName[j] = (char*)malloc(MAX_DOCKER_CONTAINER_NAME_LEN * sizeof(char));
            (*containerInfo)[i].ipv4_addr[j] = (char*)malloc(IPV4_LEN * sizeof(char));
            STR_INIT((*containerInfo)[i].vethName[j]);
            STR_INIT((*containerInfo)[i].ipv4_addr[j]);
            (*containerInfo)[i].ifa_index[j] = -100;
        } 

        fseek(docker_ptr, 0, SEEK_SET);
        for (int j = 0; fgets(lineBuf, sizeof(lineBuf), docker_ptr) != NULL; ) { // Extract interface index.
            if (isdigit(lineBuf[0]) != 0) {
                sscanf(lineBuf, "%hd", &((*containerInfo)[i].ifa_index[j]));
                j = ((*containerInfo)[i].ifa_index[j] == 1) ? j : j + 1;
            }
        }

        fclose(docker_ptr);
    }

    for (int i = 0; i < *containerCnt; i++) { // 4. Get veth name relative to ifa index
        for (int j = 0; j < (*containerInfo)[i].ifaCount; j++) {
            sprintf(commandBuf, GET_DOCKER_VETH_NAME, (*containerInfo)[i].ifa_index[j]);
            if ((docker_ptr = popen(commandBuf, "r")) == NULL) { 
                sprintf(lineBuf, "ip -br addr - index(%d)", (*containerInfo)[i].ifa_index[j]);
                exception(-2, "get_Docker_Container_Information", lineBuf);
                return;
            }

            fscanf(docker_ptr, "%s", (*containerInfo)[i].vethName[j]);
            
            pclose(docker_ptr);
        }
    }

    for (int i = 0; i < *containerCnt; i++) { // 5. Get ipv4 address of container.
        sprintf(commandBuf, GET_DOCKER_VETH_IP, (*containerInfo)[i].pid);
        if ((docker_ptr = popen(commandBuf, "r")) == NULL) {
            sprintf(lineBuf, "nsenter -t - pid(%d; %s)", (*containerInfo)[i].pid, (*containerInfo)[i].containerName);
            exception(-2, "get_Docker_Container_Information", lineBuf);
            return;
        }

        while (fgets(lineBuf, sizeof(lineBuf), docker_ptr) != NULL) {
            if (isdigit(lineBuf[0]) == 0) { 
                continue;
            }

            sscanf(lineBuf, "%d:", &tmpIndex);

            for (int j = 0; j < (*containerInfo)[i].ifaCount; j++) {
                if (tmpIndex == (*containerInfo)[i].ifa_index[j]) {
                    fgets(lineBuf, sizeof(lineBuf), docker_ptr);
                    fgets(lineBuf, sizeof(lineBuf), docker_ptr); // lineBuf:     inet xxx.xxx.xxx.xxx/xx ~
                    sscanf(lineBuf, NSENTER_FORM, (*containerInfo)[i].ipv4_addr[j]); // Write ipv4 address with xxx.xxx.xxx.xxx form.
                    break;
                }
            }
        }

        pclose(docker_ptr);
    }
}

void convert_Veth_to_Container_Info(DockerInfo* containerInfo, IFASpeed* ifaBuf, short containerCnt){ // Change veth name to container name. - (*ifaBuf).ifa_name: (Before) vethxxxxxx -> (After) example_Container )
    // kinds of Change information: veth Name (to container name) and ipv4 address ("N/A" to container's ipv4 name)
    // containerInfo: had to be allocated and written the information previously. (At this function, containerInfo is array with data, and only used to read data.)
    // So, Caller invokes this function with array that have container information.
    // ifaBuf is pointer of each element of ifa List. So, Caller must invokes this function with ifaBuf that be in the form of &((*ifaBuf)[i]))

    for (int i = 0; i < containerCnt; i++) {
        for (int j = 0; j < containerInfo[i].ifaCount; j++) {
            if (containerInfo[i].checked[j] == 1) { // Skip already checked veth
                continue;
            }

            if (strcmp(containerInfo[i].vethName[j], ifaBuf->ifa_name) == 0) { // Find veth that same in ifaBuf.
                strncpy(ifaBuf->ifa_name, containerInfo[i].containerName, MAX_DOCKER_CONTAINER_NAME_LEN); // Change veth name to container name.
                strncpy(ifaBuf->ipv4_addr, containerInfo[i].ipv4_addr[j], IPV4_LEN); // Save ipv4 address of veth allocated in container.
                containerInfo[i].checked[j] = 1;
                return;
            }
        } 
    }
}

void convert_BridgeID_to_Name(char* bridgeId) { // Change Bridge ID to Bridge Name.
    FILE* docker_ptr = NULL;
    char commandBuf[COMMAND_MAX_LINE] = { '\0' }, nameBuf[MAX_DOCKER_CONTAINER_NAME_LEN] = { '\0' };
    char* targetPtr = bridgeId + strlen(CHECK_BRIDGE_FORM);

    sprintf(commandBuf, GET_DOCKER_BRIDGE_NAME, targetPtr);
    if ((docker_ptr = popen(commandBuf, "r")) == NULL) { // Get docker bridge name
        exception(-2, "convert_BridgeID_to_Name", "docker network ls");
        return;
    }
    fscanf(docker_ptr, DOCKER_BRIDGE_FORM, nameBuf); // Convert bridge ID to name
    strcpy(commandBuf, BRIDGE_PREFIX);
    strcat(commandBuf, nameBuf);
    strcpy(bridgeId, commandBuf);
}

void sort_Docker_Network(IFASpeed* ifaBuf, short ifaCount) { // Sorting ifa_name of docker network section.
    /*
    <IFA List>
    1. System IFA
    2. Bonding / Teaming
    3. docker network <- Here is sorting target.
    */
    int docker_start_idx = 0, docker_zero_idx = 0, same_IFA_Cnt = 1;
    int bridge_start_idx = 0;
    IFASpeed tmpBuf;

    for (int i = 0; i < ifaCount; i++) { // Place "docker0" interface in Docker network list at the top
        if ((docker_start_idx == 0) && ((STRN_CMP_EQUAL(ifaBuf[i].ifa_name, DOCKER_ZERO_INTERFACE)) || (STRN_CMP_EQUAL(ifaBuf[i].ifa_name, BRIDGE_PREFIX)) || (STRN_CMP_EQUAL(ifaBuf[i].ifa_name, DOCKER_VETH_INTERFACE_PREFIX)))) { // If docker network start
            docker_start_idx = i; // index of docker network section
        }

        if ((docker_zero_idx == 0) && (STRN_CMP_EQUAL(ifaBuf[i].ifa_name, DOCKER_ZERO_INTERFACE))) {
            docker_zero_idx = i; // index of "docker0" interface
        }

        if (docker_start_idx != 0 && docker_zero_idx != 0) {
            tmpBuf = ifaBuf[docker_start_idx];
            ifaBuf[docker_start_idx] = ifaBuf[docker_zero_idx];
            ifaBuf[docker_zero_idx] = tmpBuf;
            docker_zero_idx = docker_start_idx;
        }

    }
    docker_start_idx++;


    for (int i = docker_start_idx; i < ifaCount - 1; i++) { // Sorting docker network section
        for (int j = i + 1; j < ifaCount; j++) {
            if (strcmp(ifaBuf[i].ifa_name, ifaBuf[j].ifa_name) > 0) {
                tmpBuf = ifaBuf[i];
                ifaBuf[i] = ifaBuf[j];
                ifaBuf[j] = tmpBuf;
            }
        }
    }

    same_IFA_Cnt = 0; // <- Here, this variable is the number of bridge interface
    for (int i = docker_start_idx; i < ifaCount; i++) { // Get the number of Bridge interface
        if ((bridge_start_idx == 0) && (STRN_CMP_EQUAL(ifaBuf[i].ifa_name, BRIDGE_PREFIX))) { // index of first Bridge interface
            bridge_start_idx = i;
        }
        if (STRN_CMP_EQUAL(ifaBuf[i].ifa_name, BRIDGE_PREFIX)) {
            same_IFA_Cnt++;
        }
    }

    printf("%d %d\n", docker_start_idx, same_IFA_Cnt);

    for (int i = bridge_start_idx; i < same_IFA_Cnt + bridge_start_idx - 1; i++) {
        for (int j = i + 1; j < same_IFA_Cnt + bridge_start_idx; j++) {
            printf("%d %s %d %s\n", i, ifaBuf[i].ipv4_addr, j, ifaBuf[j].ipv4_addr);
            if (compare_IPv4(ifaBuf[i].ipv4_addr, ifaBuf[j].ipv4_addr) > 0) {
                tmpBuf = ifaBuf[i];
                ifaBuf[i] = ifaBuf[j];
                ifaBuf[j] = tmpBuf;
            }
            printf("%d %s %d %s\n\n", i, ifaBuf[i].ipv4_addr, j, ifaBuf[j].ipv4_addr);
        }
    }

    docker_start_idx += same_IFA_Cnt;

    for (int i = docker_start_idx ; i < ifaCount; i++) { // Sorting on the basis of IPv4 Address in same interface
        same_IFA_Cnt = 1; // <- Here, this variable is the number of interface(= container) that the name is same but ipv4 address is not same.
        for (int j = i + 1; strcmp(ifaBuf[i].ifa_name, ifaBuf[j].ifa_name) == 0; j++) { // Get the number of same interface
            same_IFA_Cnt++;
        }
        
        for (int j = i; j < i + same_IFA_Cnt - 1; j++) {
            for (int k = j + 1; k < i + same_IFA_Cnt; k++) {
                if (compare_IPv4(ifaBuf[j].ipv4_addr, ifaBuf[k].ipv4_addr) > 0) {
                    tmpBuf = ifaBuf[j];
                    ifaBuf[j] = ifaBuf[k];
                    ifaBuf[k] = tmpBuf;
                }
            }
        }
        i += (same_IFA_Cnt - 1);
    }
}

short compare_IPv4 (const char* str1, const char* str2) { // Compare the order of two ipv4 address. 
    char str1Buf[IPV4_LEN], str2Buf[IPV4_LEN];
    short ipVal[4];

    sscanf(str1, "%hd.%hd.%hd.%hd", &ipVal[0], &ipVal[1], &ipVal[2], &ipVal[3]); // xxx.xxx.xxx.xxx form -> extract each "xxx" value.
    sprintf(str1Buf, "%03hd.%03hd.%03hd.%03hd", ipVal[0], ipVal[1], ipVal[2], ipVal[3]);
    sscanf(str2, "%hd.%hd.%hd.%hd", &ipVal[0], &ipVal[1], &ipVal[2], &ipVal[3]); // xxx.xxx.xxx.xxx form -> extract each "xxx" value.
    sprintf(str2Buf, "%03hd.%03hd.%03hd.%03hd", ipVal[0], ipVal[1], ipVal[2], ipVal[3]);

    return strcmp(str1Buf, str2Buf);
}

/* Functions that get partition information (from /proc/diskstats, /proc/mounts) */
void get_Partition_IO_Speed(short partitionCnt, int idx, const char* fileSystem, float* readSpeed, float* writeSpeed){ // Get speed of Partition's Read/Write Speed
    FILE *fp = NULL;
    size_t read = 0, write = 0, sector_size = 0;
    static size_t *priv_read = NULL, *priv_write = NULL;
    char target[BUF_MAX_LINE], compTarget[BUF_MAX_LINE]; // for dm-x

    if (priv_read == NULL){
        priv_read = (size_t*)calloc(partitionCnt, sizeof(size_t));
    }

    if (priv_write == NULL){
        priv_write = (size_t*)calloc(partitionCnt, sizeof(size_t));
    }

    *readSpeed = -100; // Initialization; -100 means "N/A"
    *writeSpeed = -100;

    if (strcmp(fileSystem, PATH_TMPFS) != 0) { // Filter "tmpfs"
        get_Original_Path_for_LVM_Partitions(fileSystem, target); // Get dm-x (Real Path)

        if ((sector_size = get_Sector_Size(target)) == 0) { // Get sector size
            return;
        }

        if ((fp = fopen(DISKSTATS_LOCATION, "r")) == NULL) {
            exception(-1, "get_Partition_IO_Speed", DISKSTATS_LOCATION);
            return;
        }

        while (fscanf(fp, DISKSTATS_FORM, compTarget, &read, &write) != EOF) {
            if (strcmp(compTarget, target) == 0) { // find target partition
                (*readSpeed) = ((read - priv_read[idx]) * sector_size) / (float)1024; // Convert speed unit to "KB/s"
                (*writeSpeed) = ((write - priv_write[idx]) * sector_size) / (float)1024; // Convert speed unit to "KB/s"
                break;
            }
        }
        priv_read[idx] = read;
        priv_write[idx] = write;

        fclose(fp);
    }
}

size_t get_Sector_Size(const char* device){ // Get sector size of partition
    size_t sector_size = 0;
    FILE* fp = NULL;
    char path[MAX_SECTOR_PATH_LEN] = { '\0' }, tmpPath[BUF_MAX_LINE];

    strcpy(tmpPath, device);

    if (strstr(tmpPath, DM_PATH) == NULL) {
        for (int i = 0; i < (int)strlen(device); (tmpPath[i++] = '\0'));
        strncpy(tmpPath, device, 3);
    }

    sprintf(path, GET_SECTOR_SIZE, tmpPath); // Get path (/sys/block/<Device>/queue/hw_sector_size)


    if ((fp = fopen(path, "r")) == NULL) {
        exception(-1, "get_Sector_Size", path);
        return 0;
    }

    fscanf(fp, "%ld", &sector_size);

    return sector_size;
}

void get_Original_Path_for_LVM_Partitions(const char* originPath, char* destination){ // Get Real path for Symbolic Link. (originPath: /dev/mapper/rl-xxxx, destination: dm-x)
    char real_path[MAX_MOUNTPATH_LEN];
    char* dest = NULL;

    if (realpath(originPath, real_path) == NULL) { // Get Real path for originPath(/dev/mapper/rl-xxx -> Symbolic Link to /dev/dm-x)
        exception(-4, "get_Original_Path_for_LVM_Partitions", "realpath()");
        return;
    }
    
    dest = strrchr(real_path, '/');

    if (dest != NULL){
        strcpy(destination, dest + 1);
    } else {
        strcpy(destination, real_path);
    }
}

void get_MountPath_from_FileSystem(const char* fileSystem, char*** mountPath, short* mountPathCnt){ // Get the mount path using fileSystem path of Disk.
    FILE* lsblk_ptr = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' }, originPath[5] = { '\0' }, mountBuf[MAX_MOUNTPATH_LEN] = { '\0' };
    int mountIdx = 0;

    for (int i = 5; fileSystem[i] != '\0'; i++) {
        originPath[i - 5] = fileSystem[i];
        originPath[i - 4] = '\0';
    }

    if ((lsblk_ptr = popen(GET_MOUNTPATH_LSBLK, "r")) == NULL) {
        exception(-2, "get_MountPath_from_FileSystem", "lsblk - mountPathCnt");
        return;
    }

    *mountPathCnt = 0;

    while (fgets(lineBuf, sizeof(lineBuf), lsblk_ptr) != NULL) { // Get the number of child filesystem. (= partition)
        if (strstr(lineBuf, originPath) != NULL){
            do {
                fgets(lineBuf, sizeof(lineBuf), lsblk_ptr);
                sscanf(lineBuf, "%*s %*s %*s %*s %*s %*s %s", mountBuf);
                (*mountPathCnt) += ((strcmp(mountBuf, "") == 0) ? 0 : 1);
                strcpy(mountBuf, "");
            } while (isalpha(lineBuf[0]) == 0);
            break;
        }
    }

    pclose(lsblk_ptr);

    // *mountPath == NULL (Always) -> Caller initialize mountpath pointer to NULL.
    *mountPath = (char**)calloc((*mountPathCnt), sizeof(char*)); // Allocate 2D-Array for mountPath && Initialization to NULL
    if ((lsblk_ptr = popen(GET_MOUNTPATH_LSBLK, "r")) == NULL) {
        exception(-2, "get_MountPath_from_FileSystem", "lsblk - mountPath");
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), lsblk_ptr) != NULL) {
        if (strstr(lineBuf, originPath) != NULL) {
            while (fgets(lineBuf, sizeof(lineBuf), lsblk_ptr) != NULL) {
                if (isalpha(lineBuf[0]) == 0) {
                    sscanf(lineBuf, "%*s %*s %*s %*s %*s %*s %s", mountBuf);
                    if (strcmp(mountBuf, "") == 0) {
                        continue;
                    }
                    (*mountPath)[mountIdx] = (char*)malloc(strlen(mountBuf) * sizeof(char));
                    strcpy((*mountPath)[mountIdx++], mountBuf);
                    strcpy(mountBuf, "");
                }
                
                if (mountIdx == (*mountPathCnt)) {
                    break;
                }
            }    
        }

    }
    
    return;
}

short get_Partition_Information(PartitionInfo** partition_list_ptr, short* partition_cnt){ // Get the partition list and capacity of each partitions.
    // partition_list_ptr: Must be 'not' allocated to memory. (At this function, partition list is allocated to memory.)
    // So, Caller invokes this function with pointer that initialized with "NULL"
    FILE* fp = NULL;
    int cnt = 0;
    char fileSystem[MAX_FILESYSTEM_LEN] = { '\0' };
    char mountPath[MAX_MOUNTPATH_LEN] = { '\0' };

    if ((fp = fopen(MOUNTS_LOCATION, "r")) == NULL) { // Open /proc/mounts
        exception(-1, "get_Partition_List", MOUNTS_LOCATION);
        return -100;
    }

    while (fscanf(fp, MOUNTS_FORM, fileSystem, mountPath) != EOF) { // For counting the number of partitions.
        if ((strncmp("/dev", fileSystem, 4) != 0) && (strncmp("tmpfs", fileSystem, 5) != 0)) { // For filtering partitions
            continue;
        }

        if (strncmp("/dev/loop", fileSystem, strlen("/dev/loop")) == 0) {
            continue;
        }
        cnt++;
    }

    free_Array((void**)partition_list_ptr); // Free the memory allocated for the existing array.
    *partition_list_ptr = (PartitionInfo*)malloc(cnt * sizeof(PartitionInfo)); // Allocate new memory.
    for (int i = 0; i < cnt; i++){ // Initialization
        STR_INIT((*partition_list_ptr)[i].fileSystem);
        STR_INIT((*partition_list_ptr)[i].mountPath);
        (*partition_list_ptr)[i].spaceTotal = 0;
        (*partition_list_ptr)[i].spaceUse = 0;
    }
    fseek(fp, 0, SEEK_SET); // Move pointer to head of file.

    for (int i = 0; fscanf(fp, MOUNTS_FORM, fileSystem, mountPath) != EOF; i++) { // For extracting fileSystem and mountPath of each partitions.
        if ((strncmp("/dev", fileSystem, 4) != 0) && (strncmp("tmpfs", fileSystem, 5) != 0)) { // For filtering partitions
            i--;
            continue;
        }

        if (strncmp("/dev/loop", fileSystem, strlen("/dev/loop")) == 0) {
            i--;
            continue;
        }
        
        strcpy((*partition_list_ptr)[i].fileSystem, fileSystem);
        strcpy((*partition_list_ptr)[i].mountPath, mountPath);
        get_Partition_Usage(&((*partition_list_ptr)[i]));
    }

    (*partition_cnt) = cnt;

    return 0;

}

void get_Partition_Usage(PartitionInfo* partitionBuf){ // Get the capacity of a partition.
    struct statvfs statBuf;
    char errorBuf[ERROR_MSG_LEN] = { '\0' };
    
    if (statvfs(partitionBuf->mountPath, &statBuf) == -1) { // Get capacity information of partition
        partitionBuf->spaceTotal = 0;
        partitionBuf->spaceUse = 0;
        strcpy(errorBuf, "statvfs - ");
        strcat(errorBuf, partitionBuf->mountPath);
        exception(-4, "get_Partition_Usage", errorBuf);
        return;
    }

    partitionBuf->spaceTotal = (size_t)(statBuf.f_blocks * statBuf.f_frsize) / 1024; // Unit: KB
    partitionBuf->spaceUse = (size_t)(statBuf.f_bfree * statBuf.f_frsize) / 1024; // Unit: KB, Free space
    partitionBuf->spaceUse = (partitionBuf->spaceTotal) - (partitionBuf->spaceUse); // Calculate Used space
}