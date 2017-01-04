//
// Created by ubuntu on 1/5/17.
//

#ifndef ASURIVPN_PROTOCAL_H
#define ASURIVPN_PROTOCAL_H

enum proto_type {MDHCP_REQ = 1, MDHCP_ACK, AUTH_NEED, AUTH_SEND, MSG};

struct asuri_proto{
    unsigned char version:8;
    enum proto_type type:8;
} __attribute__((packed));


#endif //ASURIVPN_PROTOCAL_H
