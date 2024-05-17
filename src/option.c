#include <ft_ping.h>

options getOptions(int argc, char **argv, char *flags) {

    options ret = 0;
    int opt;

    while ((opt = getopt(argc, argv, flags)) != -1) {
        switch(opt) {
            case 'v': 
                ret |= VERBOSE;
                break;
            case '?':
                if (optopt == 0) {
                    ret |= HELP;
                    break;
                }
                opterr = 2;
                return (1);
        }
    }
    return (ret);
}