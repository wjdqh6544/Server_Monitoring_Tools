#include "common.h"
#include "info_to_log.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

extern const DateInfo dateBuf;
extern FILE* log_ptr;

void signal_handler(int sig);
void run_as_background();
void* refresh_now_Date();
int main(void) {
    struct sigaction sa;
    pthread_t date, temp, usage, warning;
    char username[USERNAME_LEN] = { '\0' };

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, NULL);

    check_before_running(username);
    run_as_background();
    IO_Redirection(TYPE_COLLECTOR);

    get_Date();

    fprintf(stderr, "[INFO] %04d-%02d-%02d %02d:%02d:%02d %s: Collector program started.\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec, username);

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

void signal_handler(int sig) {
    if (sig == SIGTERM){
        printf("[INFO] %04d-%02d-%02d %02d:%02d:%02d Collector program stopped. (kill command)\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);
    } else if (sig == SIGKILL){
        printf("[INFO] %04d-%02d-%02d %02d:%02d:%02d Collector program stopped. (System shutdown)\n", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);
    }
    fclose(log_ptr);
    exit(1);
}

void run_as_background() {
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

void* refresh_now_Date() {
    while(1){
        get_Date();
        sleep(1);
    }
    return NULL;
}