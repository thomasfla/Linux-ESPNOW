/*
Etienne Arlaud
*/

#include <stdint.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/time.h>

#include "ESPNOW_manager.h"
#include "ESPNOW_types.h"
						
static uint8_t my_mac[6] = {0xF8, 0x1A, 0x67, 0xb7, 0xEB, 0x0B};
static uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t ESP_mac[6] = {0xB4,0xE6,0x2D,0xB5,0x9F,0x85};

ESPNOW_manager *handler;

uint8_t payload[250];

void callback(uint8_t src_mac[6], uint8_t *data, int len) {
	memcpy(payload, data, 250);
	//handler->set_dst_mac(src_mac);
	handler->send(payload,250);
	printf("wesh\n");
}

int main(int argc, char **argv) {

	handler = new ESPNOW_manager("wlp5s0", DATARATE_54Mbps, CHANNEL_freq_11, my_mac, dest_mac, false);

	//handler->set_filter(NULL, dest_mac);

	handler->set_recv_callback(&callback);

	handler->start();

	while(1) {
		sleep(1);
	}

	handler->end();
}
