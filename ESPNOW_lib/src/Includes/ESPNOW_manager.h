#ifndef ESPNOW_manager_H
#define ESPNOW_manager_H

#include <stdint.h>
#include <linux/filter.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <pthread.h>

#include "ESPNOW_types.h"


#define LEN_RAWBYTES_MAX 512

struct thread_args {
	int sock_fd;
	void (*callback)(uint8_t src_mac[6], uint8_t *data, int len);
};

class ESPNOW_manager {
	public:
		ESPNOW_manager() {
			default_values();
		}

		ESPNOW_manager(char* interface) {
			default_values();
			set_interface(interface);
		}

		ESPNOW_manager(char* interface, uint8_t datarate, uint16_t channel_freq, uint8_t src_mac[6], uint8_t dst_mac[6], bool filterOn) {
			default_values();
			set_interface(interface);
			set_channel(channel_freq);
			set_datarate(datarate);
			set_src_mac(src_mac);
			set_dst_mac(dst_mac);
			
			if(filterOn) {
				set_filter(dst_mac, src_mac);
			} else {
				set_filter(NULL, NULL);
			}
			
		}

		void unset_filter();
		void set_filter(uint8_t *src_mac, uint8_t *dst_mac);
		void set_interface(char* interface);
		void set_recv_callback(void (*callback)(uint8_t src_mac[6], uint8_t *data, int len));
		
		void start();
		void stop();
		void end();
		
		//int send(ESPNOW_packet p);
		int send(uint8_t *payload, int len);
		int send();
		
		void set_channel(uint16_t channel_freq) { mypacket.set_channel(channel_freq); }
		void set_datarate(uint8_t datarate) { mypacket.set_datarate(datarate); }
		void set_src_mac(uint8_t src_mac[6]) { mypacket.set_src_mac(src_mac); }
		void set_dst_mac(uint8_t dst_mac[6]) { mypacket.set_dst_mac(dst_mac); }
	

		ESPNOW_packet mypacket;
	private:
		int sock_fd;
		struct sock_fprog bpf;
		int socket_priority;
		char* interface;

		

		pthread_t recv_thd_id;
		struct thread_args recv_thread_params;
		static void* sock_recv_thread (void *p_arg);

		void default_values() {
			bpf.len = 0;
			socket_priority = 7; //Priority
			recv_thread_params.callback = NULL;
		}

};



#endif
