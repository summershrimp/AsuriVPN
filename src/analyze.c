//
// Created by ubuntu on 12/22/16.
//

//
//  main.c
//  seclab-scanner
//
//  Created by Summer on 6/9/16.
//  Copyright © 2016 summer. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include "analyze.h"

int sock_raw;
FILE *logfile;
int i, j;
struct sockaddr_in source, dest;


void print_ether_header(unsigned char *buffer, int size) {
    struct ether_header *ethhdr = (struct ether_header *) buffer;

    fprintf(logfile, "\n");
    fprintf(logfile, "MAC Header\n");
    fprintf(logfile, "   |-Destination Mac   : %02x:%02x:%02x:%02x:%02x:%02x\n",
            ethhdr->ether_dhost[0], ethhdr->ether_dhost[1], ethhdr->ether_dhost[2], ethhdr->ether_dhost[3],
            ethhdr->ether_dhost[4], ethhdr->ether_dhost[5]);
    fprintf(logfile, "   |-Source Mac        : %02x:%02x:%02x:%02x:%02x:%02x\n",
            ethhdr->ether_shost[0], ethhdr->ether_shost[1], ethhdr->ether_shost[2], ethhdr->ether_shost[3],
            ethhdr->ether_shost[4], ethhdr->ether_shost[5]);
    fprintf(logfile, "   |-Protocol          : %d\n", ethhdr->ether_type);
}

void print_ip_header(unsigned char *buffer, int size) {
    unsigned short iphdrlen;

    struct iphdr *iph = (struct iphdr *) buffer;
    iphdrlen = iph->ihl * 4;

    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;

    fprintf(logfile, "\n");
    fprintf(logfile, "IP Header\n");
    fprintf(logfile, "   |-IP Version        : %d\n", (unsigned int) iph->version);
    fprintf(logfile, "   |-IP Header Length  : %d DWORDS or %d Bytes\n", (unsigned int) iph->ihl,
            ((unsigned int) (iph->ihl)) * 4);
    fprintf(logfile, "   |-Type Of Service   : %d\n", (unsigned int) iph->tos);
    fprintf(logfile, "   |-IP Total Length   : %d  Bytes(size of Packet)\n", ntohs(iph->tot_len));
    fprintf(logfile, "   |-Identification    : %d\n", ntohs(iph->id));


    fprintf(logfile, "   |-TTL      : %d\n", (unsigned int) iph->ttl);
    fprintf(logfile, "   |-Protocol : %d\n", (unsigned int) iph->protocol);
    fprintf(logfile, "   |-Checksum : %d\n", ntohs(iph->check));
    fprintf(logfile, "   |-Source IP        : %s\n", inet_ntoa(source.sin_addr));
    fprintf(logfile, "   |-Destination IP   : %s\n", inet_ntoa(dest.sin_addr));

    switch (iph->protocol) {
        case IPPROTO_TCP:
            print_tcp_packet(buffer, size);
            break;
        case IPPROTO_UDP:
            print_udp_packet(buffer, size);
            break;
        case IPPROTO_ICMP:
            print_icmp_packet(buffer, size);
            break;
        default:
            print_data(buffer + iphdrlen, size - iphdrlen);

    }
}

