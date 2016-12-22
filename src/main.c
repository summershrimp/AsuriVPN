#include <stdio.h>
#include <sys/socket.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "devices.h"
#include "analyze.h"

char buf[1500];

int main() {
    //make_daemon();
    struct vpnif vpni;
    int count;
    strncpy(vpni.tun_dev, "tun0", IFNAMSIZ);
    inet_pton(AF_INET, "10.0.8.1", &vpni.addr);
    inet_pton(AF_INET, "255.255.255.0", &vpni.mask);
    tun_init(&vpni);
    tun_set_address(&vpni);
    tun_up(&vpni);
    logfile = stdout;
    while (1) {
        count = read(vpni.fd, buf, 1500);

        if (count > 0) {
            print_data(buf, 4);
            print_ip_header(buf + 4, count - 4);
        }
    }
    return 0;
}

