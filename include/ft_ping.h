#pragma once

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

typedef enum {
    VERBOSE = 1 << 0,
    HELP = 1 << 1,
} options;

void printHelp();
options getOptions(int argc, char **argv, char *flags);