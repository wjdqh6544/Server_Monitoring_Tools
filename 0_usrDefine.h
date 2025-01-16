#ifndef USRDEFINE_H

#define USRDEFINE_H

#define UNIT_COUNT 6
#define DISK_COUNT 16

/* Common */
#define ERROR_MSG_LEN 256

/* Main Program */
#define ERROR_LOG_MAIN "/var/log/00_Server_Monitoring/main_program_log"

/* Information to Log */
#define LOG_PATH "/var/log/00_Server_Monitoring"
#define HISTORY_PATH "/var/log/00_Server_Monitoring/00_history" // 40
#define ERROR_LOG_COLLECTOR "/var/log/00_Server_Monitoring/collector_log"
#define USERNAME_LEN 64

/* Information to Log - Temperature */
#define MAX_LOG_PATH_LEN 70 // LogFile Location: /var/log/~: 40(path) + 29(filename: <type>_history-YYYYDDMM)
#define GET_TEMP_COMMAND "/opt/dell/srvadmin/bin/omreport chassis temps"
#define TEMP_MAX_LINE 128
#define TEMP_LOG "temperature_history"
#define STAT_LOCATION "/proc/stat"
#define STAT_FORM "cpu  %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld"

/* Information to Log - Usage */
#define USAGE_LOG "usage_history"
#define MEMINFO_MAX_LINE 64
#define MEMINFO_LOCATION "/proc/meminfo"
#define MEMINFO_FORM "%s %ld"
#define MEMTOTAL "MemTotal:"
#define MEMFREE "MemAvailable:"
#define SWAPTOTAL "SwapTotal:"
#define SWAPFREE "SwapFree:"

#endif