//
// Created by ubuntu on 12/29/16.
//

#ifndef ASURIVPN_CLIENT_H
#define ASURIVPN_CLIENT_H

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

extern int client_fd;

int client_init();

#endif //ASURIVPN_CLIENT_H
