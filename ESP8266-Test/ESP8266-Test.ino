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

#define WIFI_CHANNEL 1

//byte selfmac[6] = {0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b};
byte selfmac[6] = {0x84, 0xF3, 0xEB, 0x73, 0x55, 0x0D};

#define NB_TRIES 2000

#define HISTO_INF 400
#define HISTO_SUP 1200
#define HISTO_N_STEP 80

int histogram[HISTO_N_STEP];
int histogram_higher;
int histogram_lower;
int error_nb;

//MAC ADDRESS OF THE DEVICE YOU ARE SENDING TO
//byte remoteDevice[6] = {0x84, 0xF3, 0xEB, 0xB3, 0x66, 0xCC};
//byte remoteDevice[6] = {0x84, 0xF3, 0xEB, 0x73, 0x55, 0x0D};
//byte remoteDevice[6] = {0x00, 0x21, 0x6A, 0xAA, 0xC9, 0x8C};
byte remoteDevice[6] = {0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b};

const byte dataLength = 250;
byte cnt = 0;
byte txrxData[dataLength];
long timerData[3];
unsigned long lastsend;
int error;

void init_histo() {
  bzero(histogram, HISTO_N_STEP*sizeof(histogram));
  histogram_higher = 0;
  histogram_lower = 0;
  error_nb=0;
}

int fill_histo(long value) {
  int index = (value-HISTO_INF) * HISTO_N_STEP / (HISTO_SUP - HISTO_INF);
  if(index >= HISTO_N_STEP) {
    histogram_higher++;
  } else if(index < 0) {
    histogram_lower++;
  } else {
    histogram[index]++;
  }
}

void send_histo() {
  Serial.printf("Bounds\t%d\tµs\t%d\tµs", HISTO_INF, HISTO_SUP);
  Serial.println();
  Serial.printf("Nb out of bounds :");
  Serial.println();
  Serial.printf("Higher :\t%d\tLower :\t%d", histogram_higher, histogram_lower);
  Serial.println();
  Serial.printf("Histo :");
  Serial.println();
  for(int i=0;i<HISTO_N_STEP;i++) {
    Serial.printf("%d\t", histogram[i]);
  }
  Serial.println();
  Serial.printf("Errors :\t%d", error_nb);
  Serial.println();
}

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
    timerData[1] = micros();
    timerData[2] = timerData[1]-timerData[0];

    if(data[0] == txrxData[0] -1 ) {
      fill_histo(timerData[2]);
    } else {
      error_nb++;
    }
    
    
  });

  Serial.println(error);// if ==0 is OK
}

void loop()
{
  Serial.println();
  Serial.println("------New test :----------");
  Serial.println();
  init_histo();
  for(long int tries=0;tries<NB_TRIES;tries++) {
    lastsend = micros();
    timerData[0] = micros();
  
    esp_now_send(remoteDevice, txrxData, dataLength);
    
    txrxData[0]++;
    while(micros() - lastsend < 9999) { yield();}
  }
  send_histo();
  delay(1000);
  Serial.println();
  Serial.println("--------------------------");
  Serial.println();
}
