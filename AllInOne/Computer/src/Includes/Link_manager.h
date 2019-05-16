#ifndef LINK_MANAGER_H
#define LINK_MANAGER_H

class LINK_manager {
	public:

		virtual void set_recv_callback(void (*callback)(uint8_t src_mac[6], uint8_t *data, int len)) = 0;
		
		virtual void start() = 0;
		virtual void stop() = 0;
		virtual void end() = 0;
		
		virtual int send(uint8_t *payload, int len) = 0;
		virtual int send() = 0;
		
		void set_src_mac(uint8_t src_mac[6]) { mypacket->set_src_mac(src_mac); }
		void set_dst_mac(uint8_t dst_mac[6]) { mypacket->set_dst_mac(dst_mac); }
	

		Packet_t *mypacket;
		/*
		union mypacket {
			ESPNOW_packet ESPNOW;
			ETHERTNET_packet ETHERNET;
		}
		*/

};

#endif