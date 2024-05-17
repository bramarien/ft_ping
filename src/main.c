#include <ft_ping.h>

int main(int argc, char **argv)
{
    struct addrinfo *res;
    struct protoent *sock_proto;
    int errcode;
    char buf[INET_ADDRSTRLEN];

    options flags = getOptions(argc, argv, "v?");   
    sock_proto = getprotobyname("icmp");

    if (argv[optind] == NULL && opterr != 2) {
        opterr = 2;
        dprintf(2, "ping: missing host operand\n");
    }
    if (opterr == 2) 
        dprintf(2, "Try 'ping -?' for more information.\n");
    if (sock_proto == NULL)
        dprintf(2, "Proto icmp not found please check your protocol file located at /etc/protocols\n");
    
    while (argv[optind] != NULL) {
        errcode = getaddrinfo(argv[optind], NULL, NULL, &res);
        if(errcode != 0) {
            printf("getaddrinfo: %s\n", gai_strerror(errcode));
            return 1;
        }
        printf("-------------------------------\n");
        printf("        Addr Info:\n");
        printf("Address is : \"%s\"\n", inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr, buf, sizeof(buf)));
        printf("ai_flags = %d\n", res->ai_flags);
        printf("ai_family = %d\n", res->ai_family);
        printf("ai_socktype = %d\n", res->ai_socktype);
        printf("ai_protocol = %d\n", res->ai_protocol);
        printf("ai_canonname = %s\n", res->ai_canonname);
        printf("-------------------------------\n");


        int sock = socket(AF_INET, SOCK_RAW, sock_proto->p_proto);
        printf("sock = %d\n", sock);
        
        
        optind++;
    }
    
    errcode = flags;
    return ((opterr == 2) ? 1 : 0);
}