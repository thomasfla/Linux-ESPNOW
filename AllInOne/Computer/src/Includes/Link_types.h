#ifndef LINK_TYPES_H
#define LINK_TYPES_H

struct Packet_t{
	static int OFFSET() {return sizeof(Packet_t);}

	virtual void set_src_mac(uint8_t my_mac[6])		 = 0;
	virtual void set_dst_mac(uint8_t dst_mac[6])	 = 0;
	
	virtual int toBytes(uint8_t *bytes, int max_len) = 0;

	virtual uint8_t* get_payload_ptr()				 = 0;
	virtual int get_payload_len()		 			 = 0;

	virtual void set_payload_len(int len) 			 = 0;

	virtual uint8_t* get_src_mac_FromRaw(uint8_t *raw_bytes, int len)	= 0;
	virtual int get_payload_len_FromRaw(uint8_t *raw_bytes, int len)	= 0;
	virtual uint8_t* get_payload_FromRaw(uint8_t *raw_bytes, int len)	= 0;

	Packet_t *mypacket;
	
} __attribute__((__packed__));


#endif