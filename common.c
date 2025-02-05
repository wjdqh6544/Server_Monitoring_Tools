#include "0_usrDefine.h"
#include "common.h"
#include "zz_struct.h"
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <dirent.h>
#include <unistd.h>

DateInfo dateBuf;
FILE* log_ptr = NULL;

void get_Date() { // Get system time
    time_t current_time;
    struct tm *tm;
    current_time = time(NULL);
    tm = localtime(&current_time);
    
    dateBuf.year = (short)((tm->tm_year) + 1900);
    dateBuf.month = (short)((tm->tm_mon) + 1);
    dateBuf.day = (short)(tm->tm_mday);
    dateBuf.hrs = (short)(tm->tm_hour);
    dateBuf.min = (short)(tm->tm_min);
    dateBuf.sec = (short)(tm->tm_sec);
}

int check_Package_Installed(int type) { // Check package installation dependency. 
    FILE* command = NULL;
    char buf[BUF_MAX_LINE] = { '\0' };

    if (type == TYPE_CHECK_OMREPORT) {
        strcpy(buf, CHECK_OMREPORT);
    } else if (type == TYPE_CHECK_PERCCLI) {
        strcpy(buf, CHECK_PERCCLI);
    } else {
        return -1;
    }

    if ((command = popen(buf, "r")) == NULL) {
        return -100;
    } else {
        fgets(buf, sizeof(buf), command);
        if (strstr(buf, "No such file or directory") != NULL){
            return 0;
        }
        pclose(command);
    }
    return 1;
}

