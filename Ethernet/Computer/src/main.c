/*
	Simple udp server
*/
#include<stdio.h>	//printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include <unistd.h> //close(fd);
#include<arpa/inet.h>
#include<sys/socket.h>

#include <assert.h>

#include <sys/ioctl.h>
#include <linux/if_arp.h>

uint8_t broadcast_addr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct {
  uint8_t dst_mac[6];
  uint8_t src_mac[6];
  uint16_t ethertype;

  /* Custom payload*/
  uint16_t data_len;
  char data[127];

} eth_frame;


void die(char *s)
{
	perror(s);
	exit(1);
}

int main(void)
{
	struct sockaddr_ll s_dest_addr;
    struct ifreq ifr;
	
	memset(&s_dest_addr, 0, sizeof(s_dest_addr));
    memset(&ifr, 0, sizeof(ifr));

	int fd, recv_len;
	
	//create a RAW socket
	fd=socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if(fd == -1)
	{
		die("socket");
	}

	strncpy((char *)ifr.ifr_name, "enp4s2f1", IFNAMSIZ); //interface
	assert(ioctl(fd, SIOCGIFINDEX, &ifr)>=0);
	
	s_dest_addr.sll_family = PF_PACKET;
    s_dest_addr.sll_protocol = htons(ETH_P_ALL);
    s_dest_addr.sll_ifindex = ifr.ifr_ifindex;

    assert(bind(fd, (struct sockaddr *)&s_dest_addr, sizeof(s_dest_addr))>=0);


    eth_frame my_frame;

	while(1)
	{
		printf("Waiting for data...");
		fflush(stdout);
		
		//try to receive some data, this is a blocking call
		memset(my_frame.data, 0, 127);
		if ((recv_len = recvfrom(fd, &my_frame, sizeof(my_frame), 0, NULL, 0)) == -1)
		{
			die("recvfrom()");
		}
		
		//print details of the data received
		printf("Data:");
		for(int i=0;i<my_frame.data_len;i++) {
			printf("%c", my_frame.data[i]);
		}
		printf("\n");
		
		uint8_t temp_mac[6];

		memcpy(temp_mac, my_frame.dst_mac, sizeof(uint8_t) * 6);
		memcpy(my_frame.dst_mac, broadcast_addr, sizeof(uint8_t) * 6);
		//memcpy(my_frame.dst_mac, my_frame.src_mac, sizeof(uint8_t) * 6);
		memcpy(my_frame.src_mac, temp_mac, sizeof(uint8_t) * 6);
		
		if (sendto(fd, &my_frame, 127, 0, NULL, 0) == -1)
		{
			die("sendto()");
		}
	}

	close(fd);
	return 0;
}