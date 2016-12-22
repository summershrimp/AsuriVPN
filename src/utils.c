//
// Created by Summer on 12/22/16.
//
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void perror_exit(char *msg) {
    perror(msg);
    exit(errno);
}

int make_daemon() {
    pid_t ppid,psid;
    ppid = fork();
    if(ppid < 0) {
       perror_exit("Fork failed");
    } else if(ppid == 0) {
        psid = setsid();
        if(psid == -1) {
            perror_exit("Cannot set sid");
        }
        return 0;
    }
    exit(EXIT_SUCCESS);
    return -1;
}

