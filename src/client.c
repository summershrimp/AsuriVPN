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
#include <pthread.h>

#define MAXBUF 1024

struct event client_event;
struct event tun_event;
struct sockaddr_in server_addr;
int client_fd;

pthread_t ttun,tudp;

SSL *ssl;
SSL_CTX *ctx;

int client_init_udp();
int client_init_tcp();
int client_udp_handler(struct event *e);
int client_tcp_handler(struct event *e);
int client_tun_handler(struct event *e);
int client_set_address(struct mdhcp address);
int client_send_to_tun(char *buf, int size);

void ShowCerts(SSL * ssl) {
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        printf("数字证书信息:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("证书: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("颁发者: %s\n", line);
        free(line);
        X509_free(cert);
    } else {
        printf("无证书信息！\n");
    }
}

void* pthread_client_tun(void *ptr){
    while(1){
        log_debug("tun-handler");
        client_tun_handler(&tun_event);
    }
}

void* pthread_client_udp(void *ptr){
    while(1){
        log_debug("udp-handler");
        client_udp_handler(&client_event);
    }
}

int client_init() {
    int err;

    tun_init(&device);
    tun_event.fd = device.fd;
    tun_event.type = EVENT_TUN;
    tun_event.handler = client_tun_handler;
    event_add(&tun_event, EPOLLIN);
    if(l4proto == IPPROTO_UDP) {
        err = client_init_udp();
        if(err < 0) {
            exit(-1);
        }
        //pthread_create(&ttun, NULL, pthread_client_tun, NULL);
        //pthread_create(&tudp, NULL, pthread_client_udp, NULL);
        //pthread_join(tudp, NULL);
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

    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = connect_addr;

    client_event.fd = sockfd;
    client_event.type = EVENT_SOCKET;
    client_event.handler = client_udp_handler;
    event_add(&client_event, EPOLLIN);

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
    ctx = SSL_CTX_new(SSLv23_client_method());

    int sockfd, err;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    client_fd = sockfd;

    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = connect_addr;

    err = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(err != 0){
        perror("connect() - tcp");
        exit(errno);
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    if (SSL_connect(ssl) == -1) {
        ERR_print_errors_fp(stderr);
    } else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        ShowCerts(ssl);
    }

    client_event.fd = sockfd;
    client_event.type = EVENT_SOCKET;
    client_event.handler = client_tcp_handler;
    event_add(&client_event, EPOLLIN);

    struct asuri_proto proto;
    proto.version = 1;
    proto.type = MDHCP_REQ;

    err = write(client_fd, &proto, sizeof(proto));
    if(err < 0) {
        perror("sendto() - socket MDHCP_REQ");
        return -1;
    }
    return 0;
}

int client_tcp_handler(struct event *e) {
    char buf[1500];
    int size, err;

    // bzero(buffer, MAXBUF + 1);
    // size = SSL_read(ssl, buffer, MAXBUF);
    // if (size > 0) {
    //     printf("接收消息成功:'%s'，共%d个字节的数据\n", buffer, size);
    // } else {
    //     printf("消息接收失败！错误代码是%d，错误信息是'%s'\n", errno, strerror(errno));
    //     exit(errno);
    // }

    // bzero(buffer, MAXBUF + 1);
    // scanf("%s", buffer);
    // size = SSL_write(ssl, buffer, strlen(buffer));
    // if (size <= 0) {
    //     log_error("Send '%s' failed, errno:%d, msg:'%s'", buffer, errno, strerror(errno));
    // } else {
    //     log_info("Send '%s' success", buffer);
    // }

    size = SSL_read(ssl, buf, MAXBUF);
    if(size < 0) {
        perror("read() - tcp");
        return -1;
    }
    struct asuri_proto *p = (struct asuri_proto *) buf;
    switch(p->type){
        case MDHCP_ACK: client_set_address(*((struct mdhcp*)(buf + sizeof(struct asuri_proto)))); break;
        case AUTH_NEED: break;
        case MSG: client_send_to_tun(buf+sizeof(struct asuri_proto), size - sizeof(struct asuri_proto));
        default: break;
    }

    return 0;
}

int client_tun_handler(struct event *e) {
    char buf[1400];
    char sendbuf[1500];
    int size, err, sendsize = 0;
    size = read(e->fd, buf, sizeof(buf));
    if(size < 0) {
        perror("read() - tun");
        return -1;
    }
    log_debug("message from tun, size: %d", size);
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

int client_udp_handler(struct event *e) {
    char buf[1500];
    int size, err;
    struct asuri_proto *p;
    size = read(e->fd, buf, sizeof(buf));
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
    log_debug("message send to tun, size: %d", size);
    err = write(device.fd, buf, size);
    if(err < 0) {
        perror("write() - tun");
        return -1;
    }
}
