#include "common.h"
#include "0_fileList.h"
#include "os_info.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <grp.h>
#include <ifaddrs.h>
#include <libgen.h>
#include <limits.h>
#include <netinet/in.h>
#include <pwd.h>
#include <shadow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <time.h>
#include <utmp.h>
#include <unistd.h>

extern const DateInfo dateBuf;

/* List of functions of using to check System Running Process */
int get_Process_Status(ProcessInfo** psBuf, int lineCnt) { // Get process status -> Sorted by RAM in decreasing order.
    // lineCnt -> the number of displaying the data. It is remaining space of terminal window size.
    FILE* ps_ptr = NULL;
    char errorBuf[BUF_MAX_LINE] = { '\0' }, pathBuf[MAX_MOUNTPATH_LEN] = { '\0' };

    if (*psBuf == NULL || lineCnt != (sizeof(*psBuf) / sizeof(ProcessInfo*))) { // If windows size is changed.
        for (int i = 0; ((*psBuf != NULL) && ((size_t)i < (sizeof(*psBuf) / sizeof(ProcessInfo*)))); i++) {
            free_Array((void**)&((*psBuf)[i].command));
            free_Array((void**)&((*psBuf)[i].userName));
        }
        free_Array((void**)psBuf);

        if ((*psBuf = (ProcessInfo*)malloc(lineCnt * sizeof(ProcessInfo))) == NULL) {
            exception(-4, "get_Process_Status", "malloc() - psBuf");
            return -100;
        }
    }

    for (int i = 0; i < lineCnt; i++) { // Intialization
        STR_INIT((*psBuf)[i].userName);
        (*psBuf)[i].pid = 0;
        (*psBuf)[i].cpu = -100;
        (*psBuf)[i].mem = -100;
        (*psBuf)[i].memUseSize = 0;
        STR_INIT((*psBuf)[i].tty);
        STR_INIT((*psBuf)[i].start);
        STR_INIT((*psBuf)[i].time);
        (*psBuf)[i].command = NULL;
    }

    if ((ps_ptr = popen(GET_PS_COMMAND, "r")) == NULL) { // Get information from ps command
        strcpy(errorBuf, "popen() - ");
        strcat(errorBuf, GET_PS_COMMAND);
        exception(-2, "get_Process_Status", errorBuf);
    }

    fgets(errorBuf, sizeof(errorBuf), ps_ptr); // Skip Header | USER         PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND       
    for (int i = 0; i < lineCnt; i++) {
        fscanf(ps_ptr, GET_PS_INFO_FORM, (*psBuf)[i].userName, &((*psBuf)[i].pid), &((*psBuf)[i].cpu), &((*psBuf)[i].mem), &((*psBuf)[i].memUseSize), (*psBuf)[i].tty, (*psBuf)[i].start, (*psBuf)[i].time, pathBuf);
        if (((*psBuf)[i].command = (char*)malloc(strlen(pathBuf) * sizeof(char))) == NULL) { // Allocate array for string (Command)
            sprintf(errorBuf, "malloc() - psBuf[%d].command", i);
            exception(-4, "get_Process_Status", errorBuf);
            continue;
        }
        strcpy((*psBuf)[i].command, pathBuf);
        if (STRN_CMP_EQUAL((*psBuf)[i].tty, TTY_BACKGROUND)) {
            strcpy((*psBuf)[i].tty, TTY_BACKGROUND_STR);
        }
    }
    pclose(ps_ptr);
    return 0;
}

/* List of functions of using to check history that try to login this server */
int get_Login_History(LoginInfo** historyBuf, int* historyCnt) { // Get the history of login (Success, Failed)
    int idx = 0, btmpErr, wtmpErr;
    char commBuf[BUF_MAX_LINE];
    (void)historyBuf;
    *historyCnt = get_LineCnt_TMP_History();

    if ((*historyCnt) == -100) {
        return -100;
    }
    
    if (*historyBuf == NULL) { // Allocate Array for Login history
        if ((*historyBuf = (LoginInfo*)malloc((*historyCnt) * sizeof(struct utmp))) == NULL) {
            exception(-4, "get_Login_History", "malloc() - historyBuf");
            return -100;
        }
    }

    for (int i = 0; i < *historyCnt; i++) { // Initialization
        STR_INIT((*historyBuf)[i].userName);
        (*historyBuf)[i].uid = 0;
        (*historyBuf)[i].status = -100;
        (*historyBuf)[i].logDate = (DateInfo){ 0, 0, 0, 0, 0, 0 };
        STR_INIT((*historyBuf)[i].loginIP);
        STR_INIT((*historyBuf)[i].deviceName);
    }
    

    sprintf(commBuf, GET_TMPLOG_LIST, BTMP_WTMP_LOCATION, BTMP_NAME, BTMP_NAME); // btmp path
    btmpErr = save_History(*historyBuf, &idx, commBuf); // Save btmp info to array.

    sprintf(commBuf, GET_TMPLOG_LIST, BTMP_WTMP_LOCATION, WTMP_NAME, WTMP_NAME); // wtmp path
    wtmpErr = save_History(*historyBuf, &idx, commBuf); // Save btmp info to array.

    qsort(*historyBuf, *historyCnt, sizeof(LoginInfo), compare_Date);

    if (btmpErr == -100 && wtmpErr == -100) {
        return -100;
    } else if (btmpErr == -100 && wtmpErr != -100) { // btmpErr data load ERROR
        return -99;
    } else if (btmpErr != -100 && wtmpErr == -100) { // wtmpErr data load ERROR
        return -98;
    } else {
        return 0;
    }
}

