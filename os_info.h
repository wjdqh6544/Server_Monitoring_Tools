#ifndef OS_INFO_H

#define OS_INFO_H

/* List of functions of using to check System Running Process */
int get_Process_Status(ProcessInfo** psBuf, int lineCnt);

/* List of functions that check history that try to login this server */
int get_Login_History(LoginInfo** historyBuf, int* historyCnt);
int get_LineCnt_TMP_History();
int save_History(LoginInfo* historyBuf, int* idx, char* commBuf);
int compare_Date(const void* element1, const void* element2);

/* List of functions that check file permissions */
int get_File_Information(FileInfo** fileList, int* fileCnt);
int copy_FilePath(char* targetFilePath, char* destination);
void change_File_Stat(FileInfo* fileBuf);
void write_Stat_Change_Log(FileInfo* fileBuf, FILE** fp);

/* List of functions that get Linux user information*/
int get_UserList(UserInfo** userlist, int* userCnt);
void get_UID_Interval(uid_t* UID_MIN, uid_t* UID_MAX);
void get_PW_Changed_Date(UserInfo* userBuf);
void get_Last_Login_Time_and_IP(UserInfo* userBuf);
int get_Date_Interval(const DateInfo* targetDate);

/* List of functions that get network interface information (from /proc/net/dev) */
int get_IFA_Speed(DockerInfo** containerInfo, IFASpeed** ifa, short* ifaCount, int* containerCnt);
void get_IPv4_Addr(const char* ifa_name, char* ipv4_addr);
void get_Docker_Container_Information(DockerInfo** containerInfo, int* containerCnt);
void convert_Veth_to_Container_Info(DockerInfo* containerInfo, IFASpeed* ifaBuf, int containerCnt);
void convert_BridgeID_to_Name(char* bridgeId);
void sort_Docker_Network(IFASpeed* ifaBuf, int ifaCount);
int compare_IPv4 (const char* str1, const char* str2);

/* List of functions that get partition information (from /proc/diskstats, /proc/mounts) */
void get_Partition_IO_Speed(int partitionCnt, int idx, const char* fileSystem, float* readSpeed, float* writeSpeed);
void get_Original_Path_for_LVM_Partitions(const char* originPath, char* destination);
size_t get_Sector_Size(const char* device);
void get_MountPath_from_FileSystem(const char* fileSystem, char*** mountPath, int* mountPathCnt);
int get_Partition_Information(PartitionInfo** partition_list_ptr, int* partition_cnt);
void get_Partition_Usage(PartitionInfo* partitionBuf);

#endif
