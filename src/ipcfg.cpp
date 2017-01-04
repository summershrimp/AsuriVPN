//
// Created by ubuntu on 1/3/17.
//

#include "ipcfg.h"
#include "common.h"
#include "config.h"
#include <time.h>
#include <arpa/inet.h>
#include <map>
#include <set>
using namespace std;

int addr_allocated;

map<in_addr_t, ipcfg> local_addr_map;

struct sockaddr_cmp {
    bool operator()(const sockaddr_in &a, const sockaddr_in &b) const{
      if(a.sin_addr.s_addr == b.sin_addr.s_addr)  {
          return ntohl(a.sin_port) < ntohl(b.sin_port);
      } else {
          return ntohl(a.sin_addr.s_addr) < ntohl(b.sin_addr.s_addr);
      }
    };
};

set<sockaddr_in, sockaddr_cmp> peeraddr_set;

struct mdhcp ipcfg_new_mdhcp(struct sockaddr_in peer_addr){
    struct mdhcp addr;
    if(addr_allocated < addr_max) {
        addr.address.s_addr = htonl(addr_begin + addr_allocated);
        ++addr_allocated;
    } else {
        addr.address.s_addr = 0;
    }
    addr.netmask.s_addr = cfg_netmask;
    struct ipcfg peer;
    peer.last_alive = time(NULL);
    peer.local_addr = addr.address;
    peer.peer_addr = peer_addr;
    if(addr.address.s_addr != 0){
        in_addr_t t = addr.address.s_addr;
        local_addr_map.insert(make_pair<in_addr_t, ipcfg>(move(t), move(peer)));
        peeraddr_set.insert(move(peer_addr));
    }
    return addr;
}

int ipcfg_connected(struct sockaddr_in addr){
    return peeraddr_set.find(addr) != peeraddr_set.end();
}

struct sockaddr_in ipcfg_get_peer_sockaddr(in_addr_t addr) {
    ipcfg emptycfg;
    emptycfg.peer_addr.sin_addr.s_addr = 0;
    if(local_addr_map.find(addr) != local_addr_map.end()) {
        ipcfg &cfg = local_addr_map.at(addr);
        cfg.last_alive = time(NULL);
        return cfg.peer_addr;
    }

    return emptycfg.peer_addr;
}

