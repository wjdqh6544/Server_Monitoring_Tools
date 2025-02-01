#ifndef USRDEFINE_H

#define USRDEFINE_H
#define STR_INIT(x) strcpy(x, NA_STR)
#define STRN_CMP_EQUAL(x,y) (strlen(x) >= strlen(y)) && (strncmp(x, y, strlen(y)) == 0)
#define CHECK_EXIST_USER(x) (strstr(x, USER_NOEXIST) == NULL)

/* User custom */
#define UNIT_COUNT 6 // Number of UNIT element. (Enumerate type, in zz_struct.h)
#define MAX_CPU_COUNT 2 // Maximum number of CPUs that supported by a server unit.
#define MAX_FAN_COUNT 6 // Maximum number of Cooling fan that supported by a server unit.
#define MAX_PSU_COUNT 2 // Maximum number of Power Supply that supported by a server unit.
#define MAX_STORAGE_COUNT 16 // Number of Storage that supported by a server unit.

#define BUF_MAX_LINE 256

/* Critical Point */
#define BBU_CAPACITY_CRITICAL_POINT 200
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

/* common */
#define COMMAND_MAX_LINE 512
#define CHECK_OMREPORT "/opt/dell/srvadmin/bin/omreport 2>&1"
#define CHECK_PERCCLI "/opt/MegaRAID/perccli/perccli64 2>&1"
#define ERROR_LOG_MAIN "/var/log/00_Server_Monitoring/main_program_log"
#define ERROR_MSG_LEN 256
#define NA_STR "N/A"
#define MAX_PARTS_NAME_LEN 64
#define INFO_CMOS_BATTERY_GOOD "Good"
#define INFO_CPU_PROCESSOR_NAME "Processor Brand"
#define INFO_CPU_STATE "State"
#define INFO_CPU_CORE_CNT "Core Count"
#define INFO_FORM_STR " : %[^\n]s"
#define INFO_FORM_NUM " : %d"
#define INFO_FORM_NUM_SHORT " : %hd"
#define INFO_HOSTNAME "Host Name"
#define INFO_INDEX "Index"
#define INFO_MEM_ERROR_COLLECTION "Error Correction"
#define INFO_MEM_INSTALLED_CAP "Installed Capacity"
#define INFO_MEM_SLOT_TOTAL "Slots Available"
#define INFO_MEM_SLOT_USED "Slots Used"
#define INFO_MEM_UNIT_CONNECTOR "Connector Name"
#define INFO_MEM_UNIT_TYPE "Type"
#define INFO_MEM_UNIT_SIZE "Size"
#define INFO_NA "NA"
#define INFO_NAME "Probe Name"
#define INFO_NICS_CON_STATUS "Connection Status"
#define INFO_NICS_CONNECT "Connected"
#define INFO_NICS_DISCONNECT "Disconnected"
#define INFO_NICS_DESCRIPTION "Description"
#define INFO_NICS_DETAIL_INTERFACE "Interface: %s"
#define INFO_NICS_DETAIL_VALUE "Value"
#define INFO_NICS_DETAIL_SPEED "Speed"
#define INFO_NICS_INTERFACE_NAME "Interface Name"
#define INFO_NICS_TEAM_INTERFACES "Team Interface(s)"
#define INFO_READING "Reading"
#define INFO_STATUS "Status"
#define INFO_STATE_PRESENT "Present"
#define INFO_STATUS_OK "Ok"
#define INFO_STATUS_UNKNOWN "Unknown"
#define INFO_SERVER_MODEL "Chassis Model"
#define INFO_SERVICE_TAG "Chassis Service Tag"
#define INFO_SERVICE_CODE "Express Service Code"
#define TYPE_NICS_DISCONNECT 0
#define TYPE_NICS_CONNECT 1
#define TYPE_STATUS_OK 1
#define TYPE_STATUS_UNKNOWN -100
#define TYPE_STATUS_CRITICAL 0

