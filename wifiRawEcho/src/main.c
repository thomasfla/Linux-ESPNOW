/*
Etienne Arlaud
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
#include <sys/time.h>   

#include "packets.h"

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


static uint8_t src_mac[6] = {0xF8, 0x1A, 0x67, 0xb7, 0xEB, 0x0B};
static uint8_t dest_mac[6] = {0x84, 0xf3, 0xeb, 0x73, 0x55, 0xed};


int create_raw_socket(char *dev)
{
    struct sockaddr_ll s_dest_addr; //code from sender
    struct ifreq ifr;
    int fd, ifi, rb;

    bzero(&s_dest_addr, sizeof(s_dest_addr));
    bzero(&ifr, sizeof(ifr));

    (void)memset(&s_dest_addr, 0, sizeof(s_dest_addr));

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    assert(fd != -1); //abort if error

    strncpy((char *)ifr.ifr_name, dev, IFNAMSIZ);
    ifi = ioctl(fd, SIOCGIFINDEX, &ifr);
    assert(ifi != -1); //abort if error

    s_dest_addr.sll_family = PF_PACKET;
    //we don't use a protocol above ethernet layer, just use anything here
    s_dest_addr.sll_protocol = htons(ETH_P_ALL);
    s_dest_addr.sll_ifindex = ifr.ifr_ifindex;
    s_dest_addr.sll_hatype = ARPHRD_ETHER;
    s_dest_addr.sll_pkttype = PACKET_OTHERHOST; //PACKET_OUTGOING
    s_dest_addr.sll_halen = ETH_ALEN;
    //MAC - begin
    s_dest_addr.sll_addr[0] = dest_mac[0];
    s_dest_addr.sll_addr[1] = dest_mac[1];
    s_dest_addr.sll_addr[2] = dest_mac[2];
    s_dest_addr.sll_addr[3] = dest_mac[3];
    s_dest_addr.sll_addr[4] = dest_mac[4];
    s_dest_addr.sll_addr[5] = dest_mac[5];
    //MAC - end
    s_dest_addr.sll_addr[6] = 0x00; //not used
    s_dest_addr.sll_addr[7] = 0x00; //not used

    rb = bind(fd, (struct sockaddr *)&s_dest_addr, sizeof(s_dest_addr));
    assert(rb != -1); //abort if error

    return fd;
}

int32_t send_echo(int sock_fd, uint8_t *data, int len) {
      int32_t s32_res;

      s32_res = write(sock_fd,data,len);

      return s32_res;
}


int main(int argc, char **argv)
{
	assert(argc == 2);

    char *dev = argv[1];

    int sock_fd = -1;
    int s32_res = -1;

    sock_fd = create_raw_socket(dev); /* Creating the raw socket */

    if (-1 == sock_fd)
    {
        perror("Could not create the socket");
        goto LABEL_CLEAN_EXIT;
    }

    printf("Socket created\n");

    fflush(stdout);

    sleep(1);

    int Counter = 0;
	ESPNOW_packet my_packet;
	init_ESPNOW_packet(&my_packet);
	memcpy(my_packet.wlan.da, dest_mac, sizeof(uint8_t)*6);
	memcpy(my_packet.wlan.sa, src_mac, sizeof(uint8_t)*6);
	memcpy(my_packet.wlan.bssid, dest_mac, sizeof(uint8_t)*6);
	uint8_t raw_bytes[400];
	int len = packet_to_bytes(raw_bytes, 400, my_packet);

    while (1)
    {

            printf("Send : %d\n", Counter++);
            s32_res = send_echo(sock_fd, raw_bytes, len);
			print_packet(raw_bytes, len);

            if (-1 == s32_res)
            {
                perror("Socket send failed");
                goto LABEL_CLEAN_EXIT;
            } else {
              printf("Echo sent\n");
            }
		sleep(1);
    }

LABEL_CLEAN_EXIT:
    if (sock_fd > 0)
    {
        close(sock_fd);
    }

    printf("***** Raw Socket test- end\n");

    return EXIT_SUCCESS;
    
	
}
