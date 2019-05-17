#ifndef LINK_MANAGER_H
#define LINK_MANAGER_H

#include "Link_types.h"
#include "ESPNOW_types.h"


#define LEN_RAWBYTES_MAX 512

struct thread_args {
	int sock_fd = -1;
	void (*callback)(uint8_t src_mac[6], uint8_t *data, int len) = NULL;
	Packet_t *mypacket;
};

class LINK_manager {
	public:
		LINK_manager(Packet_t *packet_p) : mypacket(packet_p) {}

		LINK_manager(Packet_t *packet_p, char* interface) : LINK_manager(packet_p) {
			set_interface(interface);
		}

		void set_recv_callback(void (*callback)(uint8_t src_mac[6], uint8_t *data, int len));

		void set_interface(char* interface);

		void start();
		void stop();
		void end();
		
		int send(uint8_t *payload, int len);
		int send();
		
		void set_src_mac(uint8_t src_mac[6]);
		void set_dst_mac(uint8_t dst_mac[6]);
	
		Packet_t *mypacket;

	protected:
		int sock_fd = -1;
		int socket_priority = 7;
		char* interface;

		pthread_t recv_thd_id;
		struct thread_args recv_thread_params;
		static void* sock_recv_thread (void *p_arg);
};

#endif