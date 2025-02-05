#include "0_usrDefine.h"
#include "common.h"
#include "os_monitor.h"
#include "resources_monitor.h"
#include "server_monitoring_tools.h"
#include <curses.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

pthread_t date, mainFunc;
struct sigaction sa;
struct winsize wbuf;
extern DateInfo dateBuf;
extern FILE* log_ptr;
int invoked_menu = 0, selMenuIdx = 0;
char hostnameBuf[MAX_PARTS_NAME_LEN] = { '\0' };

int main(void) {
    char username[USERNAME_LEN] = { '\0' };

    check_before_running(username);
    initialization();
    ioctl(0, TIOCGWINSZ, &wbuf);
    get_Date();
    fprintf(stderr, "-------------------------------------------------------------------------\n");
    fprintf(log_ptr, "[INFO] %04d-%02d-%02d %02d:%02d:%02d %s: Monitoring program started.\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, username);

    pthread_create(&date, NULL, refresh_Date_Winsize, NULL);
    pthread_create(&mainFunc, NULL, display_main, NULL);

    pthread_join(date, NULL);
    pthread_join(mainFunc, NULL);
}

void* refresh_Date_Winsize() { // Get latest date and winsize information
    while (1) {
        if (ioctl(0, TIOCGWINSZ, &wbuf) == -1) { // Get Windows Size
            exception(-4, "refresh_Date_Winsize", "ioctl()");
        }
        get_Date();
        sleep(1);
    }
    return NULL;    
}

void signal_handler(int sig) { // Signal Handler
    char username[USERNAME_LEN] = { '\0' };
    strcpy(username, getpwuid(atoi(getenv("SUDO_UID")))->pw_name); // Get a username that close this program.
    if (sig == SIGINT) { // Ctrl + C
        fprintf(log_ptr, "[WARNING] %04d-%02d-%02d %02d:%02d:%02d %s: SIGINT (Ctrl + C) signal occured.\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, username);
        shutdown_Program();
    } else if (sig == SIGQUIT) { /* Ctrl + \ */
        fprintf(log_ptr, "[WARNING] %04d-%02d-%02d %02d:%02d:%02d %s: SIGQUIT (Ctrl + \\) signal occured.\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, username);
        shutdown_Program();
    } else if (sig == SIGSEGV) { // Segmentation Fault
        fprintf(log_ptr, "[CRITICAL] %04d-%02d-%02d %02d:%02d:%02d %s: Segmentation fault (SIGSEGV) signal occured.\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, username);
        fprintf(stdout, "\nSegmentation Fault (SISEGB) Signal Occred.\nExit.\n\n");
        shutdown_Program();
    } else if (sig == SIGABRT) { // Aborted
        fprintf(log_ptr, "[CRITICAL] %04d-%02d-%02d %02d:%02d:%02d %s: Abort (SIGABRT) signal occured.\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, username);
        fprintf(stdout, "\nAbort (SIGABRT) Signal Occred.\nExit.\n\n");
        shutdown_Program();
    } else if (sig == SIGFPE) { // Floating Point Exception
        fprintf(log_ptr, "[CRITICAL] %04d-%02d-%02d %02d:%02d:%02d %s: Floating-Point Exception (SIGFPE) signal occured.\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, username);
        fprintf(stdout, "\nFloating-Point Exception (SIGFPE) Signal Occred.\n Exit.\n\n");
        shutdown_Program();
    } else if (sig == SIGILL) { // Illegal Instruction
        fprintf(log_ptr, "[CRITICAL] %04d-%02d-%02d %02d:%02d:%02d %s: Illegal Instruction (SIGILL) signal occured.\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, username);
        fprintf(stdout, "\nIllegal Instruction (SIGILL) Signal Occred.\nExit.\n\n");
        shutdown_Program();
    } else if (sig == SIGBUS) { // Bus Error
        fprintf(log_ptr, "[CRITICAL] %04d-%02d-%02d %02d:%02d:%02d %s: Bus Error (SIGBUS) signal occured.\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, username);\
        fprintf(stdout, "\nBus Error (SIGBUF) Signal Occred.\nExit.\n\n");
        shutdown_Program();
    }
}

void initialization() { // ncurses initialization
    savetty(); // Save current terminal status
    IO_Redirection(TYPE_MONITOR);
    get_Hostname(hostnameBuf);
    setenv("ESCDELAY", "25", 1); // Edit ESC Delay -> 25ms

    sa.sa_handler = signal_handler;
    sa.sa_flags = SA_NODEFER;
    sigemptyset(&sa.sa_mask); // Signal Clear
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    
    initscr();
    cbreak(); // Enable Line Buffering. / Disable: use raw()
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    use_default_colors();
    init_pair(BLACK_TEXT_WHITE_BACKGROUND, COLOR_BLACK, COLOR_WHITE); // Black Text, White Background
    init_pair(MAGENTA_TEXT, COLOR_MAGENTA, -1); // Red Text, No Background
    init_pair(GREEN_TEXT, COLOR_GREEN, -1); // Green Text, No Background
}

void* display_main() { // Print main menu
    const char* title = MAIN_TITLE;
    const char* tail_Content = DEPARTMENT;
    MenuItem menuList[MAX_MENU_ITEMS] = {
        {"1. System Information", one_System_Information},
        {"2. Hardware Temperature", two_Hardware_Temperature},
        {"3. CPU / Memory Usage", three_CPU_Memory_Usage},
        {"4. Disk Information", four_Disk_Information},
        {"5. Network Information", five_Network_Information},
        {"6. User Account Status", six_User_Account_Status},
        {"7. Login History", seven_Login_History},
        {"8. Core File Status", eight_Core_File_Status},
        {"9. About This Program", nine_About_This_Program},
        {"Quit", shutdown_Program}
    };
    int len_x = 0, len_y = MAX_MENU_ITEMS * 2 + 6, pos_x = 0, pos_y = 0, menuMaxLen = 0;
    
    for (int i = 0; i < MAX_MENU_ITEMS; i++) { // Get Box width
        len_x = ((int)strlen(menuList[i].menuStr) > len_x) ? (int)strlen(menuList[i].menuStr) : len_x;
    }

    menuMaxLen = len_x;
    len_x = (((int)strlen(title) > len_x) ? (int)strlen(title) : len_x);
    len_x += 4;
    pos_x = (len_x - strlen(title)) / 2;
    pos_y = 2;

    WINDOW *mainWin = newwin(len_y, len_x, ((wbuf.ws_row / 2) - (len_y / 2)), ((wbuf.ws_col / 2) - (len_x / 2))); // y length, x length, y position, x position
    WINDOW *footLineWin = newwin(1, wbuf.ws_col, wbuf.ws_row - 1, 0); // Tail Line: Hostname, System date
    wbkgd(footLineWin, COLOR_PAIR(1));
    wborder(mainWin, '|', '|', '-', '-', ' ', ' ', ' ', ' ');
    mvwprintw(mainWin, pos_y, pos_x, "%s", title);
    while(invoked_menu == 0) {
        pos_y = 3;
        for (int i = 0; i < MAX_MENU_ITEMS; i++) {
            if (i == selMenuIdx) {
                wattron(mainWin, A_REVERSE);
            }
            if (i == MAX_MENU_ITEMS - 1) { // Last -> "Quit"
                mvwprintw(mainWin, (pos_y += 2), len_x - 3 - strlen(menuList[i].menuStr), "%s", menuList[i].menuStr);
            } else {
                mvwprintw(mainWin, (pos_y += 2), (len_x - menuMaxLen) / 2, "%s", menuList[i].menuStr);
            }
            if (i == selMenuIdx) {
                wattroff(mainWin, A_REVERSE);
            }
        }
        mvwprintw(footLineWin, 0, 1, "HostName: %s", hostnameBuf);
        mvwprintw(footLineWin, 0, (wbuf.ws_col - strlen(tail_Content)) / 2 + 1, "%s", tail_Content);
        mvwprintw(footLineWin, 0, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);
        refresh();
        wrefresh(mainWin);
        wrefresh(footLineWin);
        timeout(1000);
        switch(getch()) {
            case KEY_UP:
                selMenuIdx = (selMenuIdx > 0) ? selMenuIdx - 1 : MAX_MENU_ITEMS - 1;
                break;
            case KEY_DOWN:
                selMenuIdx = (selMenuIdx < MAX_MENU_ITEMS - 1) ? selMenuIdx + 1 : 0;
                break;
            case 10:
                menuList[selMenuIdx].function();
                break;
        }
    }
    clear();
    delwin(mainWin);
    delwin(footLineWin);
    return NULL;
}

void shutdown_Program() {
    char username[USERNAME_LEN] = { '\0' };

    strcpy(username, getpwuid(atoi(getenv("SUDO_UID")))->pw_name); // Get a username that close this program.
    fprintf(log_ptr, "[INFO] %04d-%02d-%02d %02d:%02d:%02d %s: Monitoring program terminated.\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, username);
    fprintf(log_ptr, "-------------------------------------------------------------------------\n\n");
    fclose(log_ptr);
    curs_set(1); // Restore Terminal Settings
    echo();
    resetty();
    clear();
    endwin(); // Terminate ncurse window
    flushinp(); // Clear Buffer 
    exit(0);
}