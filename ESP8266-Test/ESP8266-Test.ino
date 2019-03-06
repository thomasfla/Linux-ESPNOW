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

#define WIFI_CHANNEL 1

//byte selfmac[6] = {0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b};
byte selfmac[6] = {0x84, 0xF3, 0xEB, 0x73, 0x55, 0x0D};

//MAC ADDRESS OF THE DEVICE YOU ARE SENDING TO
//byte remoteDevice[6] = {0x84, 0xF3, 0xEB, 0xB3, 0x66, 0xCC};
//byte remoteDevice[6] = {0x84, 0xF3, 0xEB, 0x73, 0x55, 0x0D};
//byte remoteDevice[6] = {0x00, 0x21, 0x6A, 0xAA, 0xC9, 0x8C};
byte remoteDevice[6] = {0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b};

const byte dataLength = 250;
byte cnt = 0;
byte txrxData[dataLength];
long timerData[3];
int error ;
void setup()
{
  for (int i = 0; i < dataLength; i++)
  {
    txrxData[i] = 0x12;
  }

  wifi_set_macaddr(STATION_IF,selfmac);


  
  pinMode(2, OUTPUT); //TX
  pinMode(0, OUTPUT); //RX
  Serial.begin(115200);
  Serial.print("\r\n\r\nDevice MAC: ");
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  Serial.println(WiFi.macAddress());
  Serial.println("\r\nESP_Now Dual Mode Transmitter + Receiver [MASTER].\r\n");
  esp_now_init();
  delay(10);
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(remoteDevice, ESP_NOW_ROLE_CONTROLLER, WIFI_CHANNEL, NULL, 0);

  error = esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len)
  {
    Serial.println(error);// if ==0 is OK
    //timerData[1] = micros();
    // timerData[2] = timerData[1]-timerData[0];
    //digitalWrite(0, HIGH);
    delayMicroseconds(10);
    Serial.printf("\tReceived [%d]\tTook\t[%d]micros\r\n", data[0], timerData[2]);
    //memcpy(txrxData, data, len );
    //digitalWrite(0, LOW);
  });

  Serial.println(error);// if ==0 is OK
}

void loop()
{
  while (micros() - timerData[0] < 99999) delayMicroseconds(1);
  timerData[0] = micros();

  //digitalWrite(2, HIGH);
  esp_now_send(remoteDevice, txrxData, dataLength);
  Serial.printf("\tSent [%d]\r\n", txrxData[0]);
  Serial.println(error);// if ==0 is OK
  //digitalWrite(2, LOW);
  txrxData[0]++;
}


