//
// Created by ubuntu on 12/29/16.
//

#include <arpa/inet.h>

#include "config.h"
#include "common.h"
#include "utils.h"
#include <ctype.h>
struct vpnif device;
int port = 0;
int fd_max = 0;
int l4proto = 0;
int client = 0;
int server = 0;

unsigned int addr_begin;
unsigned int addr_max;
in_addr_t cfg_netmask;
in_addr_t listen_addr;
in_addr_t connect_addr;

char cfg_file[256] = "asurivpn.conf";
FILE *cfg;

int line_process(char *buf);
in_addr_t calc_subnet_begin(char *addr, int netmask);
int calc_subnet_host_count(int t);
int key_compare(char* key, char *str);
in_addr_t calc_subnet_netmask(int n);

int config_init() {
    char buf[1024];
    char *p;
    cfg = fopen(cfg_file, "r");

    if(NULL == cfg){
        perror("fopen() - cfg_file");
    }
    while(!feof(cfg)){
        p = fgets(buf, sizeof(buf),cfg);
        if(p == NULL || strblank(buf)){
            continue;
        }
        line_process(buf);
    }
    fd_max = 1024;
    return 0;
}

int line_process(char *buf){
    char tmp[1024];
    int t, n;
    if(!key_compare("server", buf)){
        server = 1;
        log_info("Server mode configured.");
    } else if(!key_compare("client", buf)) {
        client = 1;
        log_info("Client mode configured.");
    }else if(!key_compare("listen", buf)) {
        sscanf(buf, "listen %s",tmp);
        listen_addr = inet_addr(tmp);
        if(listen_addr == INADDR_NONE) {
            log_error("Wrong listen address: %s", tmp);
            exit(-1);
        }
        log_info("Listen on: %s configured", inet_ntoa(*(struct in_addr *)&listen_addr));
    }else if(!key_compare("port", buf)) {
        t = sscanf(buf, "port %d", &port);
        if(t == 0){
            log_error("port in config file error.");
            exit(0);
        }
        log_info("port: %d configured", port);
    }else if(!key_compare("net", buf)) {
        n = sscanf(buf, "net %s", tmp);
        char *p;
        if(n != 1){
            log_error("subnet config error.");
            exit(0);
        }
        strtok(tmp,"/");
        p = strtok(NULL, "/");
        if(p == NULL) {
            log_error("subnet mask config error.");
            exit(0);
        }
        t = atoi(p);
        if(t == 0){
            log_error("subnet mask config error.");
            exit(0);
        }
        addr_begin = calc_subnet_begin(tmp, t);
        if(addr_begin == INADDR_NONE) {
            log_error("Wrong subnet address: %s", tmp);
            exit(-1);
        }
        cfg_netmask = calc_subnet_netmask(t);
        if(cfg_netmask == INADDR_NONE){
            log_error("Wrong subnet netmask: %d", t);
            exit(-1);
        }
        addr_max = calc_subnet_host_count(t);
        unsigned int tns = htonl(addr_begin);
        device.mask = cfg_netmask;
        device.addr = tns;
        ++addr_begin;
        log_info("Subnet configured, address: %s ", inet_ntoa(*(struct in_addr *)&tns));
        log_info("netmask: %s", inet_ntoa(*(struct in_addr *)&cfg_netmask));
    }else if(!key_compare("udp", buf)) {
        l4proto = IPPROTO_UDP;
        log_info("udp mode configured.");
    }else if(!key_compare("tcp", buf)) {
        l4proto = IPPROTO_TCP;
        log_info("udp mode configured.");
    }else if(!key_compare("connect", buf)) {
        sscanf(buf, "connect %s",tmp);
        connect_addr = inet_addr(tmp);
        if(listen_addr == INADDR_NONE) {
            log_error("Wrong connect address: %s");
            exit(-1);
        }
        log_info("connect %s configured", inet_ntoa(*(struct in_addr *)&connect_addr));
    } else {
        log_warn("Unknown config: %s", buf);
    }
}

int key_compare(char* key, char *str){
    int i;
    size_t tk = strlen(key);
    size_t ts = strlen(str);
    if(tk>ts) return -1;
    for(i=0; i<tk; ++i) {
        if(key[i] != str[i])
            return -1;
    }
    if(isspace(str[i]) || str[i] == 0)
        return 0;
    return -1;
}

in_addr_t calc_subnet_begin(char *addr, int netmask){
    int t = 32 - netmask;
    int i;
    if(t < 1){
        return INADDR_NONE;
    }
    in_addr_t address = inet_addr(addr);
    unsigned int inet_addr = ntohl(address);
    unsigned int inet_mask = 0;
    for(i = 0; i<netmask; ++i) {
        inet_mask <<= 1;
        inet_mask |= 1;
    }
    for(i = 0; i<t; ++i) {
        inet_mask <<= 1;
        inet_mask |= 0;
    }
    return (inet_addr & inet_mask ) + 1;
}

int calc_subnet_host_count(int t){
    int p = 32 - t;
    int i = 1;
    if(p <= 0)
        return 0;
    while(p--){
        i*=2;
    }
    return i - 3;
}
in_addr_t calc_subnet_netmask(int n){
    unsigned int inet_mask = 0;
    int t = 32 - n, i;
    if(t < 0){
        return INADDR_NONE;
    }
    for(i = 0; i<n; ++i) {
        inet_mask <<= 1;
        inet_mask |= 1;
    }
    for(i = 0; i<t; ++i) {
        inet_mask <<= 1;
    }
    return htonl(inet_mask);
}