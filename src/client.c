//
// Created by ubuntu on 12/29/16.
//
#include "common.h"
#include "client.h"
#include "device.h"
#include "config.h"
#include "event.h"
#include "protocal.h"
#include "ipcfg.h"
#include <arpa/inet.h>


struct event client_event;
struct event tun_event;
struct sockaddr_in server_addr;
int client_fd;

int client_init_udp();
int client_init_tcp();
int client_udp_handler(struct event e);
int client_tun_handler(struct event e);
int client_set_address(struct mdhcp address);
int client_send_to_tun(char *buf, int size);

int client_init() {
    int err;

    tun_init(&device);
    tun_event.fd = device.fd;
    tun_event.type = EVENT_TUN;
    tun_event.handler = client_tun_handler;
    event_add(&tun_event, EPOLLIN|EPOLLET);
    if(l4proto == IPPROTO_UDP) {
        err = client_init_udp();
        if(err < 0) {
            exit(-1);
        }
    } else if (l4proto == IPPROTO_TCP) {
        err = client_init_tcp();
        if(err < 0) {
            exit(-1);
        }
    }
    return 0;
}

int client_init_udp() {
    int sockfd, err;
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    client_fd = sockfd;

    server_addr.sin_port = htons(listen_port);
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "10.16.40.23", &server_addr.sin_addr);

    client_event.fd = sockfd;
    client_event.type = EVENT_SOCKET;
    client_event.handler = client_udp_handler;
    event_add(&client_event, EPOLLIN|EPOLLET);

    struct asuri_proto proto;
    proto.version = 1;
    proto.type = MDHCP_REQ;

    err = sendto(client_fd, &proto, sizeof(proto), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(err < 0) {
        perror("sendto() - socket MDHCP_REQ");
        return -1;
    }
    return 0;
}

int client_init_tcp() {
    fputs("TCP Connection has not been implemented\n", stderr);
    exit(-1);
    return -1;
}

int client_tun_handler(struct event e) {
    char buf[1400];
    char sendbuf[1500];
    int size, err, sendsize = 0;
    size = read(e.fd, buf, sizeof(buf));
    if(size < 0) {
        perror("read() - tun");
        return -1;
    }
    struct asuri_proto proto;
    proto.type = MSG;
    proto.version = 1;
    memcpy(sendbuf, &proto, sizeof(proto));
    sendsize += sizeof(proto);
    memcpy(sendbuf + sendsize, buf, size);
    sendsize += size;

    err = sendto(client_fd, sendbuf, sendsize, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if(err < 0) {
        perror("write() - socket");
        return -1;
    }

    return 0;
}

int client_udp_handler(struct event e) {
    char buf[1500];
    int size, err;
    struct asuri_proto *p;
    size = read(e.fd, buf, sizeof(buf));
    if(size < 0) {
        perror("read() - udp");
        return -1;
    }

    p = (struct asuri_proto *) buf;
    switch(p->type){
        case MDHCP_ACK: client_set_address(*((struct mdhcp*)(buf + sizeof(struct asuri_proto)))); break;
        case AUTH_NEED: break;
        case MSG: client_send_to_tun(buf+sizeof(struct asuri_proto), size - sizeof(struct asuri_proto));
        default: break;
    }

    return 0;
}

int client_set_address(struct mdhcp address){
    device.addr = address.address.s_addr;
    device.mask = address.netmask.s_addr;
    tun_set_address(&device);
    tun_up(&device);
    return 0;
}

int client_send_to_tun(char *buf, int size){
    int err;
    err = write(device.fd, buf, size);
    if(err < 0) {
        perror("write() - tun");
        return -1;
    }
}