int get_LineCnt_TMP_History() { // Get the number of line in btmp*, wtmp* log file.
    char pathBuf[BUF_MAX_LINE], errorBuf[BUF_MAX_LINE], temp[BTMP_WTMP_FILENAME_LEN];
    FILE *ls_ptr = NULL;
    struct utmp utBuf;
    int fd = -1, cnt = 0, buf = 0;

    sprintf(pathBuf, GET_TMPLOG_LIST, BTMP_WTMP_LOCATION, BTMP_NAME, WTMP_NAME); // Get fileList of btmp*
    if ((ls_ptr = popen(pathBuf, "r")) == NULL) {
        strcpy(errorBuf, "popen() - ");
        strcat(errorBuf, pathBuf);
        exception(-2, "get_LineCnt_TMP_History", errorBuf);
        return -100;
    }

    while (fscanf(ls_ptr, "%s", temp) != EOF) { // Get the number of row in btmp* / wtmp*
        sprintf(pathBuf, TMP_LOCATION_FORM, BTMP_WTMP_LOCATION, temp);
        if ((fd = open(pathBuf, O_RDONLY)) == -1) {
            exception(-1, "get_LineCnt_TMP_History", pathBuf);
            return -100;
        }
        buf = 0;
        while(read(fd, &utBuf, sizeof(utBuf)) == sizeof(utBuf)) { // Count line of USER_PROCESS in a file
            if ((utBuf.ut_type != USER_PROCESS) && (utBuf.ut_type != LOGIN_PROCESS)) { // Filtering
                continue;
            }
            buf++;
        }
        close(fd);
        cnt += buf;
    }
    pclose(ls_ptr);

    return cnt;
}

int save_History(LoginInfo* historyBuf, int* idx, char* commBuf) { // Save history to Struct array.
    // historyBuf: Array of login history / idx: index of array / commBuf: "ls -al" command for getting list of btmp / wtmp file 
    // Not defined Macro in 0_usrDefine.h is defined at <utmp.h>. To see macro content, See "/usr/include/bits/utmp.h".
    FILE* ls_ptr = NULL;
    struct utmp utBuf;
    struct passwd *pw = NULL;
    struct tm *tm = NULL;
    time_t loginTime;
    int fd = -1;
    char errorBuf[BUF_MAX_LINE] = { '\0' }, pathBuf[BUF_MAX_LINE] = { '\0' }, filenameBuf[UT_NAMESIZE + 1] = { '\0' };
    
    if ((ls_ptr = popen(commBuf, "r")) == NULL) { // Get List of btmp
        sprintf(errorBuf, "popen() - (for Content) %s", commBuf);
        exception(-2, "save_History", errorBuf);
        return -100;
    }

    while (fscanf(ls_ptr, "%s", filenameBuf) != EOF) { // ls_ptr -> list of btmp filename 
        sprintf(pathBuf, TMP_LOCATION_FORM, BTMP_WTMP_LOCATION, filenameBuf);
        if ((fd = open(pathBuf, O_RDONLY)) == -1 ) {
            exception(-1, "save_History", pathBuf);
            return -100;
        }

        while (read(fd, &utBuf, sizeof(struct utmp)) == sizeof(struct utmp)) {
            if ((utBuf.ut_type != USER_PROCESS) && (utBuf.ut_type != LOGIN_PROCESS)) { // Extrace history of Login success or fail
                continue;
            }

            strncpy(historyBuf[*idx].userName, utBuf.ut_user, UT_NAMESIZE); // Copy username, (Maximum length of utBUf.ut_user is UT_NAMESIZE)

            if ((pw = getpwnam(utBuf.ut_user)) != NULL) { // If user exist
                historyBuf[*idx].uid = pw->pw_uid; // Copy UID
            } else { // If user does not exist
                if (strlen(historyBuf[*idx].userName) > UT_NAMESIZE - strlen(USER_NOEXIST)) { // If the length of username is more than displayable length.
                    strncpy(filenameBuf, historyBuf[*idx].userName, strlen(historyBuf[*idx].userName) - strlen(USER_NOEXIST) - strlen("..."));
                    strcat(filenameBuf, "...");
                    strcat(filenameBuf, USER_NOEXIST);
                    strcpy(historyBuf[*idx].userName, filenameBuf);
                } else { // Add "(NoExist)" String at the tail of username.
                    strcat(historyBuf[*idx].userName, USER_NOEXIST);
                }
            }

            if (strstr(pathBuf, BTMP_NAME) == NULL) {
                historyBuf[*idx].status = LOGIN_SUCCESS; // Set status (Login Success: wtmp)
            } else {
                historyBuf[*idx].status = LOGIN_FAILED; // Set status (Login Failed: btmp)
            }

            strncpy(historyBuf[*idx].deviceName, utBuf.ut_line, UT_LINESIZE); // Copy connection method

            if (STRN_CMP_EQUAL(historyBuf[*idx].deviceName, "tty")) { // If login through Console (Local)
                strncpy(historyBuf[*idx].loginIP, IP_LOCAL_STR, UT_LINESIZE);
            } else if ((STRN_CMP_EQUAL(historyBuf[*idx].deviceName, "pts")) || (STRN_CMP_EQUAL(historyBuf[*idx].deviceName, "ssh"))) { // If login through SSH (Remote)
                strncpy(historyBuf[*idx].loginIP, utBuf.ut_host, UT_LINESIZE);
            } else { // If login through other method
                strncpy(historyBuf[*idx].loginIP, IP_OTHER_STR, UT_LINESIZE);
            }

            loginTime = utBuf.ut_time; // Login Time
            tm = localtime(&loginTime);

            if ((tm->tm_year + 1900) < 1970) { // Invalid date
                (*idx)++;
                continue;
            }

            historyBuf[*idx].logDate.year = (short)((tm->tm_year) + 1900);
            historyBuf[*idx].logDate.month = (short)((tm->tm_mon) + 1);
            historyBuf[*idx].logDate.day = (short)(tm->tm_mday);
            historyBuf[*idx].logDate.hrs = (short)(tm->tm_hour);
            historyBuf[*idx].logDate.min = (short)(tm->tm_min);
            historyBuf[*idx].logDate.sec = (short)(tm->tm_sec);

            (*idx)++;
        }
        close(fd);
    }
    pclose(ls_ptr);
    return 0;
}

