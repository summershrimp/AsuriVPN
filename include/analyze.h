//
// Created by ubuntu on 12/22/16.
//

#ifndef ASURIVPN_ANALYZE_H
#define ASURIVPN_ANALYZE_H

void print_ether_header(unsigned char *buffer, int size);

void print_ip_header(unsigned char *, int);

void print_tcp_packet(unsigned char *, int);

void print_udp_packet(unsigned char *, int);

void print_icmp_packet(unsigned char *, int);

void print_data(unsigned char *, int);

extern FILE *logfile;
#endif //ASURIVPN_ANALYZE_H
