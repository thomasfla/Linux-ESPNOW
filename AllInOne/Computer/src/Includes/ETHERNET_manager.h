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
		ETHERNET_manager() {
			default_values();
		}

		ETHERNET_manager(char* interface) {
			default_values();
			set_interface(interface);
		}

		ETHERNET_manager(char* interface, uint8_t src_mac[6], uint8_t dst_mac[6]) {
			default_values();
			set_interface(interface);
			set_src_mac(src_mac);
			set_dst_mac(dst_mac);
		}

		void unset_filter();
		void set_filter(uint8_t *src_mac, uint8_t *dst_mac);
		
		
		/** Start overriding inherited methods **/
		void set_recv_callback(void (*callback)(uint8_t src_mac[6], uint8_t *data, int len)) override;
		
		void start() override;
		void stop()  override;
		void end()   override;
		
		//int send(ESPNOW_packet p);
		int send(uint8_t *payload, int len) override;
		int send() 							override;
		/** Finish overriding inherited methods **/


	private:
		int sock_fd;
		int socket_priority;
		char* interface;
		
		ESPNOW_packet myETHpacket;

		pthread_t recv_thd_id;
		struct thread_args recv_thread_params;
		static void* sock_recv_thread (void *p_arg);

		void default_values() {
			bpf.len = 0;
			//socket_priority = 7; //Priority
			recv_thread_params.callback = NULL;
			mypacket = &myETHpacket;
		}

};

#endif