/* hw_info.c */
#define DISK_INFO_FORM "%hd:%hd %d %s %s %s %s %s %s"
#define GET_CMOS_BATTERY_INFO_COMMAND "/opt/dell/srvadmin/bin/omreport chassis batteries"
#define GET_CPU_INFO_COMMAND "/opt/dell/srvadmin/bin/omreport chassis processors"
#define GET_DISK_COUNT "/opt/MegaRAID/perccli/perccli"
#define GET_DISK_INFO_COMMAND "/opt/MegaRAID/perccli/perccli64 /call show all"
#define GET_DISK_NAME "/opt/MegaRAID/perccli/perccli64 /call/eall/sall show all"
#define GET_FAN_INFO_COMMAND "/opt/dell/srvadmin/bin/omreport chassis fans"
#define GET_MEM_INFO_COMMAND "/opt/dell/srvadmin/bin/omreport chassis memory"
#define GET_NICS_INFO_COMMAND "/opt/dell/srvadmin/bin/omreport chassis nics"
#define GET_NICS_INFO_COMMAND_DETAIL "/opt/dell/srvadmin/bin/omreport chassis nics index=%d"
#define GET_PSU_INFO_COMMAND "/opt/dell/srvadmin/bin/omreport chassis pwrsupplies"
#define GET_SERVER_INFO_COMMAND "/opt/dell/srvadmin/bin/omreport chassis info"
#define GET_VDISK_LIST "/opt/MegaRAID/perccli/perccli64 /call/vall show"
#define GET_VDISK_FILESYSTEM "/opt/MegaRAID/perccli/perccli64 /call/vall show all"
#define MAX_BBU_VOLTAGE_LEN 10
#define MAX_BBU_CAPACITY_LEN 10
#define MAX_CAPACITY_LEN 16
#define MAX_DISK_MODELNAME_LEN 32
#define MAX_DISK_INTERFACE_LEN 5
#define MAX_DISK_MEDIATYPE_LEN 5
#define MAX_HBA_BIOS_VER_LEN 32
#define MAX_HBA_DRIVER_NAME_LEN 16
#define MAX_HBA_DRIVER_VER_LEN 32
#define MAX_HBA_FIRMWARE_VER_LEN 16
#define MAX_HBA_NAME_LEN 64
#define MAX_HBA_SERIAL_LEN 16
#define MAX_PHYSICAL_CPU_PATH_LEN 40
#define MAX_VDISK_NAME_LEN 32
#define MAX_VDISK_TYPE_LEN 7
#define PHYSICAL_CPU_PATH "/sys/devices/system/cpu"
#define PHYSICAL_CPU_PATH_FORM "/sys/devices/system/cpu/cpu%d/topology"
// DiskInfo.state Field
#define TYPE_DISK_STATE_ONLINE 0
#define TYPE_DISK_STATE_OFFLINE 1
#define TYPE_DISK_STATE_GHS 2 // Global HotSpare
#define TYPE_DISK_STATE_DHS 3 // Dedicated HotSpare
#define TYPE_DISK_STATE_UGOOD 4 // Unconfigured Good
#define TYPE_DISK_STATE_UBAD 5 // Unconfigured Bad
#define TYPE_DISK_STATE_SANI 6 // Sanitize
// BBUInfo.state
#define TYPE_STATUS_OPTIMAL 0
#define TYPE_STATUS_FAILED 1
#define TYPE_STATUS_DEGRADED 2
// HBAInfo.status
#define TYPE_HBA_STATUS_SUCCESS 0
#define TYPE_HBA_STATUS_FAILED 1
// BBUInfo.HBA_Cur_Personality
#define TYPE_HBA_CUR_HBA 0
#define TYPE_HBA_CUR_RAID 1
// For finding String in HBA Card
#define BBU_CAPACITY_UNIT "mAh"
#define BBU_VOLTAGE_UNIT "mV"
#define BBU_DESIGN_CAPACITY "Design Capacity"
#define BBU_FULL_CHARGE_CAPACITY "Full Charge Capacity"
#define BBU_REMAINING_CAPACITY "Remaining Capacity"
#define BBU_STATUS "Battery State "
#define BBU_STATUS_OPTIMAL "Optimal"
#define BBU_STATUS_DEGRADED "Degraded"
#define BBU_STATUS_FAILED "Failed"
#define BBU_VOLTAGE "Voltage"
#define HBA_BIOS_VER "Bios Version = "
#define HBA_CURRENT_PERSONALITY "Current Personality = "
#define HBA_CURRENT_PERSONALITY_HBA "HBA"
#define HBA_CURRENT_PERSONALITY_RAID "RAID"
#define HBA_DRIVE_GROUPS "Drive Groups = "
#define HBA_DRIVER_NAME "Driver Name = "
#define HBA_DRIVER_VER "Driver Version = "
#define HBA_FIRMWARE_VER "Firmware Version = "
#define HBA_PRODUCTNAME "Model = "
#define HBA_SERIAL_NUMBER "Serial Number = "
#define HBA_CONTROLLER_STATUS "Controller Status = " 
#define HBA_CONTROLLER_STATUS_OPTIMAL "Optimal"
#define HBA_CONTROLLER_STATUS_DEGRADED "Degraded"
#define HBA_CONTROLLER_STATUS_FAILED "Failed"
// For finding String in Physical Disk
#define DISK_NAME "Model Number = "
#define DISK_NAME_FILTER "Drive /"
#define DISK_NAME_FORM "Drive /c0/e%hd/s%hd Device attributes :"
#define DISK_PD_LIST "PD LIST :"
#define DISK_PD_LIST_FORM "%hd:%hd %hd %s %s %s %s %s %s"
#define DISK_PHYSICAL_COUNT "Physical Drives = "
#define DISK_STATE_ONLINE "Onln"
#define DISK_STATE_OFFLINE "Offln"
#define DISK_STATE_GHS "GHS" // Global HotSpare
#define DISK_STATE_DHS "DHS" // Dedicated HotSpare
#define DISK_STATE_UGOOD "UGood" // Unconfigured Good
#define DISK_STATE_UBAD "UBad" // Unconfigured Bad
#define DISK_STATE_SANI "Sntze" // Sanitize
//For finding String in Virtual Disk
#define VD_FILESYSTEM_FORM "OS Drive Name = "
#define VD_CNT "Virtual Drives = "
#define VD_LIST "Virtual Drives :"
#define VD_LIST_FORM "%hd/%hd %s %s %s %*s %*s %*s %*s %s %s %s"
#define VD_PROPERTIES_FORM "VD%d Properties :"
#define VD_STATE_OPTIMAL "Optl"
#define VD_STATE_DEGRADED "Dgrd"
#define VD_STATE_OFFLINE "OfLn"
#define VD_STATE_RECOVERY "Rec"
#define VD_STATE_PARTIALLY_DEGRADED "Pdgd"
#define VD_STATE_HIDDEN "HD"
#define VD_STATE_TRANSPORTREADY "TRANS"
#define VD_ACCESS_READ_ONLY "RO"
#define VD_ACCESS_READ_WRITE "RW"
#define VD_ACCESS_BLOCKED "B"
#define TYPE_VD_STATE_OPTIMAL 0
#define TYPE_VD_STATE_DEGRADED 1
#define TYPE_VD_STATE_OFFLINE 2
#define TYPE_VD_STATE_RECOVERY 3
#define TYPE_VD_STATE_PARTIALLY_DEGRADED 4
#define TYPE_VD_STATE_HIDDEN 5
#define TYPE_VD_STATE_TRANSPORTREADY 6
#define TYPE_VD_ACCESS_READ_ONLY 0
#define TYPE_VD_ACCESS_READ_WRITE 1
#define TYPE_VD_ACCESS_BLOCKED 2

