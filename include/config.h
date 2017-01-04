//
// Created by ubuntu on 12/29/16.
//

#ifndef ASURIVPN_CONFIG_H
#define ASURIVPN_CONFIG_H
#include "device.h"

extern struct vpnif device;
extern int listen_port;
extern int l4proto;
extern int fd_max;
extern unsigned int addr_begin;
extern unsigned int addr_max;
extern in_addr_t cfg_netmask;
int config_init();
int config_client();
#endif //ASURIVPN_CONFIG_H
