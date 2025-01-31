#ifndef FILE_LIST
#define FILE_LIST

/*
    Absolute path length of each file must be smaller than 256 or equals.
    If need to be long of the length, Edit "BUF_MAX_LINE" macro. (In 0_usrDefine.h)
    (! Important !) Don't decrease the value of BUF_MAX_LINE below "256". (If do that, The program may not work.)
*/
char* targetFile[] = { // <Abstract path of file> <Owner Name> <Group Name> <Permissions - Octal Number (ex. 777, 0654, and so on..)>
    "/etc/passwd root root 0644",
    "/etc/shadow root root 0640",
    "/etc/hosts root root 0600",
    "/home/knu_cse_sdd/00_project/server_monitoring_tools knu_cse_sdd knu_cse_sdd 0750",
    "/home/knu_cse_sdd/00_project/common.c knu_cse_sdd knu_cse_sdd 777",
    "/etc/hosts.allow root root 0600"
};

#endif