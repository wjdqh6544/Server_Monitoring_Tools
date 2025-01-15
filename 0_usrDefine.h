#ifndef USRDEFINE_H

#define USRDEFINE_H

#define ERROR_MSG_LEN 256
#define LOG_PATH "/var/log/00_Server_Monitoring"
#define HISTORY_PATH "/var/log/00_Server_Monitoring/00_history" // 40
#define UNIT_COUNT 6
#define DISK_COUNT 16

#define MAX_LOGFILE_NAME_LEN 29 // <type>_history-YYYYDDMM
#define MAX_LOG_PATH_LEN 70 // LogFile Location: /var/log/~
#define GET_TEMP_COMMAND "/opt/dell/srvadmin/bin/omreport chassis temps"
#define TEMP_MAX_LINE 128
#define TEMP_LOG "Temperature_history"


#endif