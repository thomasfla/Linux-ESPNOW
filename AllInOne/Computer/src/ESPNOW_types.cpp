#include <assert.h>

#include "ESPNOW_types.h"


void ESPNOW_packet::set_channel(uint16_t channel_freq) {
this->data.radiotap.channel_freq = channel_freq;
}

void ESPNOW_packet::set_datarate(uint8_t datarate) {
	this->data.radiotap.datarate = datarate;
}


void ESPNOW_packet::set_src_mac(uint8_t my_mac[6]) {
	memcpy(this->data.wlan.sa, my_mac, sizeof(uint8_t)*6);
}

void ESPNOW_packet::set_dst_mac(uint8_t dst_mac[6]) {
	memcpy(this->data.wlan.da, dst_mac, sizeof(uint8_t)*6);
	memcpy(this->data.wlan.bssid, dst_mac, sizeof(uint8_t)*6);
}


int ESPNOW_packet::toBytes(uint8_t *bytes, int max_len) {	
	int correct_len = sizeof(ESPNOW_data) + this->data.wlan.actionframe.content.length - 0xff;
	
	assert(correct_len <= max_len); 

	memcpy(bytes, &(this->data), correct_len);	
	
	memcpy(bytes + correct_len - sizeof(this->data.wlan.fcs), &(this->data.wlan.fcs), sizeof(this->data.wlan.fcs));

	return correct_len;
}

uint8_t* ESPNOW_packet::get_payload_ptr() {
	return this->data.wlan.actionframe.content.payload;
}

int ESPNOW_packet::get_payload_len() {
	return this->data.wlan.actionframe.content.length - 5;
}

void ESPNOW_packet::set_payload_len(int len) {
	this->data.wlan.actionframe.content.set_length(len);
}



int ESPNOW_packet::get_radiotap_len_FromRaw(uint8_t *raw_bytes, int len) {
	if(len < 4) return -1;
	return raw_bytes[2] + (raw_bytes[3] << 8);
}

uint8_t* ESPNOW_packet::get_src_mac_FromRaw(uint8_t *raw_bytes, int len) {
	int radiotap_len = get_radiotap_len_FromRaw(raw_bytes, len);

	if(len < radiotap_len + 10 + 6) return NULL;

	return raw_bytes + radiotap_len + 10;
}

int ESPNOW_packet::get_payload_len_FromRaw(uint8_t *raw_bytes, int len) {
	int radiotap_len = get_radiotap_len_FromRaw(raw_bytes, len);
	
	if(len < radiotap_len + WLAN_LEN + ACTIONFRAME_HEADER_LEN + 1) return -1;

	return raw_bytes[radiotap_len + WLAN_LEN + ACTIONFRAME_HEADER_LEN + 1] - 5;
}

uint8_t* ESPNOW_packet::get_payload_FromRaw(uint8_t *raw_bytes, int len) {
	int radiotap_len = get_radiotap_len_FromRaw(raw_bytes, len);

	if(len < radiotap_len + WLAN_LEN + ACTIONFRAME_HEADER_LEN + VENDORSPECIFIC_CONTENT_LEN) return NULL;
	
	return raw_bytes + radiotap_len + WLAN_LEN + ACTIONFRAME_HEADER_LEN + VENDORSPECIFIC_CONTENT_LEN;
}