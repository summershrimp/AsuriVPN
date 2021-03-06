//
// Created by ubuntu on 12/29/16.
//

#include <arpa/inet.h>
#include <pthread.h>

#include "server.h"
#include "device.h"
#include "event.h"
#include "config.h"
#include "common.h"
#include "ipcfg.h"
#include "protocal.h"
#include "utils.h"
#include "analyze.h"

#define MAXBUF 1024

int listen_fd = -1;
struct event listen_event;
struct sockaddr_in client_addr;
struct event tun_event;

SSL_CTX *ctx;

int server_init_udp();
int server_init_tcp();
int server_udp_handler(struct event *e);
int server_tcp_connect_handler(struct event *e);
int server_tcp_client_handler(struct event *e);
int server_tun_handler(struct event *e);
int server_reply_mdhcp(struct sockaddr_in addr);
int server_reply_mdhcp_tcp(struct ipcfg *info);
int server_send_to_tun(char *buf, unsigned int size);

int server_init() {
    int err;

    tun_init(&device);
    tun_set_address(&device);
    tun_up(&device);
    tun_event.fd = device.fd;
    tun_event.type = EVENT_TUN;
    tun_event.handler = server_tun_handler;
    event_add(&tun_event, EPOLLIN);

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
        log_error("unknown link protocol");
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
    event_add(&listen_event, EPOLLIN);

    return 0;
}

int server_init_tcp(){
    ctx = SSL_CTX_new(SSLv23_server_method());
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        log_error("Use certificate failed");
        exit(-1);
    } else {
        log_info("Use certificate success");
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
        log_error("Use private key failed");
        exit(-1);
    } else {
        log_info("Use private key success");
    }

    int sockfd, err;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    err = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if(err < 0) {
        perror("bind()");
        exit(errno);
    }
    listen(sockfd, 128);
    listen_fd = sockfd;
    listen_event.fd = listen_fd;
    listen_event.type = EVENT_SOCKET;
    listen_event.handler = server_tcp_connect_handler;
    event_add(&listen_event, EPOLLIN);
    return 0;
}

int server_tun_handler(struct event *e) {
    char buf[1400];
    char sendbuf[1500];
    struct asuri_proto proto;
    int size, err, sendsize = 0, fd;
    in_addr_t ip;
    struct ipcfg *info;
    size = read(e->fd, buf, sizeof(buf));
    if (size < 0) {
        perror("read() - tun");
        return -1;
    }

    log_debug("message from tun, size: %d", size);
    ip = get_dist_ip(buf, size);
    if(l4proto == IPPROTO_UDP) {

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
                perror("write() - tun msg udp");
                return -1;
            }
        }
    } else if(l4proto == IPPROTO_TCP){
        info = ipcfg_map_find(ip);
        if(!info){
            return -1;
        }
        proto.type = MSG;
        proto.version = 1;
        memcpy(sendbuf, &proto, sizeof(proto));
        sendsize += sizeof(proto);
        memcpy(sendbuf + sendsize, buf, size);
        sendsize += size;
        SSL_write(info->ssl, sendbuf, sendsize);
        if (err < 0) {
            perror("write() - tun msg tcp");
            return -1;
        }
    }

    return 0;
}

int server_udp_handler(struct event *e) {
    char buf[1500];
    int size, err;
    struct sockaddr_in addr;

    size = recvfrom(e->fd, buf, sizeof(buf), 0, (struct sockaddr *) &addr, &err);

    if (size < 0) {
        perror("read() - udp");
        return -1;
    }

    struct asuri_proto *p = (struct asuri_proto *) buf;
    switch (p->type) {
        case MDHCP_REQ:
            server_reply_mdhcp(addr);
            break;
        case AUTH_SEND:
            break;
        case MSG:
            server_send_to_tun(buf + sizeof(struct asuri_proto), size - sizeof(struct asuri_proto));
        default:
            break;
    }

    return 0;
}