int compare_Date(const void* element1, const void* element2) { // Compare two date. (Criteria: Ascending Order)
    // If this function returns a positive number, qsort function is change two element.
    int val;
    const LoginInfo* tmpBuf1 = (LoginInfo*)element1;
    const LoginInfo* tmpBuf2 = (LoginInfo*)element2;
    const DateInfo* date1 = &(tmpBuf1->logDate);
    const DateInfo* date2 = &(tmpBuf2->logDate);
    
    if ((val = (date1->year - date2->year)) == 0) { // Compare year. Difference of two date is Same or date1 is ahead of date2
        if ((val = (date1->month - date2->month)) == 0) { // Compare month.
            if ((val = (date1->day - date2->day)) == 0) { // Compare day.
                if ((val = (date1->hrs - date2->hrs)) == 0) { // Compare hours.
                    if ((val = (date1->min - date2->min)) == 0) { // Compare minutes.
                        if ((val = (date1->sec - date2->sec)) == 0) { // Compare seconds.
                            return val;
                        }
                    }
                }       
            }
        }
    }
    return val;
}

// int remove_History() { // Remove displayed history

// }

/* List of functions of using to check file permissions */
int get_File_Information(FileInfo** fileInfo, int* fileCnt) { // Check file permissions
    FILE* history_fp = NULL;
    char errorBuf[ERROR_MSG_LEN * 2], pathBuf[BUF_MAX_LINE], userBuf[USERNAME_LEN], groupBuf[GRPNAME_LEN], logPath[MAX_MOUNTPATH_LEN];
    struct stat statBuf;
    struct passwd *pw = NULL;
    struct group *grp = NULL;
    int notFount = 0;
    *fileCnt = sizeof(targetFile) / sizeof(targetFile[0]);

    for (int i = 0; i < *fileCnt; i++) {
        sscanf(targetFile[i], "%s %*s %*s %*o", pathBuf);
        if (access(pathBuf, F_OK) != 0) {
            sprintf(errorBuf, "File Not found. - %s", pathBuf);
            exception(-1, "get_File_Information", errorBuf);
            notFount++;
            continue;
        }
    }

    *fileCnt -= notFount; // Except the path that not exist.

    if (*fileInfo == NULL) { // Allocate Array
        if ((*fileInfo = (FileInfo*)malloc((*fileCnt) * sizeof(FileInfo))) == NULL) {
            exception(-4, "get_File_Information", "malloc() - fileInfo");
            return -100;
        }
    }

    for (int i = 0; i < *fileCnt; i++) { // Initialization
        STR_INIT((*fileInfo)[i].path);
        STR_INIT((*fileInfo)[i].fullPath);
        (*fileInfo)[i].changed[2] = 0;
        for (int j = 0; j < 2; j++) {
            (*fileInfo)[i].changed[j] = 0;
            (*fileInfo)[i].ownerUID[j] = 0;
            (*fileInfo)[i].groupGID[j] = 0;
        }
    }

    get_Filename(logPath, LOG_PATH, STAT_HISTORY_LOG, &dateBuf); // Initialization for History Log
    if ((history_fp = fopen(logPath, "a")) == NULL) {
        exception(-1, "get_File_Information", logPath);
        return -100;
    }
    fprintf(history_fp, "-------------------------------------------------------------------------\n");

    for (int i = 0; i < *fileCnt; i++) {
        sscanf(targetFile[i], "%s %s %s %o", (*fileInfo)[i].fullPath, userBuf, groupBuf, &((*fileInfo)[i].permission[1]));
        
        if (access((*fileInfo)[i].fullPath, F_OK) != 0) {
            continue;
        }

        if (copy_FilePath((*fileInfo)[i].fullPath, (*fileInfo)[i].path) == -100) {
            return -100;
        }

        if (stat((*fileInfo)[i].fullPath, &statBuf) == -1) { // Get file stat information
            sprintf(errorBuf, "stat() - %s", strrchr((*fileInfo)[i].fullPath, '/'));
            exception(-4, "get_File_Information", errorBuf);
            return -100;
        }

        (*fileInfo)[i].ownerUID[0] = statBuf.st_uid;
        (*fileInfo)[i].groupGID[0] = statBuf.st_gid;
        (*fileInfo)[i].permission[0] = statBuf.st_mode & 0777;

        if ((pw = getpwnam(userBuf)) == NULL) { // Convert user name to uid
            sprintf(errorBuf, "getpwnam() - %s", userBuf);
            exception(-4, "get_File_Information", errorBuf);
            continue;
        }

        (*fileInfo)[i].ownerUID[1] = pw->pw_uid;

        if ((grp = getgrnam(groupBuf)) == NULL) { // Convert gruop name to gid
            sprintf(errorBuf, "getgrnam() - %s", groupBuf);
            exception(-4, "get_File_Information", errorBuf);
            continue;
        }

        (*fileInfo)[i].groupGID[1] = grp->gr_gid;

        (*fileInfo)[i].changed[0] =  (((*fileInfo)[i].ownerUID[0] == (*fileInfo)[i].ownerUID[1]) ? NOT_NEED_CHANGING : NEED_CHANGING); // Check whether need to be changed uid/gid/permissions.
        (*fileInfo)[i].changed[1] =  (((*fileInfo)[i].groupGID[0] == (*fileInfo)[i].groupGID[1]) ? NOT_NEED_CHANGING : NEED_CHANGING); // Check whether need to be changed uid/gid/permissions.
        (*fileInfo)[i].changed[2] =  (((*fileInfo)[i].permission[0] == (*fileInfo)[i].permission[1]) ? NOT_NEED_CHANGING : NEED_CHANGING); // Check whether need to be changed uid/gid/permissions.

        change_File_Stat(&(*fileInfo)[i]);
        write_Stat_Change_Log(&(*fileInfo)[i], &history_fp);
    }

    fclose(history_fp);
    return 0;
}

