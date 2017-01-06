//
// Created by ubuntu on 12/29/16.
//

#ifndef ASURIVPN_SERVER_H
#define ASURIVPN_SERVER_H

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

extern int listen_fd;

int server_init();
#endif //ASURIVPN_SERVER_H
