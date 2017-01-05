//
// Created by Summer on 12/22/16.
//
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/ip.h>

#include "utils.h"
#include "common.h"
#include "ctype.h"
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

int make_nonblock(int fd) {
    int flags, s;

    flags = fcntl (fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror ("fcntl() - F_GETFL");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl (fd, F_SETFL, flags);
    if (s == -1)
    {
        perror ("fcntl() - F_SETFL, flags |= O_NONBLOCK");
        return -1;
    }

    return 0;
}

in_addr_t get_dist_ip(unsigned char buf[], int size){
    struct iphdr *hdr = (struct iphdr *) buf;
    return hdr->daddr;
}


int strblank(char *str){
    int t = strlen(str);
    int i;
    for(i=0; i<t; ++i){
        if(!isspace(str[i]))
            return 0;
    }
    return 1;
}