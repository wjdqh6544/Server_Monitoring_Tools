#include "0_usrDefine.h"
#include "common.h"
#include "hw_info.h"
#include "info_from_log.h"
#include "os_info.h"
#include "resources_monitor.h"
#include "server_monitoring_tools.h"
#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <unistd.h>

extern const DateInfo dateBuf;
extern struct winsize wbuf;
extern int invoked_menu;
extern const char hostnameBuf[];
int invoked_SubScreen = 0;
const Unit_Mapping unitMap[] = {
    { KB, "KB" },
    { MB, "MB" },
    { GB, "GB" },
    { TB, "TB" },
    { PB, "PB" },
    { EB, "EB" }
};
const char* title = SUBTITLE;
const char* footLabel = "To restore main screen, Press \"q\".";

/* List of functions relative to 1st feature (Display System Information)*/
void one_System_Information() { // Display System Information - HostName, Server Model, Service Tag / Express Service Code, CPU / Memory / CMOS Battery / Fans / PSU Status
    invoked_menu = 1;

    SystemInfo sysInfoBuf;
    int pos_x = 3, pos_y = 1, CPU_OK_CNT = 0;
    char uptimeBuf[BUF_MAX_LINE] =  { '\0' };
    WINDOW *waitWin = newwin(wbuf.ws_row, wbuf.ws_col, 0, 0); // Screen for Waiting OMSA
    wattron(waitWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wmove(waitWin, 0, 0);
    whline(waitWin, ' ', wbuf.ws_col - 1);
    mvwaddch(waitWin, 0, wbuf.ws_col - 1, ' ');
    mvwprintw(waitWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
    wattroff(waitWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    mvwprintw(waitWin, pos_y + 1, pos_x, "!! Information !!");
    mvwprintw(waitWin, pos_y + 3, pos_x + 2, "- Collecting data from PERCCLI Package...");
    mvwprintw(waitWin, pos_y + 4, pos_x + 2, "- Please wait a moment. (Up to 60 Seconds)");
    mvwprintw(waitWin, pos_y + 5, pos_x + 2, "- If this program stops on this screen, Press Ctrl+C or Ctrl+\\ to exit.");
    refresh();
    wrefresh(waitWin);

    get_System_Information_from_Omreport(&sysInfoBuf);

    delwin(waitWin);
    WINDOW *headerWin = newwin(1, wbuf.ws_col, 0, 0);
    WINDOW *userWin = newwin(wbuf.ws_row - 1 - 1, wbuf.ws_col, 1, 0);
    WINDOW *footerWin = newwin(1, wbuf.ws_col, wbuf.ws_row - 1, 0);

    while(invoked_menu == 1) {
        CPU_OK_CNT = 0;
        pos_x = 3;
        pos_y = 1;
        wbkgd(headerWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wbkgd(footerWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        mvwprintw(headerWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
        mvwprintw(userWin, pos_y, pos_x - 1, "1. System Information");
        get_UpTime(uptimeBuf);
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 6, "- Uptime: %s", uptimeBuf);
        mvwprintw(userWin, pos_y += 2, pos_x, "- Server Hostname: %s", sysInfoBuf.hostname);
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 6, "- Server ModelName: %s", sysInfoBuf.serverModel);
        mvwprintw(userWin, pos_y += 1, pos_x, "- Server ServiceTag: %s", sysInfoBuf.serviceTag);
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 6, "- Express ServiceCode: %s", sysInfoBuf.serviceCode);

        wattron(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wmove(userWin, pos_y + 2, 0);
        whline(userWin, ' ', wbuf.ws_col - 1);
        mvwaddch(userWin, pos_y + 2, wbuf.ws_col - 1, ' ');
        mvwprintw(userWin, pos_y += 2, 3, "Parts Infomration");
        wattroff(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));

        if (sysInfoBuf.psuStatus[0] == TYPE_STATUS_OK) {
            mvwprintw(userWin, pos_y += 2, pos_x, "- PSU #1 Status: OK");
        } else {
            mvwprintw(userWin, pos_y += 2, pos_x, "- PSU #1 Status: Critical (Need to Check / Replace)");
        }

        if (sysInfoBuf.psuStatus[1] == TYPE_STATUS_OK) {
            mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 6, "- PSU #2 Status: OK");
        } else {
            mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 4, "- PSU #2 Status: Critical (Need to Check / Replace)");
        }

        if (sysInfoBuf.cmosBattery.status == TYPE_STATUS_OK) {
            mvwprintw(userWin, pos_y += 2, pos_x, "- %s Status: OK (Good)", sysInfoBuf.cmosBattery.name);
        } else {
            mvwprintw(userWin, pos_y += 2, pos_x, "- %s Status: Critical (Need to Check / Replace)", sysInfoBuf.cmosBattery.name);
        }

        pos_y++;
        for (int i = 0; i < MAX_FAN_COUNT / 2; i++) {
            if (sysInfoBuf.fan[i].status == TYPE_STATUS_OK) {
                mvwprintw(userWin, pos_y += 1, pos_x, "- %s: OK (%s)", sysInfoBuf.fan[i].name, sysInfoBuf.fan[i].rpm);
            } else {
                mvwprintw(userWin, pos_y += 1, pos_x, "- %s: Critical (Need to Check / Replace)", sysInfoBuf.fan[i].name);
            }

            if (sysInfoBuf.fan[i + 1].status == TYPE_STATUS_OK) {
                mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 6, "- %s: OK (%s)", sysInfoBuf.fan[i + 1].name, sysInfoBuf.fan[i + 1].rpm);
            } else {
                mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 6, "- %s: Critical (Need to Check / Replace)", sysInfoBuf.fan[i + 1].name);
            }
        }

        pos_y++;
        for (int i = 0; i < MAX_CPU_COUNT; i++) {
            if (STRN_CMP_EQUAL(sysInfoBuf.cpu[i].name, "N/A")) {
                mvwprintw(userWin, pos_y += 1, pos_x, "- CPU%d: %s", i + 1, sysInfoBuf.cpu[i].name);
            } else {
                if (sysInfoBuf.cpu[i].status == TYPE_STATUS_OK) {
                    mvwprintw(userWin, pos_y += 1, pos_x, "- CPU%d: %s (%d Cores)  |  Status: OK (%s)", i + 1, sysInfoBuf.cpu[i].name, sysInfoBuf.cpu[i].coreCnt, INFO_STATE_PRESENT);
                    CPU_OK_CNT++;
                } else {
                    mvwprintw(userWin, pos_y += 1, pos_x, "- CPU%d: %s (%d Cores)  |  Status: Critical (Need to Check / Replace)", i + 1, sysInfoBuf.cpu[i].name, sysInfoBuf.cpu[i].coreCnt);
                }
            }
        }
        if (strstr(sysInfoBuf.mem.errorCorrection, "ECC") == NULL) {
            mvwprintw(userWin, pos_y += 2, pos_x, "- Installed Memory: %s | ECC Support: None | RAM Slot: %hd/%hd/%hd (Used/Avail./Total)", 
                sysInfoBuf.mem.installedCapacity, sysInfoBuf.mem.slotsUsed, CPU_OK_CNT * (sysInfoBuf.mem.slotsTotal / MAX_CPU_COUNT), sysInfoBuf.mem.slotsTotal);
        } else {
            mvwprintw(userWin, pos_y += 2, pos_x, "- Installed Memory: %s | ECC Support: %s | RAM Slot: %hd/%hd/%hd (Used/Avail./Total)", 
                sysInfoBuf.mem.installedCapacity, sysInfoBuf.mem.errorCorrection, sysInfoBuf.mem.slotsUsed, CPU_OK_CNT * (sysInfoBuf.mem.slotsTotal / MAX_CPU_COUNT), sysInfoBuf.mem.slotsTotal);
        }
        pos_y++;
        for (int i = 0; i < sysInfoBuf.mem.slotsUsed; i++) {
            if (sysInfoBuf.mem.unit[i].status == TYPE_STATUS_OK) {
                mvwprintw(userWin, pos_y += 1, pos_x + 2, "Slot %s: %s  |  %s  |  %s", sysInfoBuf.mem.unit[i].connectorName, sysInfoBuf.mem.unit[i].type, sysInfoBuf.mem.unit[i].capacity, "OK");
            } else {
                mvwprintw(userWin, pos_y += 1, pos_x + 2, "Slot %s: %s  |  %s  |  %s", sysInfoBuf.mem.unit[i].connectorName, sysInfoBuf.mem.unit[i].type, sysInfoBuf.mem.unit[i].capacity, "Critical");
            }
        }


        // mvwprintw(footerWin, 1, 1, "To see the list of partitions and its usage, Press \"p\"");
        mvwprintw(footerWin, 0, 1, "%s", footLabel);
        mvwprintw(footerWin, 0, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);

        refresh();
        wrefresh(headerWin);
        wrefresh(userWin);
        wrefresh(footerWin);
        timeout(1000);
        switch(getch()) {
            case 'q':
            case 'Q':
            case 27: // ESC
                invoked_menu = 0;
                break;
        }
    }

    free_Array((void**)&(sysInfoBuf.mem.unit)); // Free Memory Unit Array

    clear();
    delwin(headerWin);
    delwin(userWin);
    delwin(footerWin);
    display_main();
}

