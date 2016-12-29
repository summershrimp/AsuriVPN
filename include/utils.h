//
// Created by ubuntu on 12/22/16.
//

#ifndef ASURIVPN_UTILS_H
#define ASURIVPN_UTILS_H
int make_daemon();
void perror_exit(char *msg);
int make_nonblock(int fd);

#endif //ASURIVPN_UTILS_H
