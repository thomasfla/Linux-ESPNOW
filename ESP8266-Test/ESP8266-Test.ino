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
byte selfmac[6] = {0x84, 0xF3, 0xEB, 0x73, 0x55, 0x0D};

#define NB_TRIES 1000

#define HISTO_INF 0
#define HISTO_SUP 10000
#define HISTO_N_STEP 10

int histogram[HISTO_N_STEP];
int histogram_higher;
int histogram_lower;
int error_nb;
int receiveNb;
double avg;

long batchStart =0;

bool sendNew = true;

//MAC ADDRESS OF THE DEVICE YOU ARE SENDING TO
//byte remoteDevice[6] = {0x84, 0xF3, 0xEB, 0xB3, 0x66, 0xCC};
//byte remoteDevice[6] = {0x84, 0xF3, 0xEB, 0x73, 0x55, 0x0D};
//byte remoteDevice[6] = {0x00, 0x21, 0x6A, 0xAA, 0xC9, 0x8C};
///byte remoteDevice[6] = {0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b}; //computer
//byte remoteDevice[6] = {0x84, 0xF3, 0xEB, 0x73, 0x55, 0x1E}; //Deuxieme ESP8266
byte remoteDevice[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const byte dataLength = 250;
byte txData[dataLength];
unsigned long n_sent;
int error;

void init_histo() {
  for(int i=0;i<HISTO_N_STEP;i++) {
    histogram[i] = 0;
  }
  histogram_higher = 0;
  histogram_lower = 0;
  error_nb=0;
  receiveNb=0;
  avg=0;
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

  receiveNb++;
  avg += value;
}

void print_histo() {
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
  Serial.printf("Average :\t%f\t", receiveNb != 0 ? avg/receiveNb : -1);
  Serial.println();
  Serial.printf("Received :\t%d", receiveNb);
  Serial.println();
  Serial.printf("Errors :\t%d", error_nb);
  Serial.println();
}

void send_test() {
  
  if(n_sent < NB_TRIES) {
    long mytime = micros();
    memcpy(txData, &mytime, sizeof(mytime));
    esp_now_send(remoteDevice, txData, dataLength);
    n_sent++;
  } else if (Serial) {
    print_histo();
    Serial.println();
    Serial.println("--------------------------");
    Serial.println();
    
    n_sent=0;

    
    Serial.println();
    Serial.println("------New test :----------");
    Serial.println();
    init_histo();
    batchStart = micros();
  }
}

void setup()
{
  for (int i = 0; i < dataLength; i++)
  {
    txData[i] = 0x12;
  }

  wifi_set_macaddr(STATION_IF,selfmac);


  
  pinMode(2, OUTPUT); //TX
  pinMode(0, OUTPUT); //RX
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
    long receiveTime = micros();
    long sendTime;
    
    if(len>sizeof(sendTime)) {
      memcpy(&sendTime, data, sizeof(sendTime));

      if(sendTime>batchStart) {
        fill_histo(receiveTime - sendTime);
      }
    } else {
      error_nb++;
    }
    sendNew = true;
    
  });

    esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {
    sendNew = sendStatus != 0;
  });
  
  Serial.println(error);// if ==0 is OK

  n_sent = 0;
  blinker.attach(0.01, send_test);
}

void loop()
{
  yield();
}
