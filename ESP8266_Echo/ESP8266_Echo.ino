/*
   19/02/2019 Mac adress modified : Dell laptop

  Master/Server:
    Sends a 100Byte ESPNOW data block, and awaits a response
    SLAVE ADDRESS[84:F3:EB:B3:66:CC]
    SELF=84:F3:EB:73:55:0D

    
*/


#include <ESP8266WiFi.h>
extern "C" {
#include <espnow.h>
}

#include <Ticker.h>  //Ticker Library

Ticker blinker;

#define WIFI_CHANNEL 1

//byte selfmac[6] = {0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b};
byte selfmac[6] = {0x84, 0xF3, 0xEB, 0x73, 0x55, 0x1E};

//MAC ADDRESS OF THE DEVICE YOU ARE SENDING TO
//byte remoteDevice[6] = {0x84, 0xF3, 0xEB, 0xB3, 0x66, 0xCC};
//byte remoteDevice[6] = {0x84, 0xF3, 0xEB, 0x73, 0x55, 0x0D};
//byte remoteDevice[6] = {0x00, 0x21, 0x6A, 0xAA, 0xC9, 0x8C};
///byte remoteDevice[6] = {0x84, 0xF3, 0xEB, 0x73, 0x55, 0x0D};
//byte remoteDevice[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //broadcast
byte remoteDevice[6] {0xB4,0xE6,0x2D,0xB5,0x9F,0x85}; //ESP32

const byte dataLength = 250;
byte txData[dataLength];
int error;

void send_echo(byte *rxpacket) {
    memcpy(txData, rxpacket, 8);
    esp_now_send(remoteDevice, txData, dataLength);
}

void setup()
{
  for (int i = 0; i < dataLength; i++)
  {
    txData[i] = 0x13;
  }

  wifi_set_macaddr(STATION_IF,selfmac);

  Serial.begin(115200);
  while(!Serial) {delay(1);}
  Serial.print("\r\n\r\nDevice MAC: ");
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  Serial.println(WiFi.macAddress());
  Serial.println("\r\nESP_Now Dual Mode Transmitter + Receiver [MASTER].\r\n");
  esp_now_init();
  delay(10);
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  //esp_now_add_peer(remoteDevice, ESP_NOW_ROLE_CONTROLLER, WIFI_CHANNEL, NULL, 0);
  esp_now_add_peer(NULL, ESP_NOW_ROLE_CONTROLLER, WIFI_CHANNEL, NULL, 0);


  error = esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len)
  {
    if(len>8) {
      send_echo(data);
    }
  });
  
  Serial.println(error);// if ==0 is OK
}

void loop()
{
  yield();
}