void print_tcp_packet(unsigned char *buffer, int size) {
    unsigned short iphdrlen;

    struct iphdr *iph = (struct iphdr *) buffer;
    iphdrlen = iph->ihl * 4;
    struct tcphdr *tcph = (struct tcphdr *) (buffer + iphdrlen);

    fprintf(logfile, "\n");
    fprintf(logfile, "TCP Header\n");
    fprintf(logfile, "   |-Source Port      : %u\n", ntohs(tcph->source));
    fprintf(logfile, "   |-Destination Port : %u\n", ntohs(tcph->dest));
    fprintf(logfile, "   |-Sequence Number    : %u\n", ntohl(tcph->seq));
    fprintf(logfile, "   |-Acknowledge Number : %u\n", ntohl(tcph->ack_seq));
    fprintf(logfile, "   |-Header Length      : %d DWORDS or %d BYTES\n", (unsigned int) tcph->doff,
            (unsigned int) tcph->doff * 4);
    fprintf(logfile, "   |-Urgent Flag          : %d\n", (unsigned int) tcph->urg);
    fprintf(logfile, "   |-Acknowledgement Flag : %d\n", (unsigned int) tcph->ack);
    fprintf(logfile, "   |-Push Flag            : %d\n", (unsigned int) tcph->psh);
    fprintf(logfile, "   |-Reset Flag           : %d\n", (unsigned int) tcph->rst);
    fprintf(logfile, "   |-Synchronise Flag     : %d\n", (unsigned int) tcph->syn);
    fprintf(logfile, "   |-Finish Flag          : %d\n", (unsigned int) tcph->fin);
    fprintf(logfile, "   |-Window         : %d\n", ntohs(tcph->window));
    fprintf(logfile, "   |-Checksum       : %d\n", ntohs(tcph->check));
    fprintf(logfile, "   |-Urgent Pointer : %d\n", tcph->urg_ptr);
    fprintf(logfile, "\n");
    fprintf(logfile, "                        DATA Dump                         ");
    fprintf(logfile, "\n");
    fprintf(logfile, "Data Payload\n");
    print_data(buffer + iphdrlen + tcph->doff * 4, (size - tcph->doff * 4 - iph->ihl * 4));
}

void print_udp_packet(unsigned char *buffer, int size) {

    unsigned short iphdrlen;
    struct iphdr *iph = (struct iphdr *) buffer;
    iphdrlen = iph->ihl * 4;
    struct udphdr *udph = (struct udphdr *) (buffer + iphdrlen);

    fprintf(logfile, "\nUDP Header\n");
    fprintf(logfile, "   |-Source Port      : %d\n", ntohs(udph->source));
    fprintf(logfile, "   |-Destination Port : %d\n", ntohs(udph->dest));
    fprintf(logfile, "   |-UDP Length       : %d\n", ntohs(udph->len));
    fprintf(logfile, "   |-UDP Checksum     : %d\n", ntohs(udph->check));
    fprintf(logfile, "Data Payload\n");
    print_data(buffer + iphdrlen + sizeof udph, (size - sizeof udph - iph->ihl * 4));
}

void print_icmp_packet(unsigned char *buffer, int size) {
    unsigned short iphdrlen;

    struct iphdr *iph = (struct iphdr *) buffer;
    iphdrlen = iph->ihl * 4;

    struct icmphdr *icmph = (struct icmphdr *) (buffer + iphdrlen);

    fprintf(logfile, "\nICMP Header\n");
    fprintf(logfile, "   |-Type : %d", (unsigned int) (icmph->type));

    if ((unsigned int) (icmph->type) == 11)
        fprintf(logfile, "  (TTL Expired)\n");
    else if ((unsigned int) (icmph->type) == ICMP_ECHOREPLY)
        fprintf(logfile, "  (ICMP Echo Reply)\n");
    fprintf(logfile, "   |-Code : %d\n", (unsigned int) (icmph->code));
    fprintf(logfile, "   |-Checksum : %d\n", ntohs(icmph->checksum));
    fprintf(logfile, "\n");
    fprintf(logfile, "Data Payload\n");
    print_data(buffer + iphdrlen + sizeof icmph, (size - sizeof icmph - iph->ihl * 4));
}

void print_data(unsigned char *data, int Size) {

    for (i = 0; i < Size; i++) {
        if (i != 0 && i % 16 == 0) {
            fprintf(logfile, "         ");
            for (j = i - 16; j < i; j++) {
                if (data[j] >= 32 && data[j] <= 128)
                    fprintf(logfile, "%c", (unsigned char) data[j]);

                else fprintf(logfile, ".");
            }
            fprintf(logfile, "\n");
        }

        if (i % 16 == 0) fprintf(logfile, "   ");
        fprintf(logfile, " %02X", (unsigned int) data[i]);

        if (i == Size - 1) {
            for (j = 0; j < 15 - i % 16; j++) fprintf(logfile, "   ");

            fprintf(logfile, "         ");

            for (j = i - i % 16; j <= i; j++) {
                if (data[j] > 31 && data[j] < 128) fprintf(logfile, "%c", (unsigned char) data[j]);
                else fprintf(logfile, ".");
            }
            fprintf(logfile, "\n");
        }
    }
}