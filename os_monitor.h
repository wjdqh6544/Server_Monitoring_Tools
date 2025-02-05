#ifndef OS_MONITOR_H

#define OS_MONITOR_H
#include "zz_struct.h"

/* List of functions relative to 6th feature. (Display Linux User Account Status) */
void six_User_Account_Status();
void get_Group_Name(gid_t gid, char* gruopName);
int get_Maximum_Length_in_UserInfo(UserInfo* userList, int cnt, int type);

/* List of functions relative to 7th feature. (Display Login History) */
void seven_Login_History();
int get_Maximun_Length_in_LoginInfo(LoginInfo* historyBuf, int histCnt, int type);

/* List of functions relative to 8th feature. (Display Core File Status - Permissions, Ownership, Group) */
void eight_Core_File_Status();
int get_Maximum_Length_in_FileInfo(FileInfo* fileList, int fileCnt, int type);

/* Function relative to 9th feature (Display the information of this program - Version, Copyright, License, and so on.)*/
void nine_About_This_Program();

#endif