void exception(int code, char *func_name, char *detail) { // Print exception statement.
    char type[ERROR_MSG_LEN];
    char *detail_str = (detail != NULL) ? detail : "";
    switch (code) {
        case -1:
            strcpy(type, "Open File");
            break;
        case -2:
            strcpy(type, "Read Data");
            break;
        case -3:
            strcpy(type, "Write Data");
            break;
        case -4:
            strcpy(type, "Invoke System Call");
            break;
        case -5:
            fprintf(log_ptr, "[WARNING] %04d-%02d-%02d %02d:%02d:%02d (func. - %s) Invalid function parameter: %s\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, func_name, detail_str);
            return;
    }
    fprintf(log_ptr, "[WARNING] %04d-%02d-%02d %02d:%02d:%02d (func. - %s) Cannot %s: %s\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, func_name, type, detail_str);
}

void get_Filename(char* fullpathBuf, char* path, char* filename, const DateInfo* targetDate) { // Create a full path of Log file.
    if (path[strlen(path) - 1] == '/'){ // filename form: <type>-YYYYMMDD
        if (strcmp(filename, WARNING_LOG) == 0) {
            sprintf(fullpathBuf, "%s%s-%04d%02d", path, filename, targetDate->year, targetDate->month); // <path(Directory)>/warning_history-YYYYMM
        } else {
            sprintf(fullpathBuf, "%s%s-%04d%02d%02d", path, filename, targetDate->year, targetDate->month, targetDate->day); // <path(Directory)>/<type>-YYYYMMDD
        }
    } else {
        if (strcmp(filename, WARNING_LOG) == 0){
            sprintf(fullpathBuf, "%s/%s-%04d%02d", path, filename, targetDate->year, targetDate->month); // <path(Directory)>/warning_history-YYYYMM
        } else {
            sprintf(fullpathBuf, "%s/%s-%04d%02d%02d", path, filename, targetDate->year, targetDate->month, targetDate->day); // <path(Directory)>/<type>-YYYYMMDD
        }
    }
}

int check_Log_Directory(char* fullpath, mode_t permissions) { // Check the presence of directory, and Create it if not exists.
    DIR* dir_ptr = NULL;
    char path[MAX_LOG_PATH_LEN] = { '\0' };
    char* path_ptr = path + 1;
    if ((dir_ptr = opendir(fullpath)) == NULL){ // If directory does not exist.
        strcpy(path, fullpath);
        while(*path_ptr){ // To create the directory recursively. (Child directory exists.)
            if((*path_ptr) == '/') {
                (*path_ptr) = '\0';
                if ((dir_ptr = opendir(path)) == NULL) { // Check whether parent directory exists.
                    if (mkdir(path, permissions) == -1){
                        return -1;
                    }
                } else {
                    closedir(dir_ptr);
                }
                (*path_ptr) = '/';
            }
            path_ptr++;
        }
        if (mkdir(path, permissions) == -1){ // Create last child directory. 
            return -1;
        }
    } else {
        closedir(dir_ptr);
    }
    return 0;
}

float get_Capacity_Percent(size_t total, size_t use) { // Calculate the percent of Used space
    (total == 0) ? exception(-5, "get_Capacity_Percent", "Total value cannot be 0") : 0;
    return (float)((use / (double)total) * 100);
}

double convert_Size_Unit(double size, int autoUnit, UNIT* unit) { // Convert to the largest capacity unit. (Minimum: KB / ex. 1,048,576 KB -> 1 GB)
    double res = size;
    int i = 0;
    if (autoUnit == 1){ // Convert size to largest unit automatically.
        for (i = 0; ((res >= 1024) && (i < UNIT_COUNT)); i++) {
            res /= 1024;
        }
        *unit = (UNIT)i; // Return converted unit.
    } else { // Convert size to entered Unit.
        for (i = 0; i < (int)(*unit); i++){
            res /= 1024;
        }
    }
    return res;
}

void remove_Space_of_Head(char* ptr) { // Remove the space at the head of String.
    char* start = ptr;
    while (*start == ' ') {
        start++;
    }

    while (*start != '\0') {
        *ptr++ = *start++;
    }
    *ptr = '\0';
    return;
}

void remove_Space_of_Tail(char* ptr) { // Remove the space at the tail of String.
    int idx = strlen(ptr) - 1;

    while (idx >= 0 && ptr[idx] == ' ') {
        ptr[idx] = '\0';
        idx--;
    }
    return;
}

void free_Array(void** ptr) { // Unload allocated Array from memory.
    // ptr: Pointer of pointer that point 1D Array. (Address of the variable that contain 1D Array)
    if (*ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
    }
}

void check_before_running(char* username) { // Check execution conditions of this program
    int code = 0;

    if (geteuid() != 0) { // Check the program is run with root privileges.
        printf("\nThis program must be running with root privileges. (using sudo or as root)...\n\nexit.\n\n");
        exit(-1);
    }

    code = check_Package_Installed(TYPE_CHECK_OMREPORT); // Check whether "omreport" package is installed.
    switch (code) {
        case -1:
            printf("\nCheck_Package_Installed - Invalid Parameter.\n\n");
            exit(-1);
        case -100:
            printf("\nSystem Call(popen) invoking ERROR... \nexit.\n\n");
            exit(-1);
        case 0:
            printf("\n- OMSA (OpenManage Server Administrator) is not installed..");
            printf("\n- Please install OMSA. (Package Name: OM-SrvAdmin-Dell-Web-LX)");
            printf("\n- To download rpm package, please visit DELL Website.\n\nexit.\n\n");
            exit(-1);
    }

    code = check_Package_Installed(TYPE_CHECK_PERCCLI); // Check whether "perccli" package is installed.
    switch (code) {
        case -1:
            printf("\nCheck_Package_Installed - Invalid Parameter.\n\n");
            exit(-1);
        case -100:
            printf("\nSystem Call(popen) invoking ERROR... \nexit.\n\n");
            exit(-1);
        case 0:
            printf("\n- PERCCLI(PERC controller CLI utility) is not installed..");
            printf("\n- Please install Perccli. (Package Name: Perccli)");
            printf("\n- To download rpm package, please visit DELL Website.");
            printf("\n- This program uses perccli64. If the system is 32bit, Edit define macro. (Is in 0_usrDefine.h)\n\nexit.\n\n");
            exit(-1);
    }

    if (check_Log_Directory(HISTORY_PATH, 0750) == -1){ // Check the presense of "/var/log/00_Server_Monitoring/00_history" directory. (History file is saved to this.)
        printf("\nCannot create Log destination directory. (%s).\nexit.\n\n", HISTORY_PATH);
        exit(-1);
    }

    strcpy(username, getpwuid(atoi(getenv("SUDO_UID")))->pw_name); // Get a username that run this (background collector) program.
}

void IO_Redirection(int type) { // I/O Redirection 
    // Collector: stdin -> Close, stdout & stderr -> Redirect to file (collector_log)
    // Server_monitoring_tools: stderr -> Redirect to file (monitoring_tool_log)
    int fd = -1;

    fflush(stdout);
    fflush(stderr);

    if (type == TYPE_COLLECTOR) { // Collector log
        log_ptr = fopen(ERROR_LOG_COLLECTOR, "a");
    } else { // Monitoring tools log
        log_ptr = fopen(ERROR_LOG_MONITOR, "a");
    }

    if (log_ptr == NULL) {
        printf("\nFailed to create / open Log file...\nexit.\n\n");
        exit(-1);
    }

    if (dup2(fileno(log_ptr), STDERR_FILENO) == -1){ // Redirect stderr to file
        printf("\nSystem Call(dup2 - stderr) invoking ERROR... \nexit.");
        exit(-1);
    }

    if (type == TYPE_COLLECTOR) { // Collector -> Close stdin.
        fd = open("/dev/null", O_RDONLY);
        if (dup2(fd, STDIN_FILENO) == -1){ // Close (disable) stdin
            printf("\nSystem Call(dup2 - stdin) invoking ERROR... \nexit.");
            exit(-1);
        }

        close(fd);

        if (dup2(fileno(log_ptr), STDOUT_FILENO) == -1) { // Connect stdout to file
            printf("\nSystem Call(dup2 - stdout) invoking ERROR... \nexit.");
            exit(-1);
        }
    }

    setvbuf(stdout, NULL, _IOLBF, 0); // Set buffering mode (Line buffering)
    setvbuf(stderr, NULL, _IOLBF, 0);
    setvbuf(log_ptr, NULL, _IOLBF, 0); // Set buffering mode (Line buffering) for log_ptr (Log)
}

void get_Hostname(char* buf) { // Get Linux Hostname
    FILE* fp = NULL;
    if ((fp = fopen(GET_HOSTNAME_PATH, "r")) == NULL) {
        strcpy(buf, "N/A");
        exception(-1, "get_Hostname", GET_HOSTNAME_PATH);
        return;
    }

    fgets(buf, MAX_PARTS_NAME_LEN, fp);
    fclose(fp);
    buf[strlen(buf) - 1] = '\0';
}

void get_External_IPv4(char* ipBuf) { // Get IPv4 (External IP): Using ifconfig.me
    FILE* curl_ptr = NULL;
    char errBuf[ERROR_MSG_LEN];
    if ((curl_ptr = popen(GET_EXTERNAL_IPV4_ADDR, "r")) == NULL) {
        sprintf(errBuf, "popen() - %s", GET_EXTERNAL_IPV4_ADDR);
        exception(-4, "get_External_IPv4", errBuf);
        STR_INIT(ipBuf);
        return;
    }

    fscanf(curl_ptr, "%s", ipBuf);
    if (strlen(ipBuf) == 0) {
        STR_INIT(ipBuf);
    }
}

void get_UpTime(char* buf) { // Get Uptime from System Boot
    struct sysinfo info;
    char res[BUF_MAX_LINE] = { '\0' };
    if (sysinfo(&info) == -1) {
        strcpy(buf, "N/A");
        exception(-4, "get_Uptime", "sysinfo()");
        return;
    }
    snprintf(res, sizeof(res), "%d days, %02d:%02d:%02d", (int)((info.uptime / 3600) / 24)%10000u, (int)((info.uptime % (24 * 3600)) / 3600)%100u, (int)((info.uptime % 3600) / 60)%100u, (int)(info.uptime % 60)%100u);
    strcpy(buf,res);
}
