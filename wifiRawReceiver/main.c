/*
Florenc Caminade
Thomas FLayols

Receive raw 802.11 packet including ESP-NOW vendor specific action frame.
https://hackaday.io/project/161896
https://github.com/thomasfla/Linux-ESPNOW

Adapted from : 
https://stackoverflow.com/questions/10824827/raw-sockets-communication-over-wifi-receiver-not-able-to-receive-packets

1/Find your wifi interface:
$ iwconfig

2/Setup your interface in monitor mode :
$ sudo ifconfig wlp5s0 down
$ sudo iwconfig wlp5s0 mode monitor
$ sudo ifconfig wlp5s0 up

3/Run this code as root 
*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <assert.h>

#define PACKET_LENGTH 400 //Approximate
#define MYDATA 18         //0x12
#define MAX_PACKET_LEN 1000
/*our MAC address*/
//{0xF8, 0x1A, 0x67, 0xB7, 0xeB, 0x0B};

/*ESP8266 host MAC address*/
//{0x84,0xF3,0xEB,0x73,0x55,0x0D};

void print_packet(uint8_t *data, int len)
{
    printf("----------------------------new packet-----------------------------------\n");
    int i;
    for (i = 0; i < len; i++)
    {
        if (i % 16 == 0)
            printf("\n");
        printf("0x%02x, ", data[i]);
    }
    printf("\n\n");
}

int create_raw_socket(char *dev)
{
    struct sockaddr_ll sll;
    struct ifreq ifr;
    int fd, ifi, rb;

    bzero(&sll, sizeof(sll));
    bzero(&ifr, sizeof(ifr));

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    assert(fd != -1);

    strncpy((char *)ifr.ifr_name, dev, IFNAMSIZ);
    ifi = ioctl(fd, SIOCGIFINDEX, &ifr);
    assert(ifi != -1);

    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_family = PF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_pkttype = PACKET_OTHERHOST;

    rb = bind(fd, (struct sockaddr *)&sll, sizeof(sll));
    assert(rb != -1);

    return fd;
}

int main(int argc, char **argv)
{
    uint8_t buff[MAX_PACKET_LEN] = {0};
    int sock_fd;
    char *dev = argv[1];

    sock_fd = create_raw_socket(dev); /* Creating the raw socket */

    printf("\n Waiting to receive packets ........ \n");

    while (1)
    {
        int len = recvfrom(sock_fd, buff, MAX_PACKET_LEN, MSG_TRUNC, NULL, 0);

        if (len < 0)
        {
            perror("Socket receive failed or error");
            break;
        }
        else
        {
            printf("len:%d\n", len);
            print_packet(buff, len);
        }
    }
    close(sock_fd);
    return 0;
}
