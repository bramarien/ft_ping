#pragma once

#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <linux/icmp.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


typedef enum {
    VERBOSE = 1 << 0,
    HELP = 1 << 1,
} options;

struct icmp_packet {
    struct icmphdr icmp_hdr;
    char data[48];
};

void printHelp();
options getOptions(int argc, char **argv, char *flags);