#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi_internal.h>

#include <Ticker.h>  //Ticker Library

#define CHANNEL 10
#define DATARATE WIFI_PHY_RATE_12M

#define N_BATCH 1000

static byte dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; //broadcast
//static byte dest_mac[6] = {0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b}; //computers
//static byte dest_mac[6] ={0x84, 0xF3, 0xEB, 0x73, 0x55, 0x1E}; //ESP8266 (2) Echo mode

esp_now_peer_info_t peer;
wifi_config_t wifi_config;

Ticker blinker;

#define DATA_LEN 127
uint8_t txData[DATA_LEN];

void init_data() {
  for(int i=0;i<DATA_LEN;i++) {
    txData[i] = 0x14;
  }
}
