//
// Created by ubuntu on 12/22/16.
//

#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "device.h"
#include "utils.h"
#include "common.h"

int tun_init(struct vpnif *dev) {
    struct ifreq ifr;
    int fd, err;
    fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) {
        perror_exit("open() - /dev/net/tun");

    }
    bzero(&ifr, sizeof(ifr));
    ifr.ifr_ifru.ifru_flags = IFF_TUN | IFF_NO_PI;
    if (dev->tun_dev) {
        strncpy(ifr.ifr_ifrn.ifrn_name, dev->tun_dev, IF_NAMESIZE);
    }

    err = ioctl(fd, TUNSETIFF, (void *) &ifr);
    if (err < 0) {
        close(fd);
        perror_exit("ioctl() - TUNSETIFF");
    }
    strcpy(dev->tun_dev, ifr.ifr_name);
    dev->fd = fd;
    return fd;
}

int tun_remove(struct vpnif *dev) {
    struct ifreq ifr;
    int err;
    err = ioctl(dev->fd, TUNSETPERSIST, 0);
    if (err < 0) {
        close(dev->fd);
        perror_exit("ioctl() - TUNSETPRESIST");
    }
    close(dev->fd);
    dev->fd = 0;
    return 0;
}

int tun_set_address(struct vpnif *dev) {
    struct ifreq ifr;
    struct sockaddr_in addr;
    int sockfd, err;
    bzero(&ifr, sizeof(ifr));
    bzero(&addr, sizeof(addr));

    strncpy(ifr.ifr_ifrn.ifrn_name, dev->tun_dev, IFNAMSIZ);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = dev->addr;

    sockfd = socket(addr.sin_family, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror_exit("socket() - SOCK_DGRAM");
    }

    ifr.ifr_ifru.ifru_addr = *(struct sockaddr *) &addr;
    err = ioctl(sockfd, SIOCSIFADDR, (void *) &ifr);
    if (err) {
        perror_exit("ioctl() - SIOCSIFADDR");
    }

    addr.sin_addr.s_addr = dev->mask;
    ifr.ifr_ifru.ifru_netmask = *(struct sockaddr *) &addr;
    err = ioctl(sockfd, SIOCSIFNETMASK, (void *) &ifr);
    if (err) {
        perror_exit("ioctl() - SIOCSIFADDR");
    }
    close(sockfd);
    return 0;
}

int tun_up(struct vpnif *dev) {
    struct ifreq ifr;
    int sockfd, err;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror_exit("socket() - SOCK_DGRAM");
    }
    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev->tun_dev, IFNAMSIZ);
    ifr.ifr_ifru.ifru_flags |= IFF_UP;
    ifr.ifr_ifru.ifru_flags |= IFF_RUNNING;
    err = ioctl(sockfd, SIOCSIFFLAGS, (void *) &ifr);
    if (err) {
        perror_exit("ioctl() - SIOCSIFFLAGS");
    }
    close(sockfd);
}