/*
Florenc Caminade
Thomas FLayols
Etienne Arlaud

Receive raw 802.11 packet and filter ESP-NOW vendor specific action frame using BPF filters.
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
#include <linux/filter.h>

#define PACKET_LENGTH 400 //Approximate
#define MYDATA 18         //0x12
#define MAX_PACKET_LEN 1000

/*our MAC address*/
//{0xF8, 0x1A, 0x67, 0xB7, 0xeB, 0x0B};

/*ESP8266 host MAC address*/
//{0x84,0xF3,0xEB,0x73,0x55,0x0D};

#define FILTER_LENGTH 11

//filter action frame packets
static struct sock_filter bpfcode[FILTER_LENGTH] = {
  { 0x30, 0, 0, 0x00000003 }, // ldb [3]      //radiotap header length : MS byte
  { 0x64, 0, 0, 0x00000008 },	// lsh #8       //left shift it
  { 0x7, 0, 0, 0x00000000 },  // tax          //'store' it in X register
  { 0x30, 0, 0, 0x00000002 },	// ldb [2]      //radiotap header length : LS byte
  { 0x4c, 0, 0, 0x00000000 },	// or x         //combine A & X to get radiotap header length in A
  { 0x7, 0, 0, 0x00000000 },	// tax          //'store' it in X
  { 0x50, 0, 0, 0x00000000 },	// ldb [x + 0]  //right after radiotap header is the type and subtype
  { 0x54, 0, 0, 0x000000fc },	// and #0xfc    //mask the interesting bits, a.k.a 0b1111 1100
  { 0x15, 0, 1, 0x000000d0 },	// jeq #0xd0 jt9 jf10 //compare the types and subtypes
  { 0x6, 0, 0, 0x00040000 },	// ret #262144  //return 'True'
  { 0x6, 0, 0, 0x00000000 },	// ret #0       //return 'False'
};

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

int create_raw_socket(char *dev, struct sock_fprog *bpf)
{
    struct sockaddr_ll sll;
    struct ifreq ifr;
    int fd, ifi, rb, attach_filter;

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

    attach_filter = setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, bpf, sizeof(*bpf));
    assert(attach_filter != -1);

    return fd;
}

int main(int argc, char **argv)
{
    uint8_t buff[MAX_PACKET_LEN] = {0};
    int sock_fd;
    char *dev = argv[1];
    struct sock_fprog bpf = {FILTER_LENGTH, bpfcode};

    sock_fd = create_raw_socket(dev, &bpf); /* Creating the raw socket */

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
