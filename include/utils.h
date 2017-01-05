//
// Created by ubuntu on 12/22/16.
//

#ifndef ASURIVPN_UTILS_H
#define ASURIVPN_UTILS_H

#include <netinet/in.h>
int make_daemon();
void perror_exit(char *msg);
int make_nonblock(int fd);
in_addr_t get_dist_ip(unsigned char buf[], int size);
int strblank(char *str);
#endif //ASURIVPN_UTILS_H
