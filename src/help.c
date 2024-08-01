#include <ft_ping.h>

void printHelp() {
    printf(
    "Usage: ./ft_ping [OPTION...] HOST ...\n"
    "Send ICMP ECHO_REQUEST packets to network hosts.\n"
    "\n"
    "Options valid for all request types:\n"
    "\n"
    "-i, --interval=NUMBER      wait NUMBER seconds between sending each packet\n"
    "-t                         specify N as time-to-live\n"
    "-v, --verbose              verbose output\n"
    "-?, --help                 give this help list\n"
    "\n"
    "\n"
    "Report bugs to <github.com/bramarien/ft_ping.git>.\n");
    return;
}