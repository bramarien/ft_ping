#include <ft_ping.h>

uint16_t calculate_icmp_checksum(struct icmp_packet *icmp, int len) {
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *)icmp;
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

int main(int argc, char **argv)
{
    int errcode;
    struct addrinfo *dest_adr = NULL;
    struct protoent *sock_proto = NULL;
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

        errcode = getaddrinfo(argv[optind], NULL, NULL, &dest_adr);
        if(errcode != 0) {
            printf("getaddrinfo: %s\n", gai_strerror(errcode));
            return 1;
        }
        printf("-------------------------------\n");
        printf("        Addr Info:\n");
        printf("Address is : \"%s\"\n", inet_ntop(AF_INET, &((struct sockaddr_in *)dest_adr->ai_addr)->sin_addr, buf, sizeof(buf)));
        printf("-------------------------------\n");


        int sock = socket(AF_INET, SOCK_RAW, sock_proto->p_proto);
        if (sock == -1) {
            printf("sock broken\n");
            return 1;
        }

        struct icmp_packet packet;
        u_int16_t identification = 0xff;
        u_int16_t seq_number = 0;

        packet.icmp_hdr.type = 8;
        packet.icmp_hdr.code = 15;
        packet.icmp_hdr.checksum = 0;
        bzero(packet.data, sizeof(packet.data));

        packet.icmp_hdr.checksum = calculate_icmp_checksum(&packet, sizeof(struct icmp_packet));

        printf("Checks = %x\n", packet.icmp_hdr.checksum);
        
        char buf[sizeof(struct icmp_packet)];
        memcpy(buf, &packet.icmp_hdr, sizeof(packet.icmp_hdr));
        memcpy(buf + sizeof(struct icmphdr), &packet.data, sizeof(packet.data));
        
        int err = sendto(sock, &buf, sizeof(packet), 0, dest_adr->ai_addr, INET_ADDRSTRLEN);

        char buffer[1024];
        int bytes = recvfrom(sock, &buffer, sizeof(buffer), 0, dest_adr->ai_addr, &dest_adr->ai_addrlen);

        printf("bytes received = %d\n", bytes);

        struct ip *recv_ip_header = (struct ip *)buffer;
        struct icmp *recv_icmp_header = (struct icmp *)(buffer + (recv_ip_header->ip_hl * 4));

        printf("Len is received = %d\n", recv_ip_header->ip_hl);
        optind++;
    }
    
    errcode = flags;
    return ((opterr == 2) ? 1 : 0);
}