/* os_info.c */
// Docker container
#define BRIDGE_PREFIX "(br)"
#define CHECK_BRIDGE_FORM "br-"
#define DOCKER_BRIDGE_FORM "%*s %s %*s %*s"
#define DOCKER_ZERO_INTERFACE "docker0"
#define DOCKER_VETH_INTERFACE_PREFIX "veth"
#define GET_DOCKER_CONTAINER_NAME "docker ps --format {{.Names}}"
#define GET_DOCKER_INSPECT_CONTAINER "docker inspect %s --format \"{{.State.Pid}}\""
#define GET_DOCKER_BRIDGE_NAME "docker network ls | grep %s"
#define GET_DOCKER_VETH_NAME "ip -br addr | grep if%d | awk \'{print $1}\' | cut -d '@' -f 1"
#define GET_DOCKER_VETH_IP "nsenter -t %d -n ip addr show"
#define IGMP_LOCATION "/proc/%d/net/igmp"
#define IGMP_FORM "%d %*s %*s %*s %*s"
#define NSENTER_FORM " inet %[^/]"
#define MAX_DOCKER_CONTAINER_NAME_LEN 128
// File Permissions
#define GRPNAME_LEN 32
#define MAX_PATH_LEN 32
#define STAT_HISTORY_LOG "stat_history"
#define STATUS_LEN 8
#define FAILED_CHANGING -1
#define FAILED_CHANGING_STR "Failed"
#define NEED_CHANGING 1
#define NEED_CHANGING_STR "Updated"
#define NOT_NEED_CHANGING 0
#define NOT_NEED_CHANGING_STR "OK"
// Linux Users
#define GET_LATEST_LOGIN_HISTORY_COMMAND "last -n 1 "
#define LOGIN_DEFS_LOCATION "/etc/login.defs"
#define PASSWD_LOCATION "/etc/passwd"
#define PASSWD_FORM "%s"
#define UID_FORM "%*s %d"
#define UID_MIN_STR "UID_MIN"
#define UID_MAX_STR "UID_MAX"
#define WTMP_LOCATION "/var/log/wtmp"
//Linux Users Login
#define BTMP_WTMP_FILENAME_LEN 16
#define BTMP_WTMP_LOCATION "/var/log/"
#define BTMP_NAME "btmp"
#define WTMP_NAME "wtmp"
#define TMP_LOCATION_FORM "%s%s"
#define USER_NOEXIST "(NoExist)"
#define GET_TMPLOG_LIST "ls -al %s | awk \'{print $9}\' | grep -E \"^%s*|^%s*\""
#define LOGIN_SUCCESS 1
#define LOGIN_SUCCESS_STR "SUCCESS"
#define LOGIN_FAILED 0
#define LOGIN_FAILED_STR "FAILED"
#define IP_LOCAL_STR "Local / Console"
#define IP_OTHER_STR "Other Methods"
// Networks Interface
#define IPV4_LEN 16
#define NET_DEV_FORM "%s %ld %*d %ld %ld %*d %*d %*d %*d %ld %*d %ld %ld %*d %*d %*d %*d"
// Partitions
#define DISKSTATS_FORM "%*d %*d %s %*d %*d %ld %*d %*d %*d %ld %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d"
#define DISKSTATS_LOCATION "/proc/diskstats"
#define PATH_TMPFS "tmpfs"
#define GET_MOUNTPATH_LSBLK "lsblk -a"
#define MAX_FILESYSTEM_LEN 4096
#define MAX_MOUNTPATH_LEN 4096
#define MAX_SECTOR_PATH_LEN 512
#define MOUNTS_FORM "%s %s %*s %*s %*s %*s"
#define MOUNTS_LOCATION "/proc/mounts"
#define GET_SECTOR_SIZE "/sys/block/%s/queue/hw_sector_size"
#define DM_PATH "dm-"
// Process Status (ps)
#define GET_PS_COMMAND "ps -aux --sort=-%mem"
#define GET_PS_INFO_FORM "%s %d %f %f %*s %ld %s %*s %s %s %[^\n]s"
#define TTY_BACKGROUND "?"
#define TTY_BACKGROUND_STR "<BG>"
#define TIME_LEN 10

