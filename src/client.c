//
// Created by ubuntu on 12/29/16.
//
#include "common.h"
#include "client.h"
#include "device.h"
#include "config.h"
#include "event.h"
#include <arpa/inet.h>


struct event client_event;
struct event tun_event;
struct sockaddr_in server_addr;
int client_fd;

int client_init_udp();
int client_init_tcp();
int client_udp_handler(struct event e);
int client_tun_handler(struct event e);

int client_init() {
    int err;

    tun_init(&device);
    tun_set_address(&device);
    tun_up(&device);
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
    return 0;
}

int client_init_tcp() {
    fputs("TCP Connection has not been implemented\n", stderr);
    exit(-1);
    return -1;
}

int client_tun_handler(struct event e) {
    char buf[1500];
    int size, err;
    size = read(e.fd, buf, sizeof(buf));
    if(size < 0) {
        perror("read() - tun");
        return -1;
    }

    err = sendto(client_fd, buf, size, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if(err < 0) {
        perror("write() - socket");
        return -1;
    }

    return 0;
}

int client_udp_handler(struct event e) {
    char buf[1500];
    int size, err;
    size = read(e.fd, buf, sizeof(buf));
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