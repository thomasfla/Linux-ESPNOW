#include "packets.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

#define byte_n(word, n) ((uint64_t) word & (((uint64_t) 1<<((n+1)*8)) -1))>>(n*8)

void init_ESPNOW_packet(ESPNOW_packet *packet) {
	packet->radiotap.version = 0x00;
	packet->radiotap.pad = 0x00;
	packet->radiotap.length = 14;
	packet->radiotap.present_1 = 0x0000000e;
	packet->radiotap.flags = 0x10;
	packet->radiotap.datarate = 0x0c;
	packet->radiotap.channel_freq = 0x096c;
	packet->radiotap.channel_flags_quarter = 0x00c0;

	packet->wlan.type = 0xd0;
	packet->wlan.flags = 0x00;
	packet->wlan.duration = 0x3a01;
	packet->wlan.seq = 0x7051;
	
	packet->wlan.actionframe.category_code = 0x7f;
	packet->wlan.actionframe.OUI[0] = 0x18;
	packet->wlan.actionframe.OUI[1] = 0xfe;
	packet->wlan.actionframe.OUI[2] = 0x34;
	
	packet->wlan.actionframe.content.elementID = 0xdd;
	packet->wlan.actionframe.content.length = 0xff;
	packet->wlan.actionframe.content.OUI[0] = 0x18;
	packet->wlan.actionframe.content.OUI[1] = 0xfe;
	packet->wlan.actionframe.content.OUI[2] = 0x34;
	packet->wlan.actionframe.content.type = 0x04;
	packet->wlan.actionframe.content.version = 0x01;

	for(int i=0;i<250;i++) {
			packet->wlan.actionframe.content.payload[i] = 0x13;
	}

	packet->wlan.fcs = 0x1cd535d3;
}

int packet_to_bytes(uint8_t *bytes, int max_length, ESPNOW_packet packet) {
	int length = 0;
	length = IEEE80211_radiotap_to_bytes(bytes, max_length, packet.radiotap);
	length += IEEE80211_wlan_to_bytes(bytes+length, max_length - length, packet.wlan);
	return length;
}

int IEEE80211_radiotap_to_bytes(uint8_t *bytes, int max_length, struct IEEE80211_radiotap radiotap) {
	int len = sizeof(radiotap);	
	assert(max_length > len);	
	
	memcpy(bytes, &radiotap, len);
	
	return sizeof(len);
}

int IEEE80211_wlan_to_bytes(uint8_t *bytes, int max_length, struct IEEE80211_wlan wlan) {
	assert(max_length > 28);

	bytes[0] = wlan.type;
	bytes[1] = wlan.flags;
	bytes[2] = byte_n(wlan.duration, 0);
	bytes[3] = byte_n(wlan.duration, 1);
	memcpy(bytes+4, wlan.da, sizeof(uint8_t)*6);
	memcpy(bytes+10, wlan.sa, sizeof(uint8_t)*6);
	memcpy(bytes+16, wlan.bssid, sizeof(uint8_t)*6);
	bytes[22] = byte_n(wlan.seq, 0);
	bytes[23] = byte_n(wlan.seq, 1);

	int len = IEEE80211_actionframe_to_bytes(bytes+24, max_length-24, wlan.actionframe);
	
	//bytes[24+len+1] = byte_n(wlan.fcs,0);
	//bytes[24+len+2] = byte_n(wlan.fcs,1);
	//bytes[24+len+3] = byte_n(wlan.fcs,2);
	//bytes[24+len+4] = byte_n(wlan.fcs,3);
	
	return 24+len +1;
}

int IEEE80211_actionframe_to_bytes(uint8_t *bytes, int max_length, struct IEEE80211_actionframe actionframe) {
	assert(max_length > 4);
	
	
	bytes[0] = actionframe.category_code;
	memcpy(bytes+1, actionframe.OUI, sizeof(uint8_t)*3);
	
	memcpy(bytes+4, actionframe.unknown_bytes, sizeof(uint8_t)*4);

	int len = IEEE80211_vendorspecific_to_bytes(bytes+8, max_length-8, actionframe.content);
	
	return 7+len +1;
}

int IEEE80211_vendorspecific_to_bytes(uint8_t *bytes, int max_length, struct IEEE80211_vendorspecific vendorspecific) {
	assert(max_length > 10);
	
	bytes[0] = vendorspecific.elementID;
	bytes[1] = vendorspecific.length;
	memcpy(bytes+2, vendorspecific.OUI, sizeof(uint8_t)*3);
	bytes[5] = vendorspecific.type;
	bytes[6] = vendorspecific.version;
	memcpy(bytes+7, vendorspecific.payload, sizeof(uint8_t)*250);
	
	return 6 + 250 +1;
}