/* Warning Log */
#define TYPE_MEM_USAGE 1
#define TYPE_CPU_USAGE 2
#define TYPE_CPU_TEMP 3
#define TYPE_STORAGE_TEMP 4
#define TYPE_RAID_CORE_TEMP 5
#define TYPE_RAID_CTRL_TEMP 6
#define TYPE_BBU_TEMP 7
#define TYPE_EXHAUST_TEMP 8
#define TYPE_INLET_TEMP 9
#define WARNING_LOG "warning_history"

/* Information to Log */
#define ERROR_LOG_COLLECTOR "/var/log/00_Server_Monitoring/collector_log"
#define HISTORY_PATH "/var/log/00_Server_Monitoring/00_history" // 40
#define LOG_PATH "/var/log/00_Server_Monitoring"
#define USERNAME_LEN 33

/* Information to Log - Temperature */
#define BBU_TEMP "Temperature"
#define CPU1_TEMP "CPU1"
#define CPU2_TEMP "CPU2"
#define STORAGE_TEMP_FORM "Drive Temperature = %hdC%*s"
#define EXHAUST_TEMP "Exhaust"
#define GET_BBU_INFO_COMMAND "/opt/MegaRAID/perccli/perccli64 /c0/bbu show all"
#define GET_TEMP_OMREPORT_COMMAND "/opt/dell/srvadmin/bin/omreport chassis temps"
#define GET_RAID_TEMP_PERCCLI_COMMAND "/opt/MegaRAID/perccli/perccli64 /c0 show temperature"
#define GET_STORAGE_TEMP_PERCCLI_COMMAND "/opt/MegaRAID/perccli/perccli64 /c0/eall/sall show all | grep Temperature"
#define INLET_TEMP "Inlet"
#define MAX_LOG_PATH_LEN 70 // LogFile Location: /var/log/~: 40(path) + 29(filename: <type>_history-YYYYDDMM)
#define RAID_CORE_TEMP "ROC temperature(Degree Celsius)"
#define RAID_CTRL_TEMP "Ctrl temperature(Degree Celsius)"
#define STAT_FORM "%*s %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld"
#define STAT_LOCATION "/proc/stat"
#define TEMP_LOG "temperature_history"

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