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

int config_init() {
    strncpy(device.tun_dev, "tun0", IFNAMSIZ);
    inet_pton(AF_INET, "10.0.8.1", &device.addr);
    inet_pton(AF_INET, "255.255.255.0", &device.mask);
    l4proto = IPPROTO_UDP;
    listen_port = 9443;
    fd_max = 2000;
    return 0;
}

int config_client() {
    inet_pton(AF_INET, "10.0.8.2", &device.addr);
    inet_pton(AF_INET, "255.255.255.0", &device.mask);
}