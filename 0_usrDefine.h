#ifndef USRDEFINE_H

#define USRDEFINE_H

#define UNIT_COUNT 6

/* Common */
#define CHECK_OMREPORT "/opt/dell/srvadmin/bin/omreport 2>&1"
#define CHECK_PERCCLI "/opt/MegaRAID/perccli/perccli64 2>&1"
#define ERROR_MSG_LEN 256
#define CPU_TEMP_CRITICAL_POINT 65
#define CPU_USAGE_CRITICAL_PERCENT 85
#define DISK_USAGE_CRITICAL_PERCENT 90
#define EXHAUST_TEMP_CRITICAL_POINT 55
#define INLET_TEMP_CRITICAL_POINT 25
#define MEM_USAGE_CRITICAL_PERCENT 90
#define RAID_CORE_TEMP_CRITICAL_POINT 65
#define RAID_CTRL_TEMP_CRITICAL_POINT 55
#define STORAGE_TEMP_CRITICAL_POINT 60
#define SWAP_USAGE_CRITICAL_PERCENT 0

/* Main Program */
#define ERROR_LOG_MAIN "/var/log/00_Server_Monitoring/main_program_log"

/* Warning Log */
#define TYPE_MEM_USAGE 1
#define TYPE_CPU_USAGE 2
#define TYPE_CPU_TEMP 3
#define TYPE_STORAGE_TEMP 4
#define TYPE_RAID_CORE_TEMP 5
#define TYPE_RAID_CTRL_TEMP 6
#define TYPE_EXHAUST_TEMP 7
#define TYPE_INLET_TEMP 8
#define WARNING_LOG "warning_history"

/* Information to Log */
#define ERROR_LOG_COLLECTOR "/var/log/00_Server_Monitoring/collector_log"
#define HISTORY_PATH "/var/log/00_Server_Monitoring/00_history" // 40
#define LOG_PATH "/var/log/00_Server_Monitoring"
#define USERNAME_LEN 64

/* Information to Log - Temperature */
#define CPU1_TEMP "CPU1"
#define CPU2_TEMP "CPU2"
#define STORAGE_TEMP_FORM "Drive Temperature = %hdC%*s"
#define EXHAUST_TEMP "Exhaust"
#define GET_TEMP_OMREPORT_COMMAND "/opt/dell/srvadmin/bin/omreport chassis temps"
#define GET_RAID_TEMP_PERCCLI_COMMAND "/opt/MegaRAID/perccli/perccli64 /c0 show temperature"
#define GET_STORAGE_TEMP_PERCCLI_COMMAND "/opt/MegaRAID/perccli/perccli64 /c0/eall/sall show all | grep Temperature"
#define INLET_TEMP "Inlet"
#define MAX_CPU_COUNT 2
#define MAX_STORAGE_COUNT 16
#define MAX_LOG_PATH_LEN 70 // LogFile Location: /var/log/~: 40(path) + 29(filename: <type>_history-YYYYDDMM)
#define RAID_CORE_TEMP "ROC temperature(Degree Celsius)"
#define RAID_CTRL_TEMP "Ctrl temperature(Degree Celsius)"
#define STAT_FORM "cpu  %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld"
#define STAT_LOCATION "/proc/stat"
#define TEMP_LOG "temperature_history"
#define TEMP_MAX_LINE 128

/* Information to Log - Usage */
#define MEMFREE "MemAvailable:"
#define MEMINFO_MAX_LINE 64
#define MEMINFO_LOCATION "/proc/meminfo"
#define MEMINFO_FORM "%s %ld"
#define MEMTOTAL "MemTotal:"
#define SWAPTOTAL "SwapTotal:"
#define SWAPFREE "SwapFree:"
#define USAGE_LOG "usage_history"

#endif