int server_tcp_connect_handler(struct event *e) {
    SSL *ssl;
    struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));
    bzero(addr, sizeof(struct sockaddr_in));
    int len = sizeof(struct sockaddr_in);
    int client = accept(e->fd, (struct sockaddr *)addr, &len);
    if (client < 0) {
        perror("accept() - tcp");
        return -1;
    }
    log_debug("tcp new client: %s:%d ", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client);
    if (SSL_accept(ssl) != 1) {
        ERR_print_errors_fp(stderr);
        close(e->fd);
        return -1;
    }
    log_debug("SSL accepted");
    struct ipcfg *info = malloc(sizeof(struct ipcfg));
    info->peer_fd = client;
    info->last_alive = time(NULL);
    info->peer_addr = *addr;
    info->ssl = ssl;

    struct event *ev;
    ev = malloc(sizeof(struct event));
    ev->fd = client;
    ev->handler = server_tcp_client_handler;
    ev->type = EVENT_SOCKET;
    ev->ptr = info;
    event_add(ev, EPOLLIN);
    return 0;
}

int server_tcp_client_handler(struct event *e) {
    char buf[1500];
    int size, err;
    struct sockaddr_in *addr;
    struct ipcfg *info = e->ptr;
    SSL *ssl = info->ssl;
    size = SSL_read(ssl, buf, 1500);
    if (size > 0) {
        log_debug("Receive ssl data: %d", size);
        struct asuri_proto *p = (struct asuri_proto *) buf;
        switch (p->type) {
            case MDHCP_REQ:
                server_reply_mdhcp_tcp(info);
                break;
            case AUTH_SEND:
                break;
            case MSG:
                server_send_to_tun(buf + sizeof(struct asuri_proto), size - sizeof(struct asuri_proto));
            default:
                printf("%s", buf);
                break;
        }
    } else {
        err = SSL_get_error(ssl, size);
        switch (err) {
            case SSL_ERROR_ZERO_RETURN:
                log_debug("tcp client disconnected. ");
                event_delete(e, EPOLLIN);
                ipcfg_map_delete(info->local_addr.s_addr);
                free(e->ptr);
                free(e);
                SSL_shutdown(ssl);
                SSL_free(ssl);
                close(e->fd);
                return -1;
                break;
            case SSL_ERROR_WANT_READ:
                log_debug("SSL_ERROR_WANT_READ");
                break;
            case SSL_ERROR_WANT_WRITE:
                log_debug("SSL_ERROR_WANT_WRITE");
                break;
            case SSL_ERROR_SYSCALL:
                log_debug("SSL_ERROR_SYSCALL");
                break;
            case SSL_ERROR_SSL:
                log_debug("SSL_ERROR_SSL");
                break;
            default:
                log_debug("SSL_get_error returns %d", err);
                break;
        }
    }

    return 0;
}

int server_reply_mdhcp_tcp(struct ipcfg *info){
    int size = 0, err;
    struct mdhcp dhcp_addr = ipcfg_new_mdhcp(info->peer_addr);
    info->local_addr = dhcp_addr.address;
    struct asuri_proto proto;
    char buf[1400];
    ipcfg_map_add(dhcp_addr.address.s_addr, info);
    proto.version = 1;
    proto.type = MDHCP_ACK;
    memcpy(buf, &proto, sizeof(proto));
    size += sizeof(proto);
    memcpy(buf + size, &dhcp_addr, sizeof(dhcp_addr));
    size += sizeof(dhcp_addr);

    err = SSL_write(info->ssl, buf, size);

    if(err < 0) {
        perror("write() - mdhcp-tcp");
        return -1;
    }
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

    err = write(listen_fd, buf, size);

    if(err < 0) {
        perror("write() - mdhcp-udp");
        return -1;
    }
    return 0;
}

int server_send_to_tun(char buf[], unsigned int size){
    unsigned int err;
    err = write(device.fd, buf, size);
    log_debug("message send to tun, size: %d", size);
    if(err < 0) {
        perror("write() - tun");
        return -1;
    }
    return 0;

}