int copy_FilePath(char* targetFilePath, char* destination) { // Copy file path to FileInfo element.
    char* fileName = NULL;
    char pathBuf[BUF_MAX_LINE], firstD[MAX_PATH_LEN - 7];
    if (strlen(targetFilePath) > MAX_PATH_LEN) { // If path length exceeds the bufer size.
        if ((fileName = basename(targetFilePath)) == NULL) { // Get filename
            sprintf(pathBuf, "basename() - %s", targetFilePath);
            exception(-4, "copy_FilePath", pathBuf);
            return -100;
        }

        if (strlen(fileName) > MAX_PATH_LEN) {
            strncpy(destination, fileName, MAX_PATH_LEN - 5);
            strcat(destination, " ...");
            return 0;

        } else {
            sscanf(targetFilePath, "/%[^/]", firstD); // Extract first directory
            if ((strlen("/") + strlen(firstD) + strlen("/.../") + strlen(fileName)) > MAX_PATH_LEN) { // ex. length of (/etc/.../fstab) > MAX_PATH_LEN
                strncpy(destination, fileName, MAX_PATH_LEN);
                return 0;
            }
            snprintf(destination, MAX_PATH_LEN, "/%s/.../%s", firstD, fileName);
            return 0;
        }

    } else {
        strncpy(destination, targetFilePath, MAX_PATH_LEN);
        return 0;
    }
}

void change_File_Stat(FileInfo* fileBuf) { // Change file permissions, onwership, group
    char errorBuf[ERROR_MSG_LEN];
    char* targetPtr = NULL;
    
    if (fileBuf->changed[0] == NEED_CHANGING) { // If UID needs to be changed
        if (chown(fileBuf->fullPath, fileBuf->ownerUID[1], -1) == -1) {
            targetPtr = strrchr(fileBuf->fullPath, '/');
            sprintf(errorBuf, "chown() - targetUID: %d | Name: %s", fileBuf->ownerUID[1], targetPtr);
            exception(-4, "change_File_Stat", errorBuf);
            fileBuf->changed[0] = FAILED_CHANGING;
        }
    }

    if (fileBuf->changed[1] == NEED_CHANGING) { // If GID needs to be changed
        if (chown(fileBuf->fullPath, -1, fileBuf->groupGID[1]) == -1) {
            targetPtr = strrchr(fileBuf->fullPath, '/');
            sprintf(errorBuf, "chown() - targetGID: %d | Name: %s", fileBuf->groupGID[1], targetPtr);
            exception(-4, "change_File_Stat", errorBuf);
            fileBuf->changed[1] = FAILED_CHANGING;
        }
    }

    if (fileBuf->changed[2] == NEED_CHANGING) { // If Permissions needs to be changed
        if ((chmod(fileBuf->fullPath, fileBuf->permission[1])) == -1) {
            targetPtr = strrchr(fileBuf->fullPath, '/');
            sprintf(errorBuf, "chown() - targetPerm.: %d | Name: %s", fileBuf->permission[1], targetPtr);
            exception(-4, "change_File_Stat", errorBuf);
            fileBuf->changed[2] = FAILED_CHANGING;
        }
    }
}

