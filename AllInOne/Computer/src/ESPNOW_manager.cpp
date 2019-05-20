#include <stdio.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#include "ESPNOW_manager.h"


#define MAC_2_MSBytes(MAC)  MAC == NULL ? 0 : (MAC[0] << 8) | MAC[1]
#define MAC_4_LSBytes(MAC)  MAC == NULL ? 0 : (((((MAC[2] << 8) | MAC[3]) << 8) | MAC[4]) << 8) | MAC[5]


void ESPNOW_manager::unset_filter() {
	if(this->bpf.filter != NULL) {
		free(this->bpf.filter);
		this->bpf.filter = NULL;
	}
	this->bpf.len = 0;
}

void ESPNOW_manager::set_filter(uint8_t *src_mac, uint8_t *dst_mac) {
	//sudo tcpdump -i wlp5s0 'type 0 subtype 0xd0 and wlan[24:4]=0x7f18fe34 and wlan[32]=221 and wlan[33:4]&0xffffff = 0x18fe34 and wlan[37]=0x4 and wlan dst 11:22:33:44:55:66 and wlan src 77:88:99:aa:bb:cc' -dd
	unset_filter();

	this->bpf.len = 53;

	uint32_t MSB_dst = MAC_2_MSBytes(dst_mac);
	uint32_t LSB_dst = MAC_4_LSBytes(dst_mac);

	uint32_t MSB_src = MAC_2_MSBytes(src_mac);
	uint32_t LSB_src = MAC_4_LSBytes(src_mac);

	uint8_t jeq_dst = dst_mac == NULL ? 0x30 : 0x15; //0x30 jump if >=. 0x15 jump if ==.
	uint8_t jeq_src = src_mac == NULL ? 0x30 : 0x15;

	struct sock_filter temp_code[this->bpf.len] = {
			{ 0x30, 0, 0, 0x00000003 },
			{ 0x64, 0, 0, 0x00000008 },
			{ 0x7, 0, 0, 0x00000000 },
			{ 0x30, 0, 0, 0x00000002 },
			{ 0x4c, 0, 0, 0x00000000 },
			{ 0x2, 0, 0, 0x00000000 },
			{ 0x7, 0, 0, 0x00000000 },
			{ 0x50, 0, 0, 0x00000000 },
			{ 0x54, 0, 0, 0x000000fc },
			{ 0x15, 0, 42, 0x000000d0 },
			{ 0x40, 0, 0, 0x00000018 },
			{ 0x15, 0, 40, 0x7f18fe34 },
			{ 0x50, 0, 0, 0x00000020 },
			{ 0x15, 0, 38, 0x000000dd },
			{ 0x40, 0, 0, 0x00000021 },
			{ 0x54, 0, 0, 0x00ffffff },
			{ 0x15, 0, 35, 0x0018fe34 },
			{ 0x50, 0, 0, 0x00000025 },
			{ 0x15, 0, 33, 0x00000004 },
			{ 0x50, 0, 0, 0x00000000 },
			{ 0x45, 31, 0, 0x00000004 },
			{ 0x45, 0, 21, 0x00000008 },
			{ 0x50, 0, 0, 0x00000001 },
			{ 0x45, 0, 4, 0x00000001 },
			{ 0x40, 0, 0, 0x00000012 },
			{ jeq_dst, 0, 26, LSB_dst },
			{ 0x48, 0, 0, 0x00000010 },
			{ jeq_dst, 4, 24, MSB_dst },
			{ 0x40, 0, 0, 0x00000006 },
			{ jeq_dst, 0, 22, LSB_dst },
			{ 0x48, 0, 0, 0x00000004 },
			{ jeq_dst, 0, 20, MSB_dst },
			{ 0x50, 0, 0, 0x00000001 },
			{ 0x45, 0, 13, 0x00000002 },
			{ 0x45, 0, 4, 0x00000001 },
			{ 0x40, 0, 0, 0x0000001a },
			{ jeq_src, 0, 15, LSB_src },
			{ 0x48, 0, 0, 0x00000018 },
			{ jeq_src, 12, 13, MSB_src },
			{ 0x40, 0, 0, 0x00000012 },
			{ jeq_src, 0, 11, LSB_src },
			{ 0x48, 0, 0, 0x00000010 },
			{ jeq_src, 8, 9, MSB_src },
			{ 0x40, 0, 0, 0x00000006 },
			{ jeq_dst, 0, 7, LSB_dst },
			{ 0x48, 0, 0, 0x00000004 },
			{ jeq_dst, 0, 5, MSB_dst },
			{ 0x40, 0, 0, 0x0000000c },
			{ jeq_src, 0, 3, LSB_src },
			{ 0x48, 0, 0, 0x0000000a },
			{ jeq_src, 0, 1, MSB_src },
			{ 0x6, 0, 0, 0x00040000 },
			{ 0x6, 0, 0, 0x00000000 }
						};

	this->bpf.filter = (sock_filter*) malloc(sizeof(sock_filter)*this->bpf.len);
	memcpy(this->bpf.filter, temp_code, sizeof(struct sock_filter) * this->bpf.len);
}

void ESPNOW_manager::bind_filter() {
	int filter_errno;

	if(bpf.len > 0 && this->sock_fd != -1) {
		filter_errno = setsockopt(this->sock_fd, SOL_SOCKET, SO_ATTACH_FILTER, &(this->bpf), sizeof(bpf));
		assert(filter_errno >= 0);
	} else {
		printf("Impossible to bind filter !");
	}
	fflush(stdout);
}