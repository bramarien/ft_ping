#include <ft_ping.h>

struct options getOptions(int argc, char **argv, char *flags) {

    struct options option;
    option.flags = 0;
    option.ttl = DEFAULT_TTL;
    int opt;

    while ((opt = getopt(argc, argv, flags)) != -1) {
        switch(opt) {
            case 't':
                option.flags |= TTL;
                option.ttl = atoi(optarg);
                break;
            case 'v': 
                option.flags |= VERBOSE;
                break;
            case '?':
                if (optopt == 0) {
                    option.flags |= HELP;
                    break;
                }
                opterr = 2;
                return (option);
        }
    }
    return (option);
}