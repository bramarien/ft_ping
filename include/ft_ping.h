#pragma once

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/icmp.h>
#include <netdb.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

#define TIMEOUT_RECV 1
#define SLEEP_SEND 1
#define DEFAULT_TTL 64
#define DEFAULT_INTERFACE "eth0"

enum flags {
    VERBOSE = 1 << 0,
    HELP = 1 << 1,
    TTL = 1 << 2,
};

struct options {
    enum flags flags;
    int ttl;
};

struct icmp_packet {
    struct ip ip_hdr;
    struct icmphdr icmp_hdr;
    char data[56];
};

void printHelp();
struct options getOptions(int argc, char **argv, char *flags);