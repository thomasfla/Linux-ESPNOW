#ifndef PACKETS_H
#define PACKETS_H

#include <stdint.h>
#include <stdio.h>

struct IEEE80211_radiotap {
	uint8_t version;                //= 0;
    uint8_t pad;                    //= 0;
    uint16_t length;                //= 0x00,0x26;
    uint32_t present_1;             //= {0x2f, 0x40, 0x00, 0xa0};
    uint32_t present_2;             //= {0x20, 0x08, 0x00, 0xa0};
    uint32_t present_3;             //= {0x20, 0x08, 0x00, 0x00};
	uint64_t timestamp;
    uint8_t flags;          		//= 10;
    uint8_t datarate;               //0x0c
    uint16_t channel_freq;          //0x6c, 0x09
    uint16_t channel_flags_quarter; //0xc0, 0x00
};

struct IEEE80211_vendorspecific {
	uint8_t elementID;		//0xdd
	uint8_t length;			//255
	uint8_t	OUI[3];			//0x18,0xfe, 0x34
	uint8_t type;			//0x04
	uint8_t version;		//0x01
	uint8_t payload[250];
	
};

struct IEEE80211_actionframe {
	uint8_t unknown_bytes[4];
	uint8_t category_code;	//0x7f
    uint8_t OUI[3];			//0x18,0xfe, 0x34
	struct IEEE80211_vendorspecific content;
};


struct IEEE80211_wlan {
	uint8_t type;					//0xd0
    uint8_t flags;            		//0x00
    uint16_t duration;         //0x3a, 0x01
    uint8_t da[6];             //0x84, 0xf3, 0xeb, 0x73, 0x55, 0x0d
    uint8_t sa[6];             //0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b
    uint8_t bssid[6];          //0x84, 0xf3, 0xeb, 0x73, 0x55, 0x0d
    uint16_t seq;              //0x70, 0x51
    
	struct IEEE80211_actionframe actionframe;

    uint32_t fcs; //0x1c, 0xd5, 0x35, 0xd3
};


typedef struct {

	struct IEEE80211_radiotap radiotap;
	struct IEEE80211_wlan wlan;

} ESPNOW_packet;

void init_ESPNOW_packet(ESPNOW_packet *packet);

int packet_to_bytes(uint8_t *bytes, int max_length, ESPNOW_packet packet);

int IEEE80211_wlan_to_bytes(uint8_t *bytes, int max_length, struct IEEE80211_wlan wlan);

int IEEE80211_actionframe_to_bytes(uint8_t *bytes, int max_length, struct IEEE80211_actionframe actionframe);

int IEEE80211_vendorspecific_to_bytes(uint8_t *bytes, int max_length, struct IEEE80211_vendorspecific vendorspecific);

int IEEE80211_radiotap_to_bytes(uint8_t *bytes, int max_length, struct IEEE80211_radiotap radiotap);

#endif