void write_Stat_Change_Log(FileInfo* fileBuf, FILE** fp) { // Write stat information Log
    fprintf(*fp, "%02hd:%02hd:%02hd %s | UID: ", dateBuf.hrs, dateBuf.min, dateBuf.sec, fileBuf->fullPath);

    switch (fileBuf->changed[0]) { // UID
        case NOT_NEED_CHANGING:
            fprintf(*fp, "%s / GID: ", NOT_NEED_CHANGING_STR);
            break;
        case NEED_CHANGING:
            fprintf(*fp, "%s (%d -> %d) / GID: ", NEED_CHANGING_STR, fileBuf->ownerUID[0], fileBuf->ownerUID[1]);
            break;
        case FAILED_CHANGING:
            fprintf(*fp, "%s (%d -> %d) / GID: ", FAILED_CHANGING_STR, fileBuf->ownerUID[0], fileBuf->ownerUID[1]);
            break;
    }

    switch (fileBuf->changed[1]) { // GID
        case NOT_NEED_CHANGING:
            fprintf(*fp, "%s / Perm.: ", NOT_NEED_CHANGING_STR);
            break;
        case NEED_CHANGING:
            fprintf(*fp, "%s (%d -> %d) / Perm.: ", NEED_CHANGING_STR, fileBuf->groupGID[0], fileBuf->groupGID[1]);
            break;
        case FAILED_CHANGING:
            fprintf(*fp, "%s (%d -> %d) / Perm.: ", FAILED_CHANGING_STR, fileBuf->groupGID[0], fileBuf->groupGID[1]);
            break;
    }

    switch (fileBuf->changed[2]) { // Permissions
        case NOT_NEED_CHANGING:
            fprintf(*fp, "%s\n", NOT_NEED_CHANGING_STR);
            break;
        case NEED_CHANGING:
            fprintf(*fp, "%s (%03o -> %03o)\n", NEED_CHANGING_STR, fileBuf->permission[0], fileBuf->permission[1]);
            break;
        case FAILED_CHANGING:
            fprintf(*fp, "%s (%03o -> %03o)\n", FAILED_CHANGING_STR, fileBuf->permission[0], fileBuf->permission[1]);
            break;
    }
}

