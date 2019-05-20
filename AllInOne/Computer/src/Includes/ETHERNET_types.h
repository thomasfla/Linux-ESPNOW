#ifndef ETHERNET_TYPES_H
#define ETHERNET_TYPES_H


#include <stdint.h>

#include "Link_types.h"


#define ETH_RECV_SIZE_MIN 16
#define ETH_SEND_SIZE_MIN 64


struct ETHERNET_data {
	uint8_t dst_mac[6];
	uint8_t src_mac[6];
	uint16_t ethertype;

	/* Custom payload*/
	uint16_t length;
	uint8_t payload[255];

	ETHERNET_data() {
		//memset(this->dst_mac, sizeof(uint8_t)*6, 0xFF);
		//memset(this->src_mac, sizeof(uint8_t)*6, 0xAA);

		this->ethertype = 0xb588; //Local Experimental Ethertype : gives us maximum freedom.
	}
} __attribute__((__packed__));

struct ETHERNET_packet : Packet_t {
	
	struct ETHERNET_data data;

	/** Start overriding inherited methods **/
	void set_src_mac(uint8_t my_mac[6]) 		override;
	void set_dst_mac(uint8_t dst_mac[6]) 		override;
	
	int toBytes(uint8_t *bytes, int max_len) 	override;
	uint8_t* get_payload_ptr() 					override;
	int get_payload_len() 						override;
	void set_payload_len(int len) 				override;

	uint8_t* get_src_mac_FromRaw(uint8_t *raw_bytes, int len)	override;
	int get_payload_len_FromRaw(uint8_t *raw_bytes, int len)	override;
	uint8_t* get_payload_FromRaw(uint8_t *raw_bytes, int len)	override;
	/** Finish overriding inherited methods **/
};

typedef struct ETHERNET_packet ETHERNET_packet;

#endif
