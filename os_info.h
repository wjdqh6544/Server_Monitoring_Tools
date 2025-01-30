#ifndef OS_INFO_H

#define OS_INFO_H

/* Functions that get network interface information (from /proc/net/dev) */
short get_IFA_Speed(DockerInfo** containerInfo, IFASpeed** ifa, short* ifaCount, short* containerCnt);
void get_IPv4_Addr(const char* ifa_name, char* ipv4_addr);
void get_Docker_Container_Information(DockerInfo** containerInfo, short* containerCnt);
void convert_Veth_to_Container_Info(DockerInfo* containerInfo, IFASpeed* ifaBuf, short containerCnt);
void convert_BridgeID_to_Name(char* bridgeId);
void sort_Docker_Network(IFASpeed* ifaBuf, short ifaCount);
short compare_IPv4 (const char* str1, const char* str2);

/* Functions that get partition information (from /proc/diskstats, /proc/mounts) */
void get_Partition_IO_Speed(short partitionCnt, int idx, const char* fileSystem, float* readSpeed, float* writeSpeed);
void get_Original_Path_for_LVM_Partitions(const char* originPath, char* destination);
size_t get_Sector_Size(const char* device);
void get_MountPath_from_FileSystem(const char* fileSystem, char*** mountPath, short* mountPathCnt);
short get_Partition_Information(PartitionInfo** partition_list_ptr, short* partition_cnt);
void get_Partition_Usage(PartitionInfo* partitionBuf);

#endif
