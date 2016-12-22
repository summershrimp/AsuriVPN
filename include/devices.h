//
// Created by ubuntu on 12/22/16.
//

#ifndef ASURIVPN_DEVICES_H
#define ASURIVPN_DEVICES_H

#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
struct vpnif {
    char tun_dev[IFNAMSIZ];
    in_addr_t addr;
    in_addr_t mask;
    int fd;
};

int tun_init(struct vpnif *dev);
int tun_remove(struct vpnif *dev);
int tun_set_address(struct vpnif *dev);
int tun_up(struct vpnif *dev);

#endif //ASURIVPN_DEVICES_H
