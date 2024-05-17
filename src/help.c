#include <ft_ping.h>

void printHelp() {
    printf(
    "Usage: ./ft_ping [OPTION...] HOST ...\n"
    "Send ICMP ECHO_REQUEST packets to network hosts.\n"
    "\n"
    "Options valid for all request types:\n"
    "\n"
    "-d, --debug                set the SO_DEBUG option\n"
    "-i, --interval=NUMBER      wait NUMBER seconds between sending each packet\n"
    "-r, --ignore-routing       send directly to a host on an attached network\n"
    "    --ttl=N                specify N as time-to-live\n"
    "-v, --verbose              verbose output\n"
    "-w, --timeout=N            stop after N seconds\n"
    "-?, --help                 give this help list\n"
    "    --usage                give a short usage message\n"
    "\n"
    "\n"
    "Report bugs to <github.com/bramarien/ft_ping.git>.\n");
    return;
}