/* List of functions relative to 2nd feature (Display Hardware Temperature) */
void two_Hardware_Temperature() { // Display Hardware Temperature
    invoked_menu = 1;

    TempLog temp1sec;
    int pos_x = 3, pos_y = 1, cpuAvgInterval = 900;
    char barBuf[BUF_MAX_LINE] = { '\0' };
    float cpu_avg_temp[2];

    WINDOW *headerWin = newwin(1, wbuf.ws_col, 0, 0);
    WINDOW *userWin = newwin(wbuf.ws_row - 1 - 1, wbuf.ws_col, 1, 0);
    WINDOW *footerWin = newwin(1, wbuf.ws_col, wbuf.ws_row - 1, 0);

    while(invoked_menu == 1) {
        pos_x = 3;
        pos_y = 1;

        get_Average_Temperature_from_Log(NULL, 1, TYPE_NOT_AVG, &temp1sec);

        wbkgd(headerWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wbkgd(footerWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        mvwprintw(headerWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
        mvwprintw(userWin, pos_y, pos_x - 1, "2. Hardware Temperature (Unit: Celcius)");
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 10, "- Server Hostname: %s", hostnameBuf);

        get_Percent_Bar(barBuf, 100 * (temp1sec.temp.inlet / (float)INLET_TEMP_CRITICAL_POINT), wbuf.ws_col / 2 - strlen("- Inlet Temperature: ") - 7 - pos_x - 3); // Content - Temp - head blank - [, ]
        mvwprintw(userWin, pos_y += 2, pos_x, "- Inlet Temperature: %s %4.1f C", barBuf, (float)temp1sec.temp.inlet);
        get_Percent_Bar(barBuf, 100 * (temp1sec.temp.exhaust / (float)EXHAUST_TEMP_CRITICAL_POINT), wbuf.ws_col / 2 - strlen("- Exhaust Temperature: ") - 7 - pos_x - 3);
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 10, "- Exhaust Temperature: %s %4.1f C", barBuf, (float)temp1sec.temp.exhaust);

        get_Percent_Bar(barBuf, 100 * (temp1sec.temp.raidCore/ (float)RAID_CORE_TEMP_CRITICAL_POINT), wbuf.ws_col / 2 - strlen("- HBA Core Temperature: ") - 7 - pos_x - 3);
        mvwprintw(userWin, pos_y += 1, pos_x, "- HBA Core Temperature: %s %4.1f C", barBuf, (float)temp1sec.temp.raidCore);
        get_Percent_Bar(barBuf, 100 * (temp1sec.temp.raidController / (float)RAID_CTRL_TEMP_CRITICAL_POINT), wbuf.ws_col / 2 - strlen("- HBA Controller Temperature: ") - 7 - pos_x - 3);
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 10, "- HBA Controller Temperature: %s %4.1f C", barBuf, (float)temp1sec.temp.raidController);
        get_Percent_Bar(barBuf, 100 * (temp1sec.temp.bbu / (float)BBU_TEMP_CRITICAL_POINT), wbuf.ws_col - strlen("- BBU (Battery Backup Unit) Temperature: ") - 7 - pos_x - 3);
        mvwprintw(userWin, pos_y += 1, pos_x, "- BBU (Battery Backup Unit) Temperature: %s %4.1f C", barBuf, (float)temp1sec.temp.bbu);

        wattron(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wmove(userWin, pos_y + 2, 0);
        whline(userWin, ' ', wbuf.ws_col - 1);
        mvwaddch(userWin, pos_y + 2, wbuf.ws_col - 1, ' ');
        mvwprintw(userWin, pos_y += 2, 3, "CPU Temperature  | Recommended: %d Celcius or less", CPU_TEMP_CRITICAL_POINT);
        wattroff(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));

        pos_y++;
        get_Average_Temperature_from_Log(cpu_avg_temp, cpuAvgInterval, TYPE_CPU_TEMP, NULL);
        for (int i = 0; i < MAX_CPU_COUNT; i++) {
            if (temp1sec.temp.cpu[i] == -100) {
                get_Percent_Bar(barBuf, 100 * (temp1sec.temp.cpu[i]/ (float)CPU_TEMP_CRITICAL_POINT), wbuf.ws_col - strlen("- CPU1 (AVG): ") - 7 - pos_x - 3 - strlen(" (Interval: 11111 Sec.)"));
                mvwprintw(userWin, pos_y += 1, pos_x, "- CPU%d %5s: %s N/A", i + 1, "", barBuf);
                get_Percent_Bar(barBuf, 100 * (cpu_avg_temp[i]/ (float)CPU_TEMP_CRITICAL_POINT), wbuf.ws_col - strlen("- CPU1 (AVG): ") - 7 - pos_x - 3 - strlen(" (Interval: 11111 Sec.)"));
                mvwprintw(userWin, pos_y += 1, pos_x, "- CPU%d %5s: %s N/A", i + 1, "(AVG)", barBuf);
            } else {
                get_Percent_Bar(barBuf, 100 * (temp1sec.temp.cpu[i]/ (float)CPU_TEMP_CRITICAL_POINT), wbuf.ws_col - strlen("- CPU1 (AVG): ") - 7 - pos_x - 3 - strlen(" (Interval: 11111 Sec.)"));
                mvwprintw(userWin, pos_y += 1, pos_x, "- CPU%d %5s: %s %4.1f C", i + 1, "", barBuf, (float)temp1sec.temp.cpu[i]);
                get_Percent_Bar(barBuf, 100 * (cpu_avg_temp[i]/ (float)CPU_TEMP_CRITICAL_POINT), wbuf.ws_col - strlen("- CPU1 (AVG): ") - 7 - pos_x - 3 - strlen(" (Interval: 11111 Sec.)"));
                mvwprintw(userWin, pos_y += 1, pos_x, "- CPU%d %5s: %s %4.1f C (Interval: %5d Sec.)", i + 1, "(AVG)", barBuf, cpu_avg_temp[i], cpuAvgInterval);
            }
        }

        wattron(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wmove(userWin, pos_y + 2, 0);
        whline(userWin, ' ', wbuf.ws_col - 1);
        mvwaddch(userWin, pos_y + 2, wbuf.ws_col - 1, ' ');
        mvwprintw(userWin, pos_y += 2, 3, "Storage Temperature | Recommended: %d Celcius or less", STORAGE_TEMP_CRITICAL_POINT);
        wattroff(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));

        for (int i = 0; i < temp1sec.temp.storage_cnt; i++) {
            get_Percent_Bar(barBuf, 100 * (temp1sec.temp.storage[i]/ (float)STORAGE_TEMP_CRITICAL_POINT), wbuf.ws_col - strlen("- Disk11 (Slot: 11): ") - 7 - pos_x - 10);
            if (temp1sec.temp.storage[i] == -100) {
                mvwprintw(userWin, pos_y += 1, pos_x, "- Disk%d (Slot - %d): %s N/A", i + 1, i + 1, barBuf);
            } else {
                mvwprintw(userWin, pos_y += 1, pos_x, "- Disk%d (Slot - %d): %s %5.1f C", i + 1, i + 1, barBuf, (float)temp1sec.temp.storage[i]);
            }
        }

        // mvwprintw(footerWin, 1, 1, "To see the list of partitions and its usage, Press \"p\"");
        mvwprintw(footerWin, 0, 1, "%s", footLabel);
        mvwprintw(footerWin, 0, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);

        refresh();
        wrefresh(headerWin);
        wrefresh(userWin);
        wrefresh(footerWin);
        timeout(1000);
        switch(getch()) {
            case 'q':
            case 'Q':
            case 27: // ESC
                invoked_menu = 0;
                break;
        }
    }

    clear();
    delwin(headerWin);
    delwin(userWin);
    delwin(footerWin);
    display_main();
}

