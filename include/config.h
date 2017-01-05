//
// Created by ubuntu on 12/29/16.
//

#ifndef ASURIVPN_CONFIG_H
#define ASURIVPN_CONFIG_H
#include "device.h"

extern struct vpnif device;
extern int port;
extern in_addr_t listen_addr;
extern int l4proto;
extern int fd_max;
extern unsigned int addr_begin;
extern unsigned int addr_max;
extern in_addr_t cfg_netmask;
extern char cfg_file[256];
extern in_addr_t connect_addr;

extern int client;
extern int server;

int config_init();

#endif //ASURIVPN_CONFIG_H