/* List of functions of using to get Linux user information*/
int get_UserList(UserInfo** userlist, int* userCnt) { // Get User List. return the number of users.
    struct passwd *userBuf;
    uid_t UID_MIN, UID_MAX;
    gid_t tmp[NGROUPS_MAX] = { 0 }, tmpVal;
    int idx = 0;
    char lineBuf[BUF_MAX_LINE];

    get_UID_Interval(&UID_MIN, &UID_MAX);

    setpwent(); // Move pointer of /etc/passwd to head.

    while ((userBuf = getpwent()) != NULL) { // Get the number of users
        if ((strcmp(userBuf->pw_name, "root") == 0) || (((userBuf->pw_uid >= UID_MIN) && (userBuf->pw_uid <= UID_MAX)) && (strcmp(userBuf->pw_name, "nobody") != 0))) {
            (*userCnt)++;
        }
    }

    if (*userCnt == 0 && userBuf == NULL){
        exception(-4, "get_UserList", "getpwent() - get usrCnt");
        return -100;
    }
    
    if (*userlist == NULL) { // UserInfo Array Allocate
        if ((*userlist = (UserInfo*)malloc((*userCnt) * sizeof(UserInfo))) == NULL) {
            exception(-4, "get_UserList", "malloc() - userlist");
            return -100;
        }
    }

    for (int i = 0; i < *userCnt; i++) { // Initialization
        (*userlist)[i].userName = NULL;
        (*userlist)[i].uid = 0;
        (*userlist)[i].grpCnt = -100;
        (*userlist)[i].gid = NULL;
        (*userlist)[i].lastLogin = (DateInfo){ 0, 0, 0, 0, 0, 0 };
        STR_INIT((*userlist)[i].loginIP);
        (*userlist)[i].lastChangePW = (DateInfo){ 0, 0, 0, 0, 0, 0 };
    }
        
    endpwent(); // Close /etc/passwd
    setpwent(); // Move pointer of /etc/passwd to head.

    while ((userBuf = getpwent()) != NULL) { // Saving User Information to array element
        if ((strcmp(userBuf->pw_name, "root") == 0) || ((userBuf->pw_uid >= 1000) && (strcmp(userBuf->pw_name, "nobody") != 0))) {
            if (((*userlist)[idx].userName = (char*)malloc((strlen(userBuf->pw_name) + 1) * sizeof(char))) == NULL) { // Allocate array for string of username
                sprintf(lineBuf, "malloc() - userlist[%d].userName", idx);
                exception(-4, "get_UserList", lineBuf);
                return -100;
            }
            strcpy((*userlist)[idx].userName, userBuf->pw_name); // Copy username
            (*userlist)[idx].uid = userBuf->pw_uid; // Copy uid of user
            (*userlist)[idx].grpCnt = NGROUPS_MAX; // Initialization

            if ((getgrouplist(userBuf->pw_name, userBuf->pw_gid, tmp, &((*userlist)[idx].grpCnt))) == -1) { // the number of groups update
                exception(-4, "get_UserList", "getgrouplist() - get grpCnt");
                return -100;
            }

            if (((*userlist)[idx].gid = (gid_t*)malloc(((*userlist)[idx].grpCnt) * sizeof(gid_t))) == NULL) { // Allocate array for Group UID (GID)
                sprintf(lineBuf, "malloc() - userlist[%d].gid", idx);
                exception(-4, "get_UserList", lineBuf);
                return -100;
            }
            if ((getgrouplist(userBuf->pw_name, userBuf->pw_gid, (*userlist)[idx].gid, &((*userlist)[idx].grpCnt))) == -1) {
                exception(-4, "get_UserList", "getgrouplist() - get gid");
                return -100;
            }
            idx++;
        }
    }

    for (int i = 0; i < *userCnt; i++) {
        for (int j = 0; j < (*userlist)[i].grpCnt - 1; j++) { // Sorting ascending order
            for (int k = j + 1; k < (*userlist)[i].grpCnt; k++) {
                if ((*userlist)[i].gid[j] > (*userlist)[i].gid[k]) {
                    tmpVal = (*userlist)[i].gid[j];
                    (*userlist)[i].gid[j] = (*userlist)[i].gid[k];
                    (*userlist)[i].gid[k] = tmpVal;
                }
            }
        }
    }

    if (idx == 0 && userBuf == NULL){
        exception(-4, "get_UserList", "getpwent() - get info of each user");
        return -100;
    }

    endpwent(); // Close /etc/passwd

    for (int i = 0; i < *userCnt; i++) {
        get_PW_Changed_Date(&((*userlist)[i]));
        get_Last_Login_Time_and_IP(&((*userlist)[i]));
    }

    return 0;
}

void get_UID_Interval(uid_t* UID_MIN, uid_t* UID_MAX) { // Get User's UID Interval (Min ~ Max)
    FILE* fp = NULL;
    char lineBuf[BUF_MAX_LINE] = { '\0' };

    *UID_MIN = 0; // Initialization
    *UID_MAX = 0;
    
    if ((fp = fopen(LOGIN_DEFS_LOCATION, "r")) == NULL) { // Open /etc/login.defs
        exception (-1, "get_UID_Interval", LOGIN_DEFS_LOCATION);
        return;
    }

    while (fgets(lineBuf, sizeof(lineBuf), fp) != NULL) {
        if (strstr(lineBuf, UID_MIN_STR) != NULL) {
            sscanf(lineBuf, UID_FORM, UID_MIN);
        }
        
        if (strstr(lineBuf, UID_MAX_STR) != NULL) {
            sscanf(lineBuf, UID_FORM, UID_MAX);
        }

        if (*UID_MIN != 0 && *UID_MAX != 0) {
            fclose(fp);
            return;
        }
    }

    fclose(fp);
}

void get_PW_Changed_Date(UserInfo* userBuf) { // Get last change date of user's password.
    // userBuf -> (Caller) get_PW_Changed_Date(&userBuf[i]) : A pointer of one element 
    struct spwd *shadowBuf = getspnam(userBuf->userName);
    struct tm *tm = NULL;
    char lineBuf[BUF_MAX_LINE];
    time_t last_changed;

    if (shadowBuf == NULL) {
        sprintf(lineBuf, "getspnam - %s", userBuf->userName);
        exception(-4, "get_PW_Changed_Date", lineBuf);
        return;
    }

    last_changed = (time_t)shadowBuf->sp_lstchg * 86400; // Convert days to seconds
    tm = localtime(&last_changed);
    if ((tm->tm_year + 1900) < 1970) { // If password is never changed
        return;
    }

    userBuf->lastChangePW.year = (short)((tm->tm_year) + 1900);
    userBuf->lastChangePW.month = (short)((tm->tm_mon) + 1);
    userBuf->lastChangePW.day = (short)(tm->tm_mday);
    userBuf->lastChangePW.hrs = (short)(tm->tm_hour);
    userBuf->lastChangePW.min = (short)(tm->tm_min);
    userBuf->lastChangePW.sec = (short)(tm->tm_sec);
}

