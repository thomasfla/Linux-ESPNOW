#include "ESPNOW_types.h"

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>

void ESPNOW_packet::set_channel(uint16_t channel_freq) {
this->radiotap.channel_freq = channel_freq;
}

void ESPNOW_packet::set_datarate(uint8_t datarate) {
	this->radiotap.datarate = datarate;
}

void ESPNOW_packet::set_src_mac(uint8_t my_mac[6]) {
	memcpy(this->wlan.sa, my_mac, sizeof(uint8_t)*6);
}

void ESPNOW_packet::set_dst_mac(uint8_t dst_mac[6]) {
	memcpy(this->wlan.da, dst_mac, sizeof(uint8_t)*6);
	memcpy(this->wlan.bssid, dst_mac, sizeof(uint8_t)*6);
}

int ESPNOW_packet::toBytes(uint8_t *bytes, int max_len) {	
	int correct_len = sizeof(ESPNOW_packet) + this->wlan.actionframe.content.length - 0xff;
	
	assert(correct_len <= max_len); 

	memcpy(bytes, this, correct_len);	
	
	memcpy(bytes + correct_len - sizeof(this->wlan.fcs), &(this->wlan.fcs), sizeof(this->wlan.fcs));

	return correct_len;
}

int ESPNOW_packet::get_radiotap_len(uint8_t *raw_bytes, int len) {
	if(len < 4) return -1;
	return (int)raw_bytes[2] + ((int)raw_bytes[3] << 8);
}

uint8_t* ESPNOW_packet::get_src_mac(uint8_t *raw_bytes, int len) {
	int radiotap_len = get_radiotap_len(raw_bytes, len);

	if(len < radiotap_len + 10 + 6) return NULL;

	return raw_bytes + radiotap_len + 10;
}

int ESPNOW_packet::get_payload_len(uint8_t *raw_bytes, int len) {
	int radiotap_len = get_radiotap_len(raw_bytes, len);
	
	if(len < radiotap_len + WLAN_LEN + ACTIONFRAME_HEADER_LEN + 1) return -1;

	return raw_bytes[radiotap_len + WLAN_LEN + ACTIONFRAME_HEADER_LEN + 1] - 5;
}

uint8_t* ESPNOW_packet::get_payload(uint8_t *raw_bytes, int len) {
	int radiotap_len = get_radiotap_len(raw_bytes, len);

	if(len < radiotap_len + WLAN_LEN + ACTIONFRAME_HEADER_LEN + VENDORSPECIFIC_CONTENT_LEN) return NULL;
	
	return raw_bytes + radiotap_len + WLAN_LEN + ACTIONFRAME_HEADER_LEN + VENDORSPECIFIC_CONTENT_LEN;
}
