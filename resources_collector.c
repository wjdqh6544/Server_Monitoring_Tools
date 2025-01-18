#include "0_usrDefine.h"
#include "common.h"
#include "info_to_log.h"
#include "zz_struct.h"
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

DateInfo dateBuf;

void signal_handler(int sig);
void check_before_running(char* username);
void run_as_background();
void IO_Redirection();
void* refresh_now_Date();
int main(void){
    struct sigaction sa;
    pthread_t date, temp, usage, warning;
    char username[USERNAME_LEN] = { '\0' };

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, NULL);

    check_before_running(username);
    run_as_background();
    IO_Redirection();

    get_Date();

    printf("[INFO] %04d%02d%02d %02d:%02d:%02d %s: Collector program started.\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, username);

    pthread_create(&date, NULL, refresh_now_Date, NULL);
    pthread_create(&temp, NULL, write_Temperature_to_Log, NULL);
    pthread_create(&usage, NULL, write_Usage_to_Log, NULL);
    pthread_create(&warning, NULL, write_Warning_to_Log, NULL);

    pthread_join(date, NULL);
    pthread_join(temp, NULL);
    pthread_join(usage, NULL);
    pthread_join(warning, NULL);

    return 0;
}

void signal_handler(int sig){
    if (sig == SIGTERM){
        printf("[INFO] %04d%02d%02d %02d:%02d:%02d Collector program stopped. (kill command)\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);
    } else if (sig == SIGKILL){
        printf("[INFO] %04d%02d%02d %02d:%02d:%02d Collector program stopped. (System shutdown)\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);
    }
    exit(1);
}

void check_before_running(char* username){
    FILE* command = NULL;
    char buf[ERROR_MSG_LEN] = { '\0' };

    if (geteuid() != 0) { // Check the program is run with root privileges.
        printf("\nThis program must be running with root privileges. (using sudo or as root)...\n\nexit.\n\n");
        exit(-1);
    }

    if ((command = popen(CHECK_OMREPORT, "r")) == NULL) {
        printf("\nSystem Call(popen) invoking ERROR... \nexit.\n\n");
    } else {
        fgets(buf, sizeof(buf), command);
        if (strstr(buf, "No such file or directory") != NULL){
            printf("\n- OMSA (OpenManage Server Administrator) is not installed..");
            printf("\n- Please install OMSA. (Package Name: OM-SrvAdmin-Dell-Web-LX)");
            printf("\n- To download rpm package, please visit DELL Website.\n\nexit.\n\n");
            exit(-1);
        }
        pclose(command);
    }

    if ((command = popen(CHECK_PERCCLI, "r")) == NULL) {
        printf("\nSystem Call(popen) invoking ERROR... \nexit.\n\n");
    } else {
        fgets(buf, sizeof(buf), command);
        if (strstr(buf, "No such file or directory") != NULL){
            printf("\n- PERCCLI(PERC controller CLI utility) is not installed..");
            printf("\n- Please install Perccli. (Package Name: Perccli)");
            printf("\n- To download rpm package, please visit DELL Website.");
            printf("\n- This program uses perccli64. If the system is 32bit, Edit define macro. (Is in 0_usrDefine.h)\n\nexit.\n\n");
            exit(-1);
        }
        pclose(command);
    }

    if (check_Log_Directory(HISTORY_PATH, 0750) == -1){ // Check the presense of "/var/log/00_Server_Monitoring/00_history" directory. (History file is saved to this.)
        printf("\nCannot create Log destination directory. (%s).\nexit.\n\n", HISTORY_PATH);
        exit(-1);
    }

    strcpy(username, getpwuid(atoi(getenv("SUDO_UID")))->pw_name); // Get a username that run this (background collector) program.
}

void run_as_background(){
    pid_t pid;

    if ((pid = fork()) < 0) { // Create child process
        printf("\nSystem Call(fork) invoking ERROR... \nexit.\n\n"); // fork() ERROR
        exit(-1);
    } else if (pid > 0) { // Halt parent process -> Running only child process
        exit(1);
    }

    if (setsid() < 0){ // Create new session -> Operate independently from the existing sessions.
        printf("\nSystem Call(setsid) invoking ERROR... \nexit.\n\n");
        exit(-1);
    }

    if ((pid = fork()) < 0) { // Create grandchild process
        printf("\nSystem Call(fork) invoking ERROR... \nexit.\n\n"); // fork() ERROR
        exit(-1);
    } else if (pid > 0) { // Halt child process -> Running only grandchild process
        exit(1);
    }

    if (chdir("/") == -1){ // Change working directory -> Remove the dependency relative to a specific directory.
        printf("\nSystem Call(chdir) invoking ERROR... \nexit.\n\n");
        exit(-1);
    }

    umask(0); // Remove default permissions of files that are created this process. -> When create a file, must be set the permission
}

void IO_Redirection(){
    int fd = -1;

    fflush(stdout);
    fflush(stderr);

    fd = open("/dev/null", O_RDONLY);
    if (dup2(fd, STDIN_FILENO) == -1){ // Close (disable) stdin
        printf("\nSystem Call(dup2 - stdin) invoking ERROR... \nexit.");
        exit(-1);
    }
    close(fd);

    fd = open(ERROR_LOG_COLLECTOR, O_WRONLY | O_CREAT | O_APPEND, 0640);
    if (dup2(fd, STDOUT_FILENO) == -1) { // Redirect stdout to file
        printf("\nSystem Call(dup2 - stdout) invoking ERROR... \nexit.");
        exit(-1);
    }

    if (dup2(fd, STDERR_FILENO) == -1){ // Redirect stderr to file
        printf("\nSystem Call(dup2 - stderr) invoking ERROR... \nexit.");
        exit(-1);
    }

    close(fd);

    setvbuf(stdout, NULL, _IOLBF, 0); // Set buffering mode (Line buffering)
    setvbuf(stderr, NULL, _IOLBF, 0);
}

void* refresh_now_Date(){
    while(1){
        get_Date();
        sleep(1);
    }
    return NULL;
}