#ifndef ETHERNET_MANAGER_H
#define ETHERNET_MANAGER_H

#include <stdint.h>
#include <linux/filter.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <pthread.h>

#include "ETHERNET_types.h"

#include "Link_manager.h"


#define LEN_RAWBYTES_MAX 512

class ETHERNET_manager : public LINK_manager {
	public:
		ETHERNET_manager() : LINK_manager(&myETHpacket) {}

		ETHERNET_manager(char* interface) : LINK_manager(&myETHpacket, interface) {}

		ETHERNET_manager(char* interface, uint8_t src_mac[6], uint8_t dst_mac[6])
		: LINK_manager(&myETHpacket, interface) {
			set_src_mac(src_mac);
			set_dst_mac(dst_mac);
		}

	private:
		ETHERNET_packet myETHpacket;
};

#endif