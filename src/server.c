//
// Created by ubuntu on 12/29/16.
//

#include <arpa/inet.h>

#include "server.h"
#include "device.h"
#include "event.h"
#include "config.h"
#include "common.h"

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
        client_addr.sin_port = 9999;
        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        err = server_init_udp();
        if(err < 0) {
            exit(-1);
        }
    } else if (l4proto == IPPROTO_TCP) {
        err = server_init_tcp();
        if(err < 0) {
            exit(-1);
        }
    }
    return 0;
}

int server_init_udp() {
    int sockfd, err;
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in addr;
    addr.sin_port = htons(listen_port);
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
    char buf[1500];
    int size, err;
    size = read(e.fd, buf, sizeof(buf));
    if(size < 0) {
        perror("read() - tun");
        return -1;
    }

    err = sendto(listen_fd, buf, size, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

    if(err < 0) {
        perror("write() - socket");
        return -1;
    }

    return 0;
}

int server_udp_handler(struct event e) {
    char buf[1500];
    int size, err;
    size = recvfrom(e.fd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &err);
    if(size < 0) {
        perror("read() - udp");
        return -1;
    }

    err = write(device.fd, buf, size);
    if(err < 0) {
        perror("write() - tun");
        return -1;
    }

    return 0;
}

int server_tcp_connect_handler(struct event e) {
    return 0;
}

int server_tcp_client_handler(struct event e) {
    return 0;
}