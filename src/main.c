#include <ft_ping.h>

int main(int argc, char **argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "v?")) != -1) {
        printf("opt => %d\n", opt);
    }
    return (0);
}