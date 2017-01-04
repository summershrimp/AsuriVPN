//
// Created by ubuntu on 1/3/17.
//

#ifndef ASURIVPN_IPCFG_H
#define ASURIVPN_IPCFG_H

#include <netinet/in.h>
#ifdef __cplusplus
extern "C"{
#endif

struct ipcfg {
    struct in_addr local_addr;
    struct sockaddr_in peer_addr; //use for udp connection
    int peer_fd; //use for tcp connection
    int last_alive; //last alive time, for udp connection timeout
};

struct ip_port{
    struct in_addr local_addr;
    in_port_t port;
};

struct mdhcp{
    struct in_addr address;
    struct in_addr netmask;
}__attribute__((packed));

extern int addr_allocated;


int ipcfg_connected(struct sockaddr_in addr);
struct mdhcp ipcfg_new_mdhcp(struct sockaddr_in peer_addr);
struct sockaddr_in ipcfg_get_peer_sockaddr(in_addr_t addr);

#ifdef __cplusplus
};
#endif

#endif //ASURIVPN_IPCFG_H
