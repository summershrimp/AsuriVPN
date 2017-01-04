//
// Created by ubuntu on 12/29/16.
//

#include <arpa/inet.h>

#include "config.h"
#include "common.h"
struct vpnif device;
int listen_port;
int fd_max;
int l4proto;
unsigned int addr_begin;
unsigned int addr_max;
in_addr_t cfg_netmask;
int config_init() {
    strncpy(device.tun_dev, "tun0", IFNAMSIZ);
    inet_pton(AF_INET, "10.0.8.1", &device.addr);
    inet_pton(AF_INET, "255.255.255.0", &device.mask);
    l4proto = IPPROTO_UDP;
    listen_port = 9443;
    fd_max = 2000;
    addr_begin = ntohl(inet_addr("10.0.8.2"));
    addr_max = addr_begin + 200;
    cfg_netmask = inet_addr("255.255.255.0");
    return 0;
}

int config_client() {
    inet_pton(AF_INET, "10.0.8.2", &device.addr);
    inet_pton(AF_INET, "255.255.255.0", &device.mask);
}