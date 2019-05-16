/*
Etienne Arlaud
*/

#include <stdint.h>
#include <stdio.h>

#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include <thread>

#include "ESPNOW_manager.h"
#include "ESPNOW_types.h"

using namespace std;

static uint8_t my_mac[6] = {0xF8, 0x1A, 0x67, 0xb7, 0xEB, 0x0B};
static uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t ESP_mac[6] = {0xB4,0xE6,0x2D,0xB5,0x9F,0x85};

ESPNOW_manager *handler;

uint8_t payload[127];

void callback(uint8_t src_mac[6], uint8_t *data, int len) {
	handler->mypacket.wlan.actionframe.content.length = 127 + 5;
	memcpy(handler->mypacket.wlan.actionframe.content.payload, data, 6);
	//handler->set_dst_mac(dest_mac);
	handler->send();
}

int main(int argc, char **argv) {
	assert(argc > 1);

	nice(-20);

	handler = new ESPNOW_manager(argv[1], DATARATE_24Mbps, CHANNEL_freq_9, my_mac, dest_mac, false);

	handler->set_filter(ESP_mac, dest_mac);

	handler->set_recv_callback(&callback);

	handler->start();

	while(1) {
		std::this_thread::yield();
	}

	handler->end();
}