void get_Last_Login_Time_and_IP(UserInfo* userBuf) { // Get last login date and IP
    struct utmp ut;
    struct tm *tm = NULL;
    time_t last_logined;
    int fd = -1;
    char lineBuf[BUF_MAX_LINE];

    if ((fd = open(WTMP_LOCATION, O_RDONLY)) == -1) {
        exception(-1, "get_Last_Login_Time", WTMP_LOCATION);
        return;
    }

    lseek(fd, -sizeof(struct utmp), SEEK_END);

    while (read(fd, &ut, sizeof(struct utmp)) == sizeof(struct utmp)) {
        if (strncmp(ut.ut_user, userBuf->userName, sizeof(ut.ut_user)) == 0 && ut.ut_type == USER_PROCESS) { 
            strncpy(lineBuf, ut.ut_line, sizeof(ut.ut_line));
            if (STRN_CMP_EQUAL(lineBuf, "tty")) { // If login through Console (Local)
                sprintf(lineBuf, "Local(%s)", ut.ut_line);
                strncpy(userBuf->loginIP, lineBuf, sizeof(userBuf->loginIP));
            } else if (STRN_CMP_EQUAL(lineBuf, "pts")) { // If login through SSH (Remote)
                strncpy(userBuf->loginIP, ut.ut_host, sizeof(userBuf->loginIP));
            } else { // If login through other method
                sprintf(lineBuf, "Other(%s)", ut.ut_line);
                strncpy(userBuf->loginIP, lineBuf, sizeof(userBuf->loginIP));
            }

            last_logined = ut.ut_time; // Login Time
            tm = localtime(&last_logined);

            if ((tm->tm_year + 1900) < 1970) { // Invalid date
                return;
            }

            userBuf->lastLogin.year = (short)((tm->tm_year) + 1900);
            userBuf->lastLogin.month = (short)((tm->tm_mon) + 1);
            userBuf->lastLogin.day = (short)(tm->tm_mday);
            userBuf->lastLogin.hrs = (short)(tm->tm_hour);
            userBuf->lastLogin.min = (short)(tm->tm_min);
            userBuf->lastLogin.sec = (short)(tm->tm_sec);

            close(fd);
            return;
        }
        lseek(fd, -sizeof(struct utmp) * 2, SEEK_CUR);
    }
    close(fd);
}

int get_Date_Interval(const DateInfo* targetDate) { // Get date interval (Target Date ~ Now)
    struct tm tm = { 0 };
    time_t targetTime, nowTime;

    tm.tm_year = targetDate->year - 1900; // Convert target date
    tm.tm_mon = targetDate->month - 1;
    tm.tm_mday = targetDate->day;

    targetTime = mktime(&tm);

    tm.tm_year = dateBuf.year - 1900; // Convert now date
    tm.tm_mon = dateBuf.month - 1;
    tm.tm_mday = dateBuf.day;

    nowTime = mktime(&tm); 

    return difftime(nowTime, targetTime) / (60 * 60 * 24); // Get differenc of two date (Unit: days)
}

