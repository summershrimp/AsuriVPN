//
// Created by ubuntu on 12/29/16.
//

#include <arpa/inet.h>

#include "server.h"
#include "device.h"
#include "event.h"
#include "config.h"
#include "common.h"
#include "ipcfg.h"
#include "protocal.h"
#include "utils.h"
#include "analyze.h"

int listen_fd = -1;
struct event listen_event;
struct sockaddr_in client_addr;
struct event tun_event;

int server_init_udp();
int server_init_tcp();
int server_udp_handler(struct event e);
int server_tcp_connect_handler(struct event e);
int server_tcp_client_handler(struct event e);
int server_tun_handler(struct event e);
int server_reply_mdhcp(struct sockaddr_in addr);
int server_send_to_tun(char *buf, unsigned int size);
int server_init() {
    int err;

    tun_init(&device);
    tun_set_address(&device);
    tun_up(&device);
    tun_event.fd = device.fd;
    tun_event.type = EVENT_TUN;
    tun_event.handler = server_tun_handler;
    event_add(&tun_event, EPOLLIN|EPOLLET);

    if(l4proto == IPPROTO_UDP) {
        err = server_init_udp();
        if(err < 0) {
            exit(-1);
        }
    } else if (l4proto == IPPROTO_TCP) {
        err = server_init_tcp();
        if(err < 0) {
            exit(-1);
        }
    } else {
        fputs("unknown link protocol\n", stderr);
        exit(-1);
    }
    return 0;
}

int server_init_udp() {
    int sockfd, err;
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in addr;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    err = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if(err < 0) {
        perror("bind()");
        exit(errno);
    }

    listen_fd = sockfd;
    listen_event.fd = listen_fd;
    listen_event.type = EVENT_SOCKET;
    listen_event.handler = server_udp_handler;
    event_add(&listen_event, EPOLLIN | EPOLLET);
    return 0;
}

int server_init_tcp(){
    fputs("TCP Connection has not been implemented\n", stderr);
    exit(-1);
    return -1;
}

int server_tun_handler(struct event e) {
    char buf[1400];
    char sendbuf[1500];
    struct asuri_proto proto;
    int size, err, sendsize = 0;
    in_addr_t ip;
    size = read(e.fd, buf, sizeof(buf));
    if (size < 0) {
        perror("read() - tun");
        return -1;
    }

    ip = get_dist_ip(buf, size);
    struct sockaddr_in peer_addr = ipcfg_get_peer_sockaddr(ip);
    if (peer_addr.sin_addr.s_addr != 0) {
        proto.type = MSG;
        proto.version = 1;
        memcpy(sendbuf, &proto, sizeof(proto));
        sendsize += sizeof(proto);
        memcpy(sendbuf + sendsize, buf, size);
        sendsize += size;
        err = sendto(listen_fd, sendbuf, sendsize, 0, (struct sockaddr *) &peer_addr, sizeof(peer_addr));
        if (err < 0) {
            perror("write() - socket");
            return -1;
        }
    }

    return 0;
}

int server_udp_handler(struct event e) {
    char buf[1500];
    int size, err;
    struct sockaddr_in addr;
    size = recvfrom(e.fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &err);

    if(size < 0) {
        perror("read() - udp");
        return -1;
    }

    struct asuri_proto *p = (struct asuri_proto*) buf;
    switch(p->type){
        case MDHCP_REQ: server_reply_mdhcp(addr);break;
        case AUTH_SEND: break;
        case MSG: server_send_to_tun(buf + sizeof(struct asuri_proto), size - sizeof(struct asuri_proto));
        default: break;
    }

    return 0;
}

int server_tcp_connect_handler(struct event e) {
    return 0;
}

int server_tcp_client_handler(struct event e) {
    return 0;
}

int server_reply_mdhcp(struct sockaddr_in addr) {
    struct mdhcp peer_addr = ipcfg_new_mdhcp(addr);
    struct asuri_proto proto;
    char buf[1400];
    int size = 0;
    unsigned int err;
    proto.version = 1;
    proto.type = MDHCP_ACK;
    memcpy(buf, &proto, sizeof(proto));
    size += sizeof(proto);
    memcpy(buf + size, &peer_addr, sizeof(peer_addr));
    size += sizeof(peer_addr);

    err = sendto(listen_fd, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));

    if(err < 0) {
        perror("write() - socket");
        return -1;
    }
    return 0;
}

int server_send_to_tun(char buf[], unsigned int size){
    unsigned int err;
    err = write(device.fd, buf, size);
    if(err < 0) {
        perror("write() - tun");
        return -1;
    }
    return 0;

}
