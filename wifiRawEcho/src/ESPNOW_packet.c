#include "ESPNOW_packet.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>


void init_ESPNOW_packet(ESPNOW_packet *packet) {
	//radiotap header	
	packet->radiotap.version = 0x00;
	packet->radiotap.pad = 0x00;
	packet->radiotap.present_1 = 0x0000000e;
	packet->radiotap.flags = 0x10;
	packet->radiotap.datarate = 0x0c;
	packet->radiotap.channel_freq = 0x096c;
	packet->radiotap.channel_flags_quarter = 0x00c0;

	packet->radiotap.length = sizeof(packet->radiotap);
	
	
	//Wlan
	packet->wlan.type = 0xd0;
	packet->wlan.flags = 0x00;
	packet->wlan.duration = 0x3a01;
	packet->wlan.seq = 0x7051;
	

	//action frame
	packet->wlan.actionframe.category_code = 0x7f;
	packet->wlan.actionframe.OUI[0] = 0x18;
	packet->wlan.actionframe.OUI[1] = 0xfe;
	packet->wlan.actionframe.OUI[2] = 0x34;
	
	//vendor specific
	packet->wlan.actionframe.content.elementID = 0xdd;
	packet->wlan.actionframe.content.length = 0xff;
	packet->wlan.actionframe.content.OUI[0] = 0x18;
	packet->wlan.actionframe.content.OUI[1] = 0xfe;
	packet->wlan.actionframe.content.OUI[2] = 0x34;
	packet->wlan.actionframe.content.type = 0x04;
	packet->wlan.actionframe.content.version = 0x01;

	//payload
	for(int i=0;i<250;i++) {
			packet->wlan.actionframe.content.payload[i] = 0x13;
	}
}


int packet_to_bytes(uint8_t *bytes, int max_length, ESPNOW_packet packet) {
	int len = sizeof(packet);	
	assert(max_length > len);	
	
	memcpy(bytes, &packet, len);
	
	return len;
}


void print_raw_packet(uint8_t *data, int len)
{
    printf("----------------------new packet (len : %d)---------------------------\n", len);
    int i;
    for (i = 0; i < len; i++)
    {
        if (i % 16 == 0)
            printf("\n");
		else if(i % 8 == 0)
			printf(" ");
        printf("0x%02x, ", data[i]);
    }
    printf("\n\n");
}