/* List of functions of using to get network interface information (from /proc/net/dev) */
int get_IFA_Speed(DockerInfo** containerInfo, IFASpeed** ifa, short* ifaCount, int* containerCnt){ // Get receive and transmit speed of network interfaces.
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

void get_Docker_Container_Information(DockerInfo** containerInfo, int* containerCnt){ // Get docker container information: name and ipv4 address corresponding to veth (vethxxxxxx: virtual interface of container)
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
        if ((*containerInfo = (DockerInfo*)malloc((*containerCnt) * sizeof(DockerInfo))) == NULL) {
            exception(-4, "get_Docker_Container_Information", "malloc() - containerInfo");
            return;
        }

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

    pclose(docker_ptr);

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

        if (((*containerInfo)[i].checked = (short*)calloc((*containerInfo)[i].ifaCount, sizeof(short))) == NULL) { // Create array for checking status -> Init to "0" Automatically.
            sprintf(lineBuf, "calloc() - containerInfo[%d].checked", i);
            exception(-4, "get_Docker_Container_Information", lineBuf);
            return;
        }
        if (((*containerInfo)[i].vethName = (char**)malloc(((*containerInfo)[i].ifaCount) * sizeof(char*))) == NULL) { // Create 2D array for veth name.
            sprintf(lineBuf, "malloc() - containerInfo[%d].vethName", i);
            exception(-4, "get_Docker_Container_Information", lineBuf);
            return;
        }
        if (((*containerInfo)[i].ipv4_addr = (char**)malloc(((*containerInfo)[i].ifaCount) * sizeof(char*))) == NULL) { // Create 2D array for IPv4 of veth.
            sprintf(lineBuf, "malloc() - containerInfo[%d].ipv4_addr", i);
            exception(-4, "get_Docker_Container_Information", lineBuf);
            return;
        }
        if (((*containerInfo)[i].ifa_index = (short*)malloc(((*containerInfo)[i].ifaCount) * sizeof(short))) == NULL) { // Create array for veth interface index.
            sprintf(lineBuf, "malloc() - containerInfo[%d].ifa_index", i);
            exception(-4, "get_Docker_Container_Information", lineBuf);
            return;
        }
        for (int j = 0; j < (*containerInfo)[i].ifaCount; j++) { // Initialization
            if (((*containerInfo)[i].vethName[j] = (char*)malloc(MAX_DOCKER_CONTAINER_NAME_LEN * sizeof(char))) == NULL) {
                sprintf(lineBuf, "malloc() - containerInfo[%d].vethName[%d]", i, j);
                exception(-4, "get_Docker_Container_Information", lineBuf);
                return;
            }
            if (((*containerInfo)[i].ipv4_addr[j] = (char*)malloc(IPV4_LEN * sizeof(char))) == NULL) {
                sprintf(lineBuf, "malloc() - containerInfo[%d].ipv4_addr[%d]", i, j);
                exception(-4, "get_Docker_Container_Information", lineBuf);
                return;
            }
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

void convert_Veth_to_Container_Info(DockerInfo* containerInfo, IFASpeed* ifaBuf, int containerCnt){ // Change veth name to container name. - (*ifaBuf).ifa_name: (Before) vethxxxxxx -> (After) example_Container )
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

    pclose(docker_ptr);
}

void sort_Docker_Network(IFASpeed* ifaBuf, int ifaCount) { // Sorting ifa_name of docker network section.
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

    for (int i = bridge_start_idx; i < same_IFA_Cnt + bridge_start_idx - 1; i++) {
        for (int j = i + 1; j < same_IFA_Cnt + bridge_start_idx; j++) {
            if (compare_IPv4(ifaBuf[i].ipv4_addr, ifaBuf[j].ipv4_addr) > 0) {
                tmpBuf = ifaBuf[i];
                ifaBuf[i] = ifaBuf[j];
                ifaBuf[j] = tmpBuf;
            }
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

int compare_IPv4 (const char* str1, const char* str2) { // Compare the order of two ipv4 address. 
    char str1Buf[IPV4_LEN], str2Buf[IPV4_LEN];
    short ipVal[4];

    sscanf(str1, "%hd.%hd.%hd.%hd", &ipVal[0], &ipVal[1], &ipVal[2], &ipVal[3]); // xxx.xxx.xxx.xxx form -> extract each "xxx" value.
    sprintf(str1Buf, "%03hd.%03hd.%03hd.%03hd", ipVal[0], ipVal[1], ipVal[2], ipVal[3]);
    sscanf(str2, "%hd.%hd.%hd.%hd", &ipVal[0], &ipVal[1], &ipVal[2], &ipVal[3]); // xxx.xxx.xxx.xxx form -> extract each "xxx" value.
    sprintf(str2Buf, "%03hd.%03hd.%03hd.%03hd", ipVal[0], ipVal[1], ipVal[2], ipVal[3]);

    return strcmp(str1Buf, str2Buf);
}

/* List of functions of using to get partition information (from /proc/diskstats, /proc/mounts) */
void get_Partition_IO_Speed(int partitionCnt, int idx, const char* fileSystem, float* readSpeed, float* writeSpeed){ // Get speed of Partition's Read/Write Speed
    FILE *fp = NULL;
    size_t read = 0, write = 0, sector_size = 0;
    static size_t *priv_read = NULL, *priv_write = NULL;
    char target[BUF_MAX_LINE], compTarget[BUF_MAX_LINE]; // for dm-x

    if (priv_read == NULL){
        if ((priv_read = (size_t*)calloc(partitionCnt, sizeof(size_t))) == NULL) {
            exception(-4, "get_Partition_IO_Speed", "calloc() - priv_read");
            return;
        }
    }

    if (priv_write == NULL){
        if ((priv_write = (size_t*)calloc(partitionCnt, sizeof(size_t))) == NULL) {
            exception(-4, "get_Partition_IO_Speed", "calloc() - priv_write");
            return;
        }
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

void get_MountPath_from_FileSystem(const char* fileSystem, char*** mountPath, int* mountPathCnt){ // Get the mount path using fileSystem path of Disk.
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
    if ((*mountPath = (char**)calloc((*mountPathCnt), sizeof(char*))) == NULL) { // Allocate 2D-Array for mountPath && Initialization to NULL
        exception(-4, "get_MountPath_from_FileSystem", "calloc() - mountPath");
        return;
    }

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
                    if (((*mountPath)[mountIdx] = (char*)malloc(strlen(mountBuf) * sizeof(char))) == NULL) {
                        sprintf(lineBuf, "malloc() - mountPath[%d]", mountIdx);
                        exception(-4, "get_MountPath_from_FileSystem", lineBuf);
                        return;
                    }
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

int get_Partition_Information(PartitionInfo** partition_list_ptr, int* partition_cnt){ // Get the partition list and capacity of each partitions.
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
    if ((*partition_list_ptr = (PartitionInfo*)malloc(cnt * sizeof(PartitionInfo))) == NULL) { // Allocate new memory.
        exception(-4, "get_Partition_Information", "malloc() - partition_list_ptr");
        return -100;
    }
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