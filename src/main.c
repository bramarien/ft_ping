#include <ft_ping.h>
#include <ifaddrs.h>

int signal_handler = 1;

void handle_c() {
    signal_handler = 0;
    printf("\n");
}

struct sockaddr_in *get_src_address(struct ifaddrs *ifaddr, char *interface) {
    
    struct sockaddr_in *ret = NULL;
    
    getifaddrs(&ifaddr);
    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa-> ifa_next) {
        if (strcmp(interface, ifa->ifa_name) == 0 && ifa->ifa_addr->sa_family == AF_INET) {
            ret = (struct sockaddr_in *)ifa->ifa_addr;
            break;
        }
    }
    return ret;
}

uint16_t calculate_icmp_checksum(uint16_t *packet, int len) {
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *)packet;
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    if (len)
        sum += *(uint8_t *)ptr;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return (uint16_t)~sum;
}

int verify_checksum(struct icmp_packet *packet) {

    uint16_t old_sum = packet->icmp_hdr.checksum;
    packet->icmp_hdr.checksum = 0;
    // uint16_t sum = calculate_icmp_checksum(packet, sizeof(struct icmp_packet ));

    return old_sum == 0;
}

int main(int argc, char **argv)
{
    int errcode;
    struct addrinfo *dest_adr = NULL;
    struct protoent *sock_proto = NULL;
    char adr_buf[INET_ADDRSTRLEN];

    struct options option = getOptions(argc, argv, "t:v?");   
    sock_proto = getprotobyname("icmp");

    if (argv[optind] == NULL && opterr != 2) {
        opterr = 2;
        dprintf(2, "ping: missing host operand\n");
    }
    if (opterr == 2) 
        dprintf(2, "Try 'ping -?' for more information.\n");
    if (sock_proto == NULL)
        dprintf(2, "Proto icmp not found please check your protocol file located at /etc/protocols\n");

    signal(SIGINT, handle_c);

    while (argv[optind] != NULL) {
        signal_handler = 1;
        errcode = getaddrinfo(argv[optind], NULL, NULL, &dest_adr);
        if(errcode != 0) {
            printf("getaddrinfo: %s\n", gai_strerror(errcode));
            return 1;
        }
        printf("PING %s (%s): %d data bytes\n", argv[optind], inet_ntop(AF_INET, &((struct sockaddr_in *)dest_adr->ai_addr)->sin_addr, adr_buf, sizeof(adr_buf)), sizeof(struct icmp_packet));

        struct timeval tv;
        int one = 1;
        char *interface = DEFAULT_INTERFACE;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int sock_fd = socket(AF_INET, SOCK_RAW, sock_proto->p_proto);

        struct ifaddrs *ifaddr;        
        getifaddrs(&ifaddr);

        struct sockaddr_in *src_addr = get_src_address(ifaddr, interface);
        if (src_addr == NULL) {
            printf("error with interface");
            freeifaddrs(ifaddr);
        }
        struct sockaddr_in *dst_addr = malloc(sizeof(dst_addr));
        memcpy(dst_addr, (struct sockaddr_in *)dest_adr->ai_addr, sizeof(*dest_adr->ai_addr));
        
        if (sock_fd == -1
        || setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0
        || setsockopt(sock_fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) != 0 ) {
            printf("sock broken\n");
            return 1;
        }

        uint8_t sequence = 0;
        double accum = 0,
            minimum = 0,
            maximum = 0,
            average = 0,
            stddev = 0;
        int nb_recv = 0;

        while (signal_handler) {
            struct icmp_packet packet;
            packet.ip_hdr.ip_hl = 5;
            packet.ip_hdr.ip_v = 4;
            packet.ip_hdr.ip_tos = 0;
            packet.ip_hdr.ip_len = sizeof(packet.ip_hdr) + sizeof(packet.icmp_hdr);
            packet.ip_hdr.ip_id = getpid();
            packet.ip_hdr.ip_off = 0;
            packet.ip_hdr.ip_ttl = option.ttl;
            packet.ip_hdr.ip_p = IPPROTO_ICMP;
            packet.ip_hdr.ip_src = src_addr->sin_addr;
            packet.ip_hdr.ip_dst = dst_addr->sin_addr;
            packet.ip_hdr.ip_sum = 0;
            packet.ip_hdr.ip_sum = calculate_icmp_checksum((uint16_t *)&packet.ip_hdr, sizeof(struct ip));

            packet.icmp_hdr.type = 8;
            packet.icmp_hdr.code = 0;
            packet.icmp_hdr.checksum = 0;
            packet.icmp_hdr.un.echo.id = getpid();
            packet.icmp_hdr.un.echo.sequence = htons(sequence);
            sequence+=1;
            bzero(packet.data, sizeof(packet.data));
            packet.icmp_hdr.checksum = calculate_icmp_checksum((uint16_t *)&packet.icmp_hdr, sizeof(struct icmphdr) + sizeof(packet.data));

            char buf[sizeof(struct icmp_packet)];

            memcpy(buf, &packet.ip_hdr, sizeof(packet.ip_hdr));
            memcpy(buf + sizeof(struct ip), &packet.icmp_hdr, sizeof(packet.icmp_hdr));
            memcpy(buf + sizeof(struct ip) + sizeof(struct icmphdr), &packet.data, sizeof(packet.data));

            // printf("Address is : \"%s\"\n", inet_ntop(AF_INET, &((struct sockaddr_in *)dest_adr->ai_addr)->sin_addr, adr_buf, sizeof(adr_buf)));
            sleep(SLEEP_SEND);
            int err = sendto(sock_fd, &buf, sizeof(packet), 0, dest_adr->ai_addr, INET_ADDRSTRLEN);
            struct timeval start, stop;
            gettimeofday(&start, NULL);

            int timeout = 1;
            while (timeout && signal_handler) {
                char buffer[1024];
                struct icmp_packet recv_packet;
                int bytes = recvfrom(sock_fd, &recv_packet, sizeof(recv_packet), 0, dest_adr->ai_addr, &dest_adr->ai_addrlen);
                if (errno == EAGAIN) {
                    timeout = 0;
                }
                if ( bytes >= 1 && signal_handler ) {
                    switch ( recv_packet.icmp_hdr.type ) {
                        case 0:
                            if (!verify_checksum(&recv_packet) || packet.icmp_hdr.un.echo.id != recv_packet.icmp_hdr.un.echo.id ) {
                                break;
                            }
                            gettimeofday(&stop, NULL);
                            accum = (( stop.tv_sec - start.tv_sec ) * 1000000) + (( stop.tv_usec - start.tv_usec ));
                            if (accum > maximum) 
                                maximum = accum;
                            if (minimum == 0 || accum < minimum)
                                minimum = accum;
                            average += accum;
                            nb_recv++;
                            printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3lf ms\n", bytes - sizeof(struct ip), inet_ntop(AF_INET, &recv_packet.ip_hdr.ip_src, buf, sizeof(buf)), ntohs(recv_packet.icmp_hdr.un.echo.sequence), recv_packet.ip_hdr.ip_ttl , accum / 1000);
                            timeout = 0;
                            break;
                        case 3:
                            switch ( recv_packet.icmp_hdr.code ) {
                                case 0: printf("Network Unreachable\n"); break;
                                case 1: printf("Host Unreachable\n"); break;
                                case 2: printf("Protocol Unreachable\n"); break;
                                case 3: printf("Port Unreachable\n"); break;
                                case 4: printf("Fragmentation Needed and Don't Fragment was Set\n"); break;
                                case 5: printf("Source Route Failed\n"); break;
                                case 6: printf("Destination Network Unknown\n"); break;
                                case 7: printf("Destination Host Unknown\n"); break;
                                case 8: printf("Source Host Isolated\n"); break;
                                case 9: printf("Communication with Destination Network is Administratively Prohibited\n"); break;
                                case 10: printf("Communication with Destination Host is Administratively Prohibited\n"); break;
                                case 11: printf("Network Unreachable for Type of Service\n"); break;
                                case 12: printf("Host Unreachable for Type of Service\n"); break;
                            }
                            break;
                        case 11:
                            if (option.flags & VERBOSE) {
                                printf("IP Hdr Dump:\n");
                                // void *bytes = (void *)recv_packet.ip_hdr;
                                // for (int i = 0; i < sizeof(recv_packet.ip_hdr); i++) {
                                //     printf(" %02x ", bytes[i]);
                                // }
                                printf("\n");
                            }
                            printf("%d bytes from %s: Time to live exceeded\n", bytes, inet_ntop(AF_INET, &recv_packet.ip_hdr.ip_src, buf, sizeof(buf)));
                            break;
                        case 14:
                            printf("Timestamp reply received\n");
                            break;
                        default:
                            printf("%d here\n", recv_packet.icmp_hdr.type);
                    }
                }
            }
        }
        printf("Min: %.3lf | Max: %.3lf | Average: %.3lf | Stddev: %.3lf \n", minimum/1000, maximum/1000, ((average / nb_recv)/1000), ((maximum - minimum)/1000));
        close(sock_fd);
        optind++;
    }

    return ((opterr == 2) ? 1 : 0);
}



// printf(":\n");
// printf("IP Hdr Dump:\n");
// printf("IP Hdr Dump:\n");



// IP Hdr Dump:
//  4500 0054 7c33 4000 0401 3f7e 0a00 020f d83a d6ae 
// Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src	Dst	Data
//  4  5  00 0054 7c33   2 0000  04  01 3f7e 10.0.2.15  216.58.214.174 
// ICMP: type 8, code 0, size 64, id 0x35b9, seq 0x0001
// 76 bytes from ge103-0-0-25.er01.lyo02.jaguar-network.net (78.153.228.172): Time to live exceeded