/* List of functions relative to 3rd feature (Display CPU / Memory Usage)*/
void three_CPU_Memory_Usage() { // Display CPU / Memory Usage and Process Status
    invoked_menu = 1;

    UsageLog usage1sec;
    ProcessInfo* psList = NULL;
    UNIT memTotalUnit, memUseUnit, memUseAvgUnit, swapUseUnit, swapTotalUnit, psMemUnit = 0;
    int pos_x = 3, pos_y = 1, barLength = 35, cpu_interval = 10, mem_intverval = 10, psLineCnt = 0, tmp;
    int userNameWidth = 0, ttyWidth = 0, startWidth = 0, timeWidth = 0, pidWidth = 8, cpuWidth = 5, memPercentWidth = 5, memSizeWidth = 8;
    char barBuf[BUF_MAX_LINE] = { '\0' };
    float cpu_avg, mem_avg[2], memUseConverted, memTotalConverted, swapUseConverted, swapTotalConverted, memUseAvgConverted, psMemUseConverted;

    WINDOW *headerWin = newwin(1, wbuf.ws_col, 0, 0);
    WINDOW *userWin = newwin(wbuf.ws_row - 1 - 1, wbuf.ws_col, 1, 0);
    WINDOW *footerWin = newwin(1, wbuf.ws_col, wbuf.ws_row - 1, 0);
    
    while(invoked_menu == 1) {
        pos_x = 3;
        pos_y = 1;

        get_Average_Usage_Percent_from_Log(NULL, 1, TYPE_NOT_AVG, &usage1sec);

        wbkgd(headerWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wbkgd(footerWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        mvwprintw(headerWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
        mvwprintw(userWin, pos_y, pos_x - 1, "3. CPU / Memory Usage");
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 13, "- Server Hostname: %s", hostnameBuf);

        get_Average_Usage_Percent_from_Log(&cpu_avg, cpu_interval, TYPE_CPU_USAGE, NULL);
        get_Average_Usage_Percent_from_Log(mem_avg, mem_intverval, TYPE_MEM_USAGE, NULL);

        memUseConverted = convert_Size_Unit(usage1sec.mem.memUse, 1, &memUseUnit);
        memTotalConverted = convert_Size_Unit(usage1sec.mem.memTotal, 1, &memTotalUnit);
        swapUseConverted = convert_Size_Unit(usage1sec.mem.swapUse, 1, &swapUseUnit);
        swapTotalConverted = convert_Size_Unit(usage1sec.mem.swapTotal, 1, &swapTotalUnit);

        memUseAvgConverted = convert_Size_Unit((mem_avg[0] / 100) * usage1sec.mem.memTotal, 1, &memUseAvgUnit);

        get_Percent_Bar(barBuf, usage1sec.cpu.usage, barLength);
        mvwprintw(userWin, pos_y += 2, pos_x, "%-20s %s %5.1f%%", "- CPU Usage", barBuf, usage1sec.cpu.usage);
        
        get_Percent_Bar(barBuf, cpu_avg, barLength);
        mvwprintw(userWin, pos_y += 1, pos_x, "%-20s %s %5.1f%% (Inteval: %5d sec.)", "- CPU Usage (AVG)", barBuf, cpu_avg, cpu_interval);
        
        get_Percent_Bar(barBuf, get_Capacity_Percent(usage1sec.mem.memTotal, usage1sec.mem.memUse), barLength);
        mvwprintw(userWin, pos_y += 2, pos_x, "%-20s %s %5.1f%% (%.2f%s / %.2f%s)", 
            "- Memory Usage", barBuf, get_Capacity_Percent(usage1sec.mem.memTotal, usage1sec.mem.memUse), memUseConverted, unitMap[memUseUnit].str, memTotalConverted, unitMap[memTotalUnit].str);
        
        get_Percent_Bar(barBuf, mem_avg[0], barLength);
        mvwprintw(userWin, pos_y += 1, pos_x, "%-20s %s %5.1f%% (%.2f%s / %.2f%s, Inteval: %5d sec.)",
            "- Memory Usage (AVG)", barBuf, mem_avg[0], memUseAvgConverted, unitMap[memUseAvgUnit].str, memTotalConverted, unitMap[memTotalUnit].str, mem_intverval);
        
        get_Percent_Bar(barBuf, get_Capacity_Percent(usage1sec.mem.swapTotal, usage1sec.mem.swapUse), barLength);
        mvwprintw(userWin, pos_y += 1, pos_x, "%-20s %s %5.1f%% (%.2f%s / %.2f%s)", 
            "- SWAP Usage", barBuf, get_Capacity_Percent(usage1sec.mem.swapTotal, usage1sec.mem.swapUse), swapUseConverted, unitMap[swapUseUnit].str, swapTotalConverted, unitMap[swapTotalUnit].str);

        wattron(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wmove(userWin, pos_y += 2, 0);
        whline(userWin, ' ', wbuf.ws_col - 1);
        mvwaddch(userWin, pos_y, wbuf.ws_col - 1, ' ');
        wmove(userWin, pos_y + 1, 0);
        whline(userWin, ' ', wbuf.ws_col - 1);
        mvwaddch(userWin, pos_y + 1, wbuf.ws_col - 1, ' ');
        pos_y++;
        psLineCnt = wbuf.ws_row - pos_y - 1 - 1 - 1;
        
        get_Process_Status(&psList, psLineCnt);
        userNameWidth = get_Maximum_Length_of_ProcessInfo(psList, psLineCnt, TYPE_PROCESS_USERNAME) + 1;
        ttyWidth = get_Maximum_Length_of_ProcessInfo(psList, psLineCnt, TYPE_PROCESS_TTY) + 1;
        startWidth = get_Maximum_Length_of_ProcessInfo(psList, psLineCnt, TYPE_PROCESS_START) + 1;
        timeWidth = get_Maximum_Length_of_ProcessInfo(psList, psLineCnt, TYPE_PROCESS_TIME) + 1;

        mvwprintw(userWin, pos_y - 1, 1, "Running Process Status | Top %d Process with High Memory Usage | <BG> = Backgruond Process", psLineCnt);
        mvwprintw(userWin, pos_y, pos_x += 1, TYPE_PROCESS_USERNAME_STR);
        mvwprintw(userWin, pos_y, pos_x += userNameWidth, "PID");
        mvwprintw(userWin, pos_y, pos_x += pidWidth, "CPU%%");
        mvwprintw(userWin, pos_y, pos_x += cpuWidth, "MEM%%");
        mvwprintw(userWin, pos_y, pos_x += memPercentWidth, "MEMSize");
        mvwprintw(userWin, pos_y, pos_x += memSizeWidth, TYPE_PROCESS_TTY_STR);
        mvwprintw(userWin, pos_y, pos_x += ttyWidth, TYPE_PROCESS_START_STR);
        mvwprintw(userWin, pos_y, pos_x += startWidth, TYPE_PROCESS_TIME_STR);
        mvwprintw(userWin, pos_y, pos_x += timeWidth, "Command");
        wattroff(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        pos_y++;

        for (int i = 0; i < psLineCnt; i++) {
            pos_x = 3;
            for (tmp = psLineCnt; tmp /= 10; pos_x++);
            psMemUseConverted = convert_Size_Unit(psList[i].memUseSize, 1, &psMemUnit);
            wmove(userWin, pos_y + i, 0);
            whline(userWin, ' ', wbuf.ws_col - 1);
            mvwprintw(userWin, pos_y + i, 1, "%d", i + 1);
            mvwprintw(userWin, pos_y + i, pos_x, "%s", psList[i].userName);
            mvwprintw(userWin, pos_y + i, pos_x += userNameWidth, "%d", psList[i].pid);
            if (psList[i].cpu == 100) {
                mvwprintw(userWin, pos_y + i, pos_x += pidWidth, "%d", (int)psList[i].cpu);
            } else {
                mvwprintw(userWin, pos_y + i, pos_x += pidWidth, "%-.1f", psList[i].cpu);
            }
            if (psList[i].mem == 100) {
                mvwprintw(userWin, pos_y + i, pos_x += cpuWidth, "%d", (int)psList[i].mem);
            } else {
                mvwprintw(userWin, pos_y + i, pos_x += cpuWidth, "%-.1f", psList[i].mem);
            }
            mvwprintw(userWin, pos_y + i, pos_x += memPercentWidth, "%.1f%s", psMemUseConverted, unitMap[psMemUnit].str);
            mvwprintw(userWin, pos_y + i, pos_x += memSizeWidth, "%s", psList[i].tty);
            mvwprintw(userWin, pos_y + i, pos_x += ttyWidth, "%s", psList[i].start);
            mvwprintw(userWin, pos_y + i, pos_x += startWidth, "%s", psList[i].time);
            mvwprintw(userWin, pos_y + i, pos_x += timeWidth, "%s", psList[i].command);
        }

        mvwprintw(footerWin, 0, 1, "%s", footLabel);
        mvwprintw(footerWin, 0, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);

        refresh();
        wrefresh(headerWin);
        wrefresh(userWin);
        wrefresh(footerWin);
        timeout(1000);
        switch(getch()) {
            case 'q':
            case 'Q':
            case 27: // ESC
                invoked_menu = 0;
                break;
        }
    }

    for (int i = 0; i < psLineCnt; i++) {
        free_Array((void**)&(psList[i].command));
    }
    free_Array((void**)&psList);

    clear();
    delwin(headerWin);
    delwin(userWin);
    delwin(footerWin);
    display_main();
}

int get_Maximum_Length_of_ProcessInfo(ProcessInfo* psList, int psCnt, int type) { // Get the maximum length of string
    int len = 0;
    if (type == TYPE_PROCESS_START) {
        len = strlen(TYPE_PROCESS_START_STR);
    } else if (type == TYPE_PROCESS_TIME) {
        len = strlen(TYPE_PROCESS_TIME_STR);
    } else if (type == TYPE_PROCESS_USERNAME) {
        len = strlen(TYPE_PROCESS_USERNAME_STR);
    } else if (type == TYPE_PROCESS_TTY) {
        len = strlen(TYPE_PROCESS_TTY_STR);
    }

    for (int i = 0; i < psCnt; i++) { 
        if (type == TYPE_PROCESS_START) {
            len = (((int)strlen(psList[i].start) > len) ? (int)strlen(psList[i].start) : len);
        } else if (type == TYPE_PROCESS_TIME) {
            len = (((int)strlen(psList[i].time) > len) ? (int)strlen(psList[i].time) : len);
        } else if (type == TYPE_PROCESS_USERNAME) {
            len = (((int)strlen(psList[i].userName) > len) ? (int)strlen(psList[i].userName) : len);
        } else if (type == TYPE_PROCESS_TTY) {
            len = (((int)strlen(psList[i].tty) > len) ? (int)strlen(psList[i].tty) : len);
        }
    }
    return len;
}

/* List of functions relative to 4th features (Display Disk Information)*/
void four_Disk_Information() { // Display Disk Information - HBA Card, Physical Disk, and Partition Information
    invoked_menu = 1;

    HBAInfo HBABuf;
    DiskInfo* diskList = NULL;
    BBUInfo BBUBuf;
    int pos_x = 3, pos_y = 1, diskCount = 0, line = 0, printListCnt = 0, tmp;
    int modelNameWidth = 0, mediatypeWidth = 0, interfaceWidth = 0, statusWidth = 0;
    // int capacityWidth = 0
    int enSltWidth = strlen(DISK_EN_SLT_TITLE) + 2, dgWidth = strlen(DISK_DG_TITLE) + 3, devIdWidth = strlen(DISK_DEV_ID_TITLE);
    char BBUStatusBuf[BUF_MAX_LINE] = { '\0' };
    WINDOW *waitWin = newwin(wbuf.ws_row, wbuf.ws_col, 0, 0); // Screen for Waiting OMSA
    wattron(waitWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wmove(waitWin, 0, 0);
    whline(waitWin, ' ', wbuf.ws_col - 1);
    mvwaddch(waitWin, 0, wbuf.ws_col - 1, ' ');
    mvwprintw(waitWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
    wattroff(waitWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    mvwprintw(waitWin, pos_y + 1, pos_x, "!! Information !!");
    mvwprintw(waitWin, pos_y + 3, pos_x + 2, "- Collecting data from PERCCLI Package...");
    mvwprintw(waitWin, pos_y + 4, pos_x + 2, "- Please wait a moment. (Up to 60 Seconds)");
    mvwprintw(waitWin, pos_y + 5, pos_x + 2, "- If this program stops on this screen, Press Ctrl+C or Ctrl+\\ to exit.");
    refresh();
    wrefresh(waitWin);

    get_HBA_Information_from_Perccli(&HBABuf);
    get_Disk_Information_from_Perccli(&diskList, &diskCount);
    get_BBU_Information_from_Perccli(&BBUBuf);
    statusWidth = get_Maximum_Length_of_DiskInfo(diskList,diskCount, TYPE_DISK_STATUS) + 4;
    modelNameWidth = get_Maximum_Length_of_DiskInfo(diskList, diskCount, TYPE_DISK_NAME) + 4;
    mediatypeWidth = get_Maximum_Length_of_DiskInfo(diskList, diskCount, TYPE_DISK_MEDIATYPE) + 4;
    interfaceWidth = get_Maximum_Length_of_DiskInfo(diskList, diskCount, TYPE_DISK_INTERFACE) + 4;
    // capacityWidth = get_Maximum_Length_of_DiskInfo(diskList, diskCount, TYPE_DISK_CAPACITY) + 4;

    delwin(waitWin);
    WINDOW *headerWin = newwin(1, wbuf.ws_col, 0, 0);
    WINDOW *userWin = newwin(wbuf.ws_row - 1 - 3, wbuf.ws_col, 1, 0);
    WINDOW *footerWin = newwin(3, wbuf.ws_col, wbuf.ws_row - 3, 0);

    pos_y = 12;
    printListCnt = wbuf.ws_row - pos_y - 1 - 5; // Total - Top Info - Header - Footer
    while(invoked_menu == 1) {
        pos_x = 3;
        pos_y = 1;
        wbkgd(headerWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wbkgd(footerWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        mvwprintw(headerWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
        mvwprintw(userWin, pos_y, pos_x - 1, "4. Disk Information - HBA Card / Physical Storage Information");
        mvwprintw(userWin, pos_y += 2, pos_x, "- Server Hostname: %s", hostnameBuf);
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET, "- HBA Card ModelName: %s", HBABuf.HBA_Name);
        mvwprintw(userWin, pos_y += 1, pos_x, "- HBA Firmware Version: %s", HBABuf.HBA_Firmware_Ver);
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET, "- HBA BIOS Version: %s", HBABuf.HBA_Bios_Ver);
        mvwprintw(userWin, pos_y += 1, pos_x, "- HBA Driver Name: %s", HBABuf.HBA_Driver_Name);
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET, "- HBA Driver Version: %s", HBABuf.HBA_Driver_Ver);
        if (HBABuf.status == TYPE_STATUS_OPTIMAL) {
            mvwprintw(userWin, pos_y += 1, pos_x, "- HBA Status: %s", HBA_CONTROLLER_STATUS_OPTIMAL);
        } else if (HBABuf.status == TYPE_STATUS_FAILED) {
            mvwprintw(userWin, pos_y += 1, pos_x, "- HBA Status: %s", HBA_CONTROLLER_STATUS_FAILED);
        } else if (HBABuf.status == TYPE_STATUS_DEGRADED) {
            mvwprintw(userWin, pos_y += 1, pos_x, "- HBA Status: %s", HBA_CONTROLLER_STATUS_DEGRADED);
        } else {
            mvwprintw(userWin, pos_y += 1, pos_x, "- HBA Status: %s", "N/A");
        }
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET, "- HBA Serial Number: %s", HBABuf.HBA_Serial_Num);

        if (BBUBuf.status == TYPE_STATUS_OPTIMAL) {
            strcpy(BBUStatusBuf, "OK (");
            strcat(BBUStatusBuf, BBU_STATUS_OPTIMAL);
            strcat(BBUStatusBuf, ")");
        } else if (BBUBuf.status == TYPE_STATUS_DEGRADED) {
            strcpy(BBUStatusBuf, BBU_STATUS_DEGRADED);
            strcat(BBUStatusBuf, "(Need to Check / Replace)");
        } else if (BBUBuf.status == TYPE_STATUS_FAILED) {
            strcpy(BBUStatusBuf, BBU_STATUS_FAILED);
            strcat(BBUStatusBuf, "(Need to Check / Replace)");
        }

        mvwprintw(userWin, pos_y += 2, pos_x, "- BBU Status: %s | Voltage: %s | Capacity: %s/%s/%s (Remain/Full/Design)", BBUStatusBuf, BBUBuf.voltage, BBUBuf.remainCapacity, BBUBuf.fullCapacity, BBUBuf.designCapacity);

        wattron(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wmove(userWin, pos_y + 2, 0);
        whline(userWin, ' ', wbuf.ws_col - 1);
        mvwaddch(userWin, pos_y + 2, wbuf.ws_col - 1, ' ');
        wmove(userWin, pos_y + 3, 0);
        whline(userWin, ' ', wbuf.ws_col - 1);
        mvwaddch(userWin, pos_y + 3, wbuf.ws_col - 1, ' ');
        pos_x = 3 + 3;
        for (tmp = diskCount; tmp /= 10; pos_x++);
        mvwprintw(userWin, pos_y += 2, 3, "[List of Storage]");
        mvwprintw(userWin, pos_y += 1, pos_x, DISK_EN_SLT_TITLE); // Enclosure:SlotNum
        mvwprintw(userWin, pos_y, pos_x += enSltWidth, DISK_DG_TITLE); // Drive Group
        mvwprintw(userWin, pos_y, pos_x += dgWidth, DISK_STATUS_TITLE); // Status
        mvwprintw(userWin, pos_y, (pos_x += statusWidth) - 2, DISK_DEV_ID_TITLE); // Device ID
        mvwprintw(userWin, pos_y, pos_x += devIdWidth, DISK_MODELNAME_TITIE); // Model Name
        mvwprintw(userWin, pos_y, pos_x += modelNameWidth, DISK_MEDIATYPE_TITLE); // Media Type (SSD, HDD, ...)
        mvwprintw(userWin, pos_y, pos_x += mediatypeWidth, DISK_INTERFACE_TITLE); // Interface Type (SATA, SAS, ...)
        mvwprintw(userWin, pos_y++, pos_x += interfaceWidth, DISK_CAPACITY_TITLE); // Capacity
        // mvwprintw(userWin, pos_y++, pos_x += capacityWidth, DISK_MAPPED_PARTITION_TITLE); // Mapped Parititon
        wattroff(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        for (int i = 0; i < printListCnt; i++) {
            wmove(userWin, pos_y + i, 0);
            whline(userWin, ' ', wbuf.ws_col - 1);
            mvwaddch(userWin, pos_y + i, wbuf.ws_col - 1, ' ');
        }

        for (int i = 0; i < printListCnt; i++) {
            if (i >= diskCount) {
                break;
            }
            pos_x = 3 + 3;
            for (tmp = diskCount; tmp /= 10; pos_x++);
            mvwprintw(userWin, pos_y + i, 3, "%d", i + 1 + line); // Index
            mvwprintw(userWin, pos_y + i, pos_x, "%d:%d", diskList[i + line].enclosureNum, diskList[i + line].slotNum); // Enclosure : SlotNum
            if (diskList[i].driveGroup != -100) {
                mvwprintw(userWin, pos_y + i, pos_x += enSltWidth, "%d", diskList[i].driveGroup); // Drive Group
            } else {
                mvwprintw(userWin, pos_y + i, pos_x += enSltWidth, NA_STR); // Drive Group (N/A)
            }
            if (diskList[i + line].status == TYPE_DISK_STATE_ONLINE) { // Status 
                mvwprintw(userWin, pos_y + i, pos_x += dgWidth, DISK_STATE_ONLINE_FULLSTR);
            } else if (diskList[i + line].status == TYPE_DISK_STATE_OFFLINE) {
                mvwprintw(userWin, pos_y + i, pos_x += dgWidth, DISK_STATE_OFFLINE_FULLSTR);
            } else if (diskList[i + line].status == TYPE_DISK_STATE_GHS) {
                mvwprintw(userWin, pos_y + i, pos_x += dgWidth, DISK_STATE_GHS_FULLSTR);
            } else if (diskList[i + line].status == TYPE_DISK_STATE_DHS) {
                mvwprintw(userWin, pos_y + i, pos_x += dgWidth, DISK_STATE_DHS_FULLSTR);
            } else if (diskList[i + line].status == TYPE_DISK_STATE_UGOOD) {
                mvwprintw(userWin, pos_y + i, pos_x += dgWidth, DISK_STATE_UGOOD_FULLSTR);
            } else if (diskList[i + line].status == TYPE_DISK_STATE_UBAD) {
                mvwprintw(userWin, pos_y + i, pos_x += dgWidth, DISK_STATE_UBAD_FULLSTR);
            } else if (diskList[i + line].status == TYPE_DISK_STATE_SANI) {
                mvwprintw(userWin, pos_y + i, pos_x += dgWidth, DISK_STATE_SANI_FULLSTR);
            } else {
                mvwprintw(userWin, pos_y + i, pos_x += dgWidth, "N/A");
            }

            mvwprintw(userWin, pos_y + i, pos_x += statusWidth, "%d", diskList[i + line].deviceID); // Device ID
            mvwprintw(userWin, pos_y + i, pos_x += devIdWidth, "%s", diskList[i + line].modelName); // Model Name
            mvwprintw(userWin, pos_y + i, pos_x += modelNameWidth, "%s", diskList[i + line].mediaType); // Media Type
            mvwprintw(userWin, pos_y + i, pos_x += mediatypeWidth, "%s", diskList[i + line].interface); // Interface
            mvwprintw(userWin, pos_y + i, pos_x += interfaceWidth, "%s", diskList[i + line].capacity); // Capacity
            // mvwprintw(userWin, pos_y + i, pos_x += capacityWidth, "%s", diskList[i + line].mappedPartition); // Mapped Partition
        }
        wattron(footerWin, A_REVERSE);
        wmove(footerWin, 0, 0);
        whline(footerWin, ' ', wbuf.ws_col);
        mvwprintw(footerWin, 0, 1, "EN: Enclosure | SLT: SlotNum | DG: DriveGroup | DevID: DeviceID | MT: MediaType | IF: InterFace");
        wattroff(footerWin, A_REVERSE);
        // mvwprintw(footerWin, 1, 1, "To see the status of Virtual Drive, Press \"v\"");
        // mvwprintw(footerWin, 1, 1, "To see the list of partitions and its usage & I/O Speed, Press \"p\"");
        mvwprintw(footerWin, 1, 1, "To see the list of partitions and its usage, Press \"p\"");
        mvwprintw(footerWin, 2, 1, "%s", footLabel);
        mvwprintw(footerWin, 2, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);

        refresh();
        wrefresh(headerWin);
        wrefresh(userWin);
        wrefresh(footerWin);
        timeout(1000);
        switch(getch()) {
            case 'n':
            case 'N':
                line -= ((line > 0) ? 1 : 0);
                break;
            case 'm':
            case 'M':
                line += ((line < diskCount - printListCnt) ? 1 : 0);
                break;
            case 'p':
            case 'P':
                display_Partition_Information();
                touchwin(headerWin);
                touchwin(userWin);
                touchwin(footerWin);
                break;
            // case 'v':
            // case 'V':
            //     touchwin(headerWin);
            //     touchwin(userWin);
            //     touchwin(footerWin);
            //     break;
            case 'q':
            case 'Q':
            case 27: // ESC
                invoked_menu = 0;
                break;
        }
    }

    free_Array((void**)&diskList); // Free UserList Array

    clear();
    delwin(headerWin);
    delwin(userWin);
    delwin(footerWin);
    display_main();
}

int get_Maximum_Length_of_DiskInfo(DiskInfo* diskList, int diskCnt, int type) { // Get the maximum length of string
    int len = 0, tmp = 0;
    if (type == TYPE_DISK_NAME) {
        len = strlen(DISK_MODELNAME_TITIE);
    } else if (type == TYPE_DISK_CAPACITY) {
        len = strlen(DISK_CAPACITY_TITLE);
    } else if (type == TYPE_DISK_MEDIATYPE) {
        len = strlen(DISK_MEDIATYPE_TITLE);
    } else if (type == TYPE_DISK_INTERFACE) {
        len = strlen(DISK_INTERFACE_TITLE);
    } else if (type == TYPE_DISK_STATUS) {
        len = strlen(DISK_STATUS_TITLE);
    }

    for (int i = 0; i < diskCnt; i++) {
        if (type == TYPE_DISK_NAME) {
            len = (((int)strlen(diskList[i].modelName) > len) ? (int)strlen(diskList[i].modelName) : len);
        } else if (type == TYPE_DISK_CAPACITY) {
            len = (((int)strlen(diskList[i].capacity) > len) ? (int)strlen(diskList[i].capacity) : len);
        } else if (type == TYPE_DISK_MEDIATYPE) {
            len = (((int)strlen(diskList[i].mediaType) > len) ? (int)strlen(diskList[i].mediaType) : len);            
        } else if (type == TYPE_DISK_INTERFACE) {
            len = (((int)strlen(diskList[i].interface) > len) ? (int)strlen(diskList[i].interface) : len);
        } else if (type == TYPE_DISK_STATUS) {
            if (diskList[i].status == TYPE_DISK_STATE_ONLINE) { // Status 
                tmp = strlen(DISK_STATE_ONLINE_FULLSTR);
            } else if (diskList[i].status == TYPE_DISK_STATE_OFFLINE) {
                tmp = strlen(DISK_STATE_OFFLINE_FULLSTR);
            } else if (diskList[i].status == TYPE_DISK_STATE_GHS) {
                tmp = strlen(DISK_STATE_GHS_FULLSTR);
            } else if (diskList[i].status == TYPE_DISK_STATE_DHS) {
                tmp = strlen(DISK_STATE_DHS_FULLSTR);
            } else if (diskList[i].status == TYPE_DISK_STATE_UGOOD) {
                tmp = strlen(DISK_STATE_UGOOD_FULLSTR);
            } else if (diskList[i].status == TYPE_DISK_STATE_UBAD) {
                tmp = strlen(DISK_STATE_UBAD_FULLSTR);
            } else if (diskList[i].status == TYPE_DISK_STATE_SANI) {
                tmp = strlen(DISK_STATE_SANI_FULLSTR);
            } else {
                tmp = strlen("N/A");
            }

            len = ((tmp > len) ? tmp : len);
        }
    }

    return len;
}

void display_Partition_Information() { // Get Partition Information - FileSystem, mountPath, Space
    invoked_SubScreen = 1;

    PartitionInfo* partList = NULL;
    UNIT useUnit = 0, totalUnit = 0;
    // UNIT readUnit, writeUnit;
    int pos_x = 3, pos_y = 1, partCnt = 0, fileSysWidth = 0, mountPathWidth = 0, tmp, printListCnt = 0, line = 0, spaceWidth = 35;
    // int speedWidth = 10;
    float useSpace = 0, totalSpace = 0;
    // float readSpeed = 0, writeSpeed = 0;
    char barBuf[BUF_MAX_LINE] = { '\0' };

    get_Partition_Information(&partList, &partCnt);
    fileSysWidth = get_Maximum_Length_of_PartitionInfo(partList, partCnt, TYPE_PARTITION_FILESYSTEM) + 2;
    mountPathWidth = get_Maximum_Length_of_PartitionInfo(partList, partCnt, TYPE_PARTITION_MOUNTPATH) + 2;

    WINDOW *headerWin = newwin(1, wbuf.ws_col, 0, 0);
    WINDOW *userWin = newwin(wbuf.ws_row - 1 - 2, wbuf.ws_col, 1, 0);
    WINDOW *footerWin = newwin(2, wbuf.ws_col, wbuf.ws_row - 2, 0);
    wbkgd(headerWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wbkgd(footerWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    mvwprintw(headerWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
    mvwprintw(userWin, pos_y, pos_x - 1, "4. Network Informtion - List of Partition, and its Usage and I/O Speed");
    mvwprintw(userWin, pos_y += 2, pos_x, "- Server Hostname: %s", hostnameBuf);
    mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 4, "!! Notification !!");
    mvwprintw(userWin, pos_y += 2, pos_x, "- # of Partition: %d", partCnt);
    mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 4, " - \"N/A\" means that no records exist for that partition.");

    wattron(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wmove(userWin, pos_y + 2, 0);
    whline(userWin, ' ', wbuf.ws_col - 1);
    mvwaddch(userWin, pos_y + 2, wbuf.ws_col - 1, ' ');
    wmove(userWin, pos_y + 3, 0);
    whline(userWin, ' ', wbuf.ws_col - 1);
    mvwaddch(userWin, pos_y + 3, wbuf.ws_col - 1, ' ');
    pos_x = 1 + 2;
    for (tmp = partCnt; tmp /= 10; pos_x++);
    mvwprintw(userWin, pos_y += 2, 1, "[List of Prtitions]");
    mvwprintw(userWin, pos_y += 1, pos_x, TYPE_PARTITION_FILESYSTEM_STR);
    mvwprintw(userWin, pos_y, pos_x += fileSysWidth, TYPE_PARTITION_MOUNTPATH_STR);
    mvwprintw(userWin, pos_y, pos_x += mountPathWidth, "Usage");
    mvwprintw(userWin, pos_y, (pos_x += spaceWidth) + 10, "(Used / Total)");
    // mvwprintw(userWin, pos_y, pos_x += spaceWidth, "Read");
    // mvwprintw(userWin, pos_y, pos_x += speedWidth, "Write");
    wattroff(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    pos_y++;
    printListCnt = wbuf.ws_row - pos_y - 1 - 2; // Total - Top Info - Header - Footer

    while(invoked_SubScreen == 1) {
        get_Partition_Information(&partList, &partCnt);
        for (int i = 0; i < printListCnt; i++) {
            wmove(userWin, pos_y + i, 0);
            whline(userWin, ' ', wbuf.ws_col - 1);
            mvwaddch(userWin, pos_y + i, wbuf.ws_col - 1, ' ');
        }
        for (int i = 0; i < printListCnt; i++) {
            if (i >= partCnt) {
                break;
            }
            pos_x = 1 + 2;
            for (tmp = partCnt; tmp /= 10; pos_x++);
            // get_Partition_IO_Speed(partCnt, i, partList[i].fileSystem, &readSpeed, &writeSpeed);
            mvwprintw(userWin, pos_y + i, 1, "%d", i + line + 1);
            mvwprintw(userWin, pos_y + i, pos_x, "%s", partList[i].fileSystem);
            mvwprintw(userWin, pos_y + i, pos_x += fileSysWidth, "%s", partList[i].mountPath);
            useSpace = convert_Size_Unit(partList[i].spaceUse, 1, &useUnit);
            totalSpace = convert_Size_Unit(partList[i].spaceTotal, 1, &totalUnit);
            get_Percent_Bar(barBuf, get_Capacity_Percent(partList[i].spaceTotal, partList[i].spaceUse), spaceWidth);
            mvwprintw(userWin, pos_y + i, pos_x += mountPathWidth, "%s %5.1f%% (%.2f%s / %.2f%s)", barBuf, get_Capacity_Percent(partList[i].spaceTotal, partList[i].spaceUse), useSpace, unitMap[useUnit].str, totalSpace, unitMap[totalUnit].str);
            

            // readSpeed = convert_Size_Unit((double)readSpeed, 1, &readUnit);
            // writeSpeed = convert_Size_Unit((double)writeSpeed, 1, &writeUnit);
            // if (readSpeed < 0) {
            //     mvwprintw(userWin, pos_y + i, pos_x += spaceWidth, "N/A");
            // } else {
            //     mvwprintw(userWin, pos_y + i, pos_x += spaceWidth, "%.1f%s/s", readSpeed, unitMap[readUnit].str);
            // }

            // if (writeSpeed < 0) {
            //     mvwprintw(userWin, pos_y + i, pos_x += speedWidth, "N/A");
            // } else {
            //     mvwprintw(userWin, pos_y + i, pos_x += speedWidth, "%.1f%s/s", writeSpeed, unitMap[writeUnit].str);
            // }

        }
        mvwprintw(footerWin, 0, 1, "To see the remaining, Press \"m\" (Next) and \"n\" (Previous).");
        mvwprintw(footerWin, 1, 1, "%s", footLabel);
        mvwprintw(footerWin, 1, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);

        refresh();
        wrefresh(headerWin);
        wrefresh(userWin);
        wrefresh(footerWin);
        timeout(1000);
        switch(getch()) {
            case 'n':
            case 'N':
                line -= ((line > 0) ? 1 : 0);
                break;
            case 'm':
            case 'M':
                line += ((line < partCnt - printListCnt) ? 1 : 0);
                break;
            case 'q':
            case 'Q':
            case 27: // ESC
                invoked_SubScreen = 0;
                break;
        }
    }

    free_Array((void**)&partList);

    clear();
    delwin(headerWin);
    delwin(userWin);
    delwin(footerWin);
    return;
}

int get_Maximum_Length_of_PartitionInfo(PartitionInfo* partList, int partCnt, int type) { // Get length of String
    int len = 0;
    if (type == TYPE_PARTITION_FILESYSTEM) {
        len = strlen(TYPE_PARTITION_FILESYSTEM_STR);
    } else if (type == TYPE_PARTITION_MOUNTPATH) {
        len = strlen(TYPE_PARTITION_MOUNTPATH_STR);
    }

    for (int i = 0; i < partCnt; i++) {
        if (type == TYPE_PARTITION_FILESYSTEM) {
            len = (((int)strlen(partList[i].fileSystem) > len) ? (int)strlen(partList[i].fileSystem) : len);
        } else if (type == TYPE_PARTITION_MOUNTPATH) {
            len = (((int)strlen(partList[i].mountPath) > len) ? (int)strlen(partList[i].mountPath) : len);
        }
    }
    return len;
}

void get_Percent_Bar(char* barBuf, float percent, int length) { // Get Percent Bar. (Ex. [########              ])
    int percent_int = percent + 0.5;

    strcpy(barBuf, "[");
    for (int i = 0; i < length; i++) {
        if (i < (int)(percent_int * ((float)length / 100))) {
            strcat(barBuf, "#");
        } else {
            strcat(barBuf, " ");
        }
    }
    strcat(barBuf, "]");
}

/* List of functions relative to 5th features (Display Network (IFA) Information) */
void five_Network_Information() { // Display Network Information: Physical IFA and Logical IFA (in Linux)
    invoked_menu = 1;

    PHYSICAL_IFA_Info* ifaList = NULL;
    DockerInfo* containerList = NULL;
    IFASpeed* ifaSpeedList = NULL;
    int pos_x = 3, pos_y = 1, ifaCount = 0, ifaSpeedCnt = 0, containerCnt = 0, line = 0, logicalWidth = 0, physicalWidth = 0, statusWidth = 0, tmp, printListCnt = 0;

    WINDOW *waitWin = newwin(wbuf.ws_row, wbuf.ws_col, 0, 0); // Screen for Waiting OMSA
    wattron(waitWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wmove(waitWin, 0, 0);
    whline(waitWin, ' ', wbuf.ws_col - 1);
    mvwaddch(waitWin, 0, wbuf.ws_col - 1, ' ');
    mvwprintw(waitWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
    wattroff(waitWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    mvwprintw(waitWin, pos_y + 1, pos_x, "!! Information !!");
    mvwprintw(waitWin, pos_y + 3, pos_x + 2, "- Collecting data from OMSA Package...");
    mvwprintw(waitWin, pos_y + 4, pos_x + 2, "- Please wait a moment. (Up to 60 Seconds.)");
    mvwprintw(waitWin, pos_y + 5, pos_x + 2, "- If this program stops on this screen, Press Ctrl+C or Ctrl+\\ to exit.");
    refresh();
    wrefresh(waitWin);

    get_Physical_IFA_Information_from_Omreport(&ifaList, &ifaCount);
    get_IFA_Speed(&containerList, &ifaSpeedList, &ifaSpeedCnt, &containerCnt, 0);
    logicalWidth = get_Maximum_Length_of_Physical_IFAInfo(ifaList, ifaCount, TYPE_NICS_LOGICAL_NAME) + 4;
    physicalWidth = get_Maximum_Length_of_Physical_IFAInfo(ifaList, ifaCount, TYPE_NICS_PHYSICAL_NAME) + 4;
    statusWidth = get_Maximum_Length_of_Physical_IFAInfo(ifaList, ifaCount, TYPE_NICS_STATUS) + 4;

    delwin(waitWin);
    WINDOW *headLineWin = newwin(1, wbuf.ws_col, 0, 0);
    WINDOW *userWin = newwin(wbuf.ws_row - 1 - 2, wbuf.ws_col, 1, 0);
    WINDOW *footLineWin = newwin(2, wbuf.ws_col, wbuf.ws_row - 2, 0);

    pos_y = 12;
    printListCnt = wbuf.ws_row - pos_y - 1 - 2; // Total - Top Info - Header - Footer
    while(invoked_menu == 1) {
        pos_x = 3;
        pos_y = 1;
        wbkgd(headLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wbkgd(footLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        mvwprintw(headLineWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
        mvwprintw(userWin, pos_y, pos_x - 1, "5. Network Informtion - Physical / Logical Network IFA(InterFAce)");
        mvwprintw(userWin, pos_y += 2, pos_x, "- Server Hostname: %s", hostnameBuf);
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET, "- # of Physical Network IFA: %d", ifaCount);
        mvwprintw(userWin, pos_y += 2, pos_x, "!! Notification !!");
        mvwprintw(userWin, pos_y += 1, pos_x, " - \"N/A\" means that no records exist for that ifa.");
        mvwprintw(userWin, pos_y += 1, pos_x, " - Virtual (Teaming / Bonding / Docker) network interface is not included.");

        wattron(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wmove(userWin, pos_y + 2, 0);
        whline(userWin, ' ', wbuf.ws_col - 1);
        mvwaddch(userWin, pos_y + 2, wbuf.ws_col - 1, ' ');
        wmove(userWin, pos_y + 3, 0);
        whline(userWin, ' ', wbuf.ws_col - 1);
        mvwaddch(userWin, pos_y + 3, wbuf.ws_col - 1, ' ');
        pos_x = 3 + 3;
        for (tmp = ifaCount; tmp /= 10; pos_x++);
        mvwprintw(userWin, pos_y += 2, 3, "[List of Physical Networks Interfaces]");
        mvwprintw(userWin, pos_y += 1, pos_x, NICS_LOGICAL_IFA_TITLE);
        mvwprintw(userWin, pos_y, pos_x += logicalWidth, NICS_PHYSICAL_IFA_TITLE);
        mvwprintw(userWin, pos_y, pos_x += physicalWidth, NICS_STATUS_TITLE);
        mvwprintw(userWin, pos_y++, pos_x += statusWidth, NICS_LINK_SPEED_TITLE);
        wattroff(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        for (int i = 0; i < printListCnt; i++) {
            wmove(userWin, pos_y + i, 0);
            whline(userWin, ' ', wbuf.ws_col - 1);
            mvwaddch(userWin, pos_y + i, wbuf.ws_col - 1, ' ');
        }
        for (int i = 0; i < printListCnt; i++) {
            if (i >= ifaCount) {
                break;
            }
            pos_x = 3 + 3;
            for (tmp = ifaCount; tmp /= 10; pos_x++);
            mvwprintw(userWin, pos_y + i, 3, "%d", i + 1 + line); // Index
            mvwprintw(userWin, pos_y + i, pos_x, "%s", ifaList[i + line].name); // Logical IFA
            mvwprintw(userWin, pos_y + i, pos_x += logicalWidth, "%s", ifaList[i + line].ifName); // Physical IFA

            if (ifaList[i + line].connected == TYPE_NICS_CONNECT) { // Connection Status
                mvwprintw(userWin, pos_y + i, pos_x += physicalWidth, "%s", TYPE_NICS_CONNECT_STR); // Connected
            } else {
                mvwprintw(userWin, pos_y + i, pos_x += physicalWidth, "%s", TYPE_NICS_DISCONNECT_STR); // Disconnected
            }

            if (STRN_CMP_EQUAL(ifaList[i + line].speed, NICS_LINK_SPEED_NOT_AVAILABLE)) { // N/A
                mvwprintw(userWin, pos_y + i, pos_x += statusWidth, "%s", NA_STR); // Link Speed
            } else {
                mvwprintw(userWin, pos_y + i, pos_x += statusWidth, "%s", ifaList[i + line].speed); // Link Speed
            }
        }

        mvwprintw(footLineWin, 0, 1, "To see the list of all network interface and its RX/TX speed, Press \"a\"");
        mvwprintw(footLineWin, 1, 1, "%s", footLabel);
        mvwprintw(footLineWin, 1, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);

        refresh();
        wrefresh(headLineWin);
        wrefresh(userWin);
        wrefresh(footLineWin);
        timeout(1000);
        switch(getch()) {
            case 'n':
            case 'N':
                line -= ((line > 0) ? 1 : 0);
                break;
            case 'm':
            case 'M':
                line += ((line < ifaCount - printListCnt) ? 1 : 0);
                break;
            case 'a':
            case 'A':
                display_IFA_Speed(&containerList, ifaSpeedList, ifaSpeedCnt);
                touchwin(headLineWin);
                touchwin(userWin);
                touchwin(footLineWin);
                break;
            case 'q':
            case 'Q':
            case 27: // ESC
                invoked_menu = 0;
                break;
        }
    }
    for (int i = 0; i < containerCnt; i++) {
        for (int j = 0; j < containerList[i].ifaCount; j++) {
            free_Array((void**)&(containerList[i].vethName[j]));
            free_Array((void**)&(containerList[i].ipv4_addr[j]));
        }
        free_Array((void**)&(containerList[i].checked));
        free_Array((void**)&(containerList[i].vethName));
        free_Array((void**)&(containerList[i].ipv4_addr));
        free_Array((void**)&(containerList[i].ifa_index));
    }
    free_Array((void**)&containerList); // Free UserList Array
    free_Array((void**)&ifaSpeedList);
    free_Array((void**)&ifaList); // Free UserList Array

    clear();
    delwin(headLineWin);
    delwin(userWin);
    delwin(footLineWin);
    display_main();
}

int get_Maximum_Length_of_Physical_IFAInfo(PHYSICAL_IFA_Info* ifaList, int ifaCount, int type) { // Get length of String
    int name = 0, tmp = 0;

    if (type == TYPE_NICS_LOGICAL_NAME) {
        name = (int)strlen(NICS_LOGICAL_IFA_TITLE);
    } else if (type == TYPE_NICS_PHYSICAL_NAME) {
        name = (int)strlen(NICS_PHYSICAL_IFA_TITLE);
    } else if (type == TYPE_NICS_STATUS) {
        name = (int)strlen(NICS_STATUS_TITLE);
    }

    for (int i = 0; i < ifaCount; i++) {
        if (type == TYPE_NICS_LOGICAL_NAME) {
            name = (((int)strlen(ifaList[i].name) > name) ? (int)strlen(ifaList[i].name) : name);
        } else if (type == TYPE_NICS_PHYSICAL_NAME) {
            name = (((int)strlen(ifaList[i].ifName) > name) ? (int)strlen(ifaList[i].ifName) : name);
        } else if (type == TYPE_NICS_STATUS) {
            tmp =  ((ifaList[i].connected == TYPE_NICS_CONNECT) ? (int)strlen(TYPE_NICS_CONNECT_STR) : (int)strlen(TYPE_NICS_DISCONNECT_STR));
            name = ((tmp > name) ? tmp : name);
        }
    }

    return name;
}

void display_IFA_Speed(DockerInfo** containerList, IFASpeed* ifaList, int ifaCount) {
    invoked_SubScreen = 1;

    UNIT unitRX, unitTX;
    float speedRX, speedTX;
    int pos_x = 3, pos_y = 1, containerCnt = 0, line = 0, nameWidth = 0, tmp, trash = 0;
    int ipv4Width = IPV4_LEN, speedWidth = 10, packetWidth = (int)strlen("Error(RX)   "), printListCnt = 0;
    char speedBuf[11] = { '\0' };

    nameWidth = get_Maximum_Length_of_Physical_IFASpeed(ifaList, ifaCount);
    
    WINDOW *headLineWin = newwin(1, wbuf.ws_col, 0, 0);
    WINDOW *userWin = newwin(wbuf.ws_row - 1 - 2, wbuf.ws_col, 1, 0);
    WINDOW *footLineWin = newwin(2, wbuf.ws_col, wbuf.ws_row - 2, 0);
    wbkgd(headLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wbkgd(footLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    mvwprintw(headLineWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
    mvwprintw(userWin, pos_y, pos_x - 1, "5. Network Informtion - List of All IFA, and its I/O Speed");
    mvwprintw(userWin, pos_y += 2, pos_x, "- Server Hostname: %s", hostnameBuf);
    mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 4, "!! Notification !!");
    mvwprintw(userWin, pos_y += 2, pos_x, "- # of Network IFA: %d (Virtual IFA Included)", ifaCount);
    mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET + 4, " - \"N/A\" means that no records exist for that ifa.");

    wattron(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wmove(userWin, pos_y + 2, 0);
    whline(userWin, ' ', wbuf.ws_col - 1);
    mvwaddch(userWin, pos_y + 2, wbuf.ws_col - 1, ' ');
    wmove(userWin, pos_y + 3, 0);
    whline(userWin, ' ', wbuf.ws_col - 1);
    mvwaddch(userWin, pos_y + 3, wbuf.ws_col - 1, ' ');
    pos_x = 1 + 2;
    for (tmp = ifaCount; tmp /= 10; pos_x++);
    mvwprintw(userWin, pos_y += 2, 1, "[List of All Networks Interfaces] - Virtual IFA included");
    mvwprintw(userWin, pos_y += 1, pos_x, "Interface");
    mvwprintw(userWin, pos_y, pos_x += nameWidth, "IPv4 Address");
    mvwprintw(userWin, pos_y, pos_x += ipv4Width, "Download");
    mvwprintw(userWin, pos_y, pos_x += speedWidth, "Upload");
    mvwprintw(userWin, pos_y, pos_x += speedWidth, "Error(RX)");
    mvwprintw(userWin, pos_y, pos_x += packetWidth, "Drop(RX)");
    mvwprintw(userWin, pos_y, pos_x += packetWidth, "Error(TX)");
    mvwprintw(userWin, pos_y, pos_x += packetWidth, "Drop(TX)");
    wattroff(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    pos_y++;
    printListCnt = wbuf.ws_row - pos_y - 1 - 2; // Total - Top Info - Header - Footer

    while(invoked_SubScreen == 1) {
        get_IFA_Speed(containerList, &ifaList, &trash, &containerCnt, 1);
        for (int i = 0; i < printListCnt; i++) {
            wmove(userWin, pos_y + i, 0);
            whline(userWin, ' ', wbuf.ws_col - 1);
            mvwaddch(userWin, pos_y + i, wbuf.ws_col - 1, ' ');
        }
        for (int i = 0; i < printListCnt; i++) {
            if (i >= ifaCount) {
                break;
            }
            pos_x = 1 + 2;
            for (tmp = ifaCount; tmp /= 10; pos_x++);

            mvwprintw(userWin, pos_y + i, 1, "%d", i + 1 + line); // Index
            mvwprintw(userWin, pos_y + i, pos_x, "%s", ifaList[i + line].ifa_name); // Logical IFA
            mvwprintw(userWin, pos_y + i, pos_x += nameWidth, "%s", ifaList[i + line].ipv4_addr); // IPv4 Address of ifa

            speedRX = convert_Size_Unit(ifaList[i + line].speedRX, 1, &unitRX);
            speedTX = convert_Size_Unit(ifaList[i + line].speedTX, 1, &unitTX);

            sprintf(speedBuf, "%.1f%s/s", speedRX, unitMap[unitRX].str); // Download
            mvwprintw(userWin, pos_y + i, pos_x += ipv4Width, "%s", speedBuf); 
            sprintf(speedBuf, "%.1f%s/s", speedTX, unitMap[unitTX].str); // Upload
            mvwprintw(userWin, pos_y + i, pos_x += speedWidth, "%s", speedBuf); 
            mvwprintw(userWin, pos_y + i, pos_x += speedWidth, "%ld", ifaList[i + line].errorRX); 
            mvwprintw(userWin, pos_y + i, pos_x += packetWidth, "%ld", ifaList[i + line].dropRX); 
            mvwprintw(userWin, pos_y + i, pos_x += packetWidth, "%ld", ifaList[i + line].errorTX); 
            mvwprintw(userWin, pos_y + i, pos_x += packetWidth, "%ld", ifaList[i + line].dropTX); 
        }
        mvwprintw(footLineWin, 0, 1, "To see the remaining, Press \"m\" (Next) and \"n\" (Previous).");
        mvwprintw(footLineWin, 1, 1, "%s", footLabel);
        mvwprintw(footLineWin, 1, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);

        refresh();
        wrefresh(headLineWin);
        wrefresh(userWin);
        wrefresh(footLineWin);
        timeout(1000);
        switch(getch()) {
            case 'n':
            case 'N':
                line -= ((line > 0) ? 1 : 0);
                break;
            case 'm':
            case 'M':
                line += ((line < ifaCount - printListCnt) ? 1 : 0);
                break;
            case 'q':
            case 'Q':
            case 27: // ESC
                invoked_SubScreen = 0;
                break;
        }
    }
    clear();
    delwin(headLineWin);
    delwin(userWin);
    delwin(footLineWin);
    return;
}

int get_Maximum_Length_of_Physical_IFASpeed(IFASpeed* ifaList, int ifaCount) { // Get length of String
    int name = 0;
    for (int i = 0; i < ifaCount; i++) {
        name = (((int)strlen(ifaList[i].ifa_name) > name) ? (int)strlen(ifaList[i].ifa_name) : name);
    }
    return name;
}