#ifndef ESPNOW_HANDLER_H
#define ESPNOW_HANDLER_H

#include <stdint.h>
#include <linux/filter.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <pthread.h>

#include "ESPNOW_types.h"

//#define MAX_BPFCODE_LEN 100

#define LEN_MAX_RAW_BYTES 400

struct args_thread {
	int sock_fd;
	void (*callback)(uint8_t src_mac[6], uint8_t *data, int len);
};

class ESPNOW_handler {
	public:
		ESPNOW_handler() {
			default_init();
		}

		ESPNOW_handler(char* interface) {
			default_init();
			set_interface(interface);
		}


		ESPNOW_handler(char* interface, uint8_t dest_mac[6]) {
			default_init();
			set_interface(interface);
			set_dest_mac(dest_mac);
		}


		void set_filter(uint8_t *src_mac, uint8_t *dst_mac);
		void set_dest_mac(uint8_t dest_mac[6]);
		void set_interface(char* interface);
		void callback();
		void start();
		int send(ESPNOW_packet p);

		//void send_payload(uint8_t *payload, int len);
		//void send_payload(uint8_t *payload, int len, uint8_t dest[6]);
		//void send_frame(IEEE80211_actionframe frame, uint8_t dest[6]);
		//void send_raw(uint8_t *buff, int len, uint8_t dest[6]);
	
	private:
		char* interface;
		uint8_t dest_mac[6];
				
		int socket_priority;

		uint8_t raw_bytes[LEN_MAX_RAW_BYTES];

		struct sock_fprog bpf;
		ESPNOW_packet mypacket;
		
		int sock_fd;

		void default_init() {
			bpf.len = -1;
			socket_priority = 7; //Priority
			thread_params.callback = NULL;
		}

		pthread_t ul_recv_thd_id;

		struct args_thread thread_params;

		static void* sock_recv_thread (void *p_arg);
};



#endif
