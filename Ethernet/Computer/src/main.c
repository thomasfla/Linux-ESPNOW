/*
	Simple udp server
*/
#include<stdio.h>	//printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>

#include <assert.h>


#include <sys/ioctl.h>
#include <linux/if_arp.h>

#define BUFLEN 512	//Max length of buffer
#define PORT_LISTEN 1111	//The port on which to listen for incoming data

#define DEST_ADDR "192.168.142.2" //"192.168.142.255"
#define PORT_DEST 2222
struct sockaddr_in dest_addr;


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
	
	bzero(&s_dest_addr, sizeof(s_dest_addr));
    bzero(&ifr, sizeof(ifr));

	int s, i, recv_len;
	char buf[BUFLEN];
	
	//create a UDP socket
	s=socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if(s == -1)
	{
		die("socket");
	}
	

	strncpy((char *)ifr.ifr_name, "enp4s2f1", IFNAMSIZ); //interface
	assert(ioctl(s, SIOCGIFINDEX, &ifr)>=0);
	
	s_dest_addr.sll_family = PF_PACKET;
    s_dest_addr.sll_protocol = htons(ETH_P_ALL);
    s_dest_addr.sll_ifindex = ifr.ifr_ifindex;


    assert(bind(s, (struct sockaddr *)&s_dest_addr, sizeof(s_dest_addr))>=0);

    eth_frame my_frame;

	while(1)
	{
		printf("Waiting for data...");
		fflush(stdout);
		
		//try to receive some data, this is a blocking call
		bzero(my_frame.data, 127);
		if ((recv_len = recvfrom(s, &my_frame, BUFLEN, 0, NULL, 0)) == -1)
		{
			die("recvfrom()");
		}
		
		//print details of the client/peer and the data received
		//printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Data:");
		for(int i=0;i<my_frame.data_len;i++) {
			printf("%c", my_frame.data[i]);
		}
		printf("\n");
		
		uint8_t temp_mac[6];

		memcpy(temp_mac, my_frame.dst_mac, sizeof(uint8_t) * 6);
		memcpy(my_frame.dst_mac, my_frame.src_mac, sizeof(uint8_t) * 6);
		memcpy(my_frame.src_mac, temp_mac, sizeof(uint8_t) * 6);
		
		if (sendto(s, &my_frame, 127, 0, NULL, 0) == -1)
		{
			die("sendto()");
		}
	}

	close(s);
	return 0;
}