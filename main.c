#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

//z wykladu
void print_as_bytes (unsigned char* buff, ssize_t length)
{
    for (ssize_t i = 0; i < length; i++, buff++)
        printf ("%.2x ", *buff);
}

u_int16_t compute_icmp_checksum (const void *buff, int length)
{
    u_int32_t sum;
    const u_int16_t* ptr = buff;
    assert (length % 2 == 0);
    for (sum = 0; length > 0; length -= 2)
        sum += *ptr++;
    sum = (sum >> 16) + (sum & 0xffff);
    return (u_int16_t)(~(sum + (sum >> 16)));
}


int czekaj_na_pakiet(int sockfd)
{
    fd_set descriptors;
    FD_ZERO (&descriptors);
    FD_SET (sockfd, &descriptors);
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int ready = select (sockfd+1, &descriptors, NULL, NULL, &tv);

    if(ready < 0)
    {
        printf("wystąpil błąd");
        return EXIT_FAILURE;
    }
    else if(ready == 0)
    {
        printf("nastąpił timeout");
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2)//
    {
        printf("brak argumentow");
        return EXIT_FAILURE;
    }


    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &recipient.sin_addr);


    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }


    //wysylanie

    int nr = 1;
    for (int ttl = 1; ttl <= 30; ttl++)//
    {
        for (int pakiet = 1; pakiet <= 3; pakiet++) {
            struct icmp header;

            header.icmp_type = ICMP_ECHO;
            header.icmp_code = 0;
            header.icmp_hun.ih_idseq.icd_id = getpid();//
            header.icmp_hun.ih_idseq.icd_seq = ttl * 10 + pakiet;//
            header.icmp_cksum = 0;
            header.icmp_cksum = compute_icmp_checksum((u_int16_t *) &header, sizeof(header));

            setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));

            ssize_t bytes_sent = sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr *) &recipient,
                                        sizeof(recipient));

            if (bytes_sent < 0) {
                printf("Błąd przy wysyłaniu pakietów");
                return EXIT_FAILURE;
            }

            printf("%d\n", bytes_sent);
        }

    }

        /*      // czekanie na pakiet i odbierz


              int ile_doszlo_pakietow = 0;
              char adresy [3][20];

              for(int pakiet = 1; pakiet <= 3; pakiet++) {
                  char sender_ip_str[20];
                  inet_ntop(AF_INET, &(recipient.sin_addr), adresy[pakiet - 1], sizeof(sender_ip_str));

                  int oczekiwanie = czekaj_na_pakiet(sockfd);

                  if (oczekiwanie == 1) {
                      ile_doszlo_pakietow ++;

                      //zczytaj
                  }

                  else {
                      break;
                  }
              }

              //jezeli poprawnie zczytalo
              if()
              {
                  // wypisywanie

                  printf("%d. ", nr);
                  nr ++;

                  //wypisz unikatowe adresy
                  for(int pakiet = 1; pakiet <= 3; pakiet++) {
                      // czy poprawny adres

                      if(strcmp(adresy[0], adresy[1]) != 0 && strcmp(adresy[0], adresy[2]) != 0)
                          printf("%s ", adresy[0] );
                      if(strcmp(adresy[0], adresy[1]) != 0 && strcmp(adresy[1], adresy[2]) != 0)
                          printf("%s ", adresy[1] );
                      if(strcmp(adresy[0], adresy[2]) != 0 && strcmp(adresy[1], adresy[2]) != 0)
                          printf("%s ", adresy[2] );
                  }


                  if(ile_doszlo_pakietow == 0) {
                      printf("*\n");
                  }
                  else if (ile_doszlo_pakietow < 3) {
                      printf("???\n");
                  }
                  else {
                      printf("%dms\n", 1000);//test
                  }
              }

              //jezeli mamy cel osiagniety to zakoncz i przerwij

          }
      }


      */

        for (;;) {

            struct sockaddr_in sender;
            socklen_t sender_len = sizeof(sender);
            u_int8_t buffer[IP_MAXPACKET];

            ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *) &sender, &sender_len);
            if (packet_len < 0) {
                fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
                return EXIT_FAILURE;
            }

            char sender_ip_str[20];
            inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));
            printf("Received IP packet with ICMP content from: %s\n", sender_ip_str);

            struct ip *ip_header = (struct ip *) buffer;
            ssize_t ip_header_len = 4 * ip_header->ip_hl;

            printf("IP header: ");
            print_as_bytes(buffer, ip_header_len);
            printf("\n");

            printf("IP data:   ");
            print_as_bytes(buffer + ip_header_len, packet_len - ip_header_len);
            printf("\n\n");
        }

        return EXIT_SUCCESS;
    }



