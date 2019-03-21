#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi_internal.h>

// Global copy of slave
esp_now_peer_info_t peer;
#define CHANNEL 10
#define DATARATE WIFI_PHY_RATE_12M

#include <Ticker.h>  //Ticker Library
Ticker blinker;

#define TRY_ESP_ACTION(action, name) if(action == ESP_OK) {Serial.println("\t+ "+String(name));} else {Serial.println("----------Error while " + String(name) + " !---------------");}

static byte dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; //broadcast
//static byte dest_mac[6] = {0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b}; //computers
//static byte dest_mac[6] ={0x84, 0xF3, 0xEB, 0x73, 0x55, 0x1E}; //ESP8266 (2) Echo mode

#define DATA_LEN 127
uint8_t txData[DATA_LEN];

void init_data() {
  for(int i=0;i<DATA_LEN;i++) {
    txData[i] = 0x14;
  }
}

#define N_BATCH 1000

#define HISTO_INF 0
#define HISTO_SUP 10000
#define HISTO_N_STEP 100

int histogram[HISTO_N_STEP];
int histogram_higher;
int histogram_lower;
int error_recv;
int receiveNb;
int error_send;
double avg;

long batchStart =0;

unsigned long n_sent;

void init_histo() {
  for(int i=0;i<HISTO_N_STEP;i++) {
    histogram[i] = 0;
  }
  histogram_higher = 0;
  histogram_lower = 0;
  error_recv=0;
  error_send=0;
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

wifi_config_t wifi_config;

void print_histo() {
  Serial.println("----------");
  Serial.printf("Units :\tus");
  Serial.println();
  Serial.printf("Bounds :\t%d\t%d", HISTO_INF, HISTO_SUP);
  Serial.println();
  Serial.printf("Nb_values :\t%d", HISTO_N_STEP);
  Serial.println();
  for(int i=0;i<HISTO_N_STEP;i++) {
    Serial.printf("%d\t", histogram[i]);
  }
  Serial.printf("%d",histogram_higher);
  
  Serial.println();
  Serial.printf("Average :\t%d\t", receiveNb != 0 ? int(avg/receiveNb) : -1);
  Serial.println();
  Serial.printf("Received :\t%d", receiveNb);
  Serial.println();
  Serial.printf("Sent :\t%d", n_sent-error_send);
  Serial.println();
}


// send data
void sendData() {
  if(n_sent < N_BATCH) {
    long mytime = micros();
    memcpy(txData, &mytime, sizeof(mytime));
    esp_now_send(peer.peer_addr, txData, sizeof(txData[0])*DATA_LEN);
    n_sent++;
  }
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(status != ESP_OK) {
    error_send++;
  }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *rxData, int len) {
  long receiveTime = micros();
    long sendTime;
    
    if(len>sizeof(sendTime)) {
      memcpy(&sendTime, rxData, sizeof(sendTime));

      if(sendTime>batchStart) {
        fill_histo(receiveTime - sendTime);
      }
    } else {
      error_recv++;
    }
}

void setup() {
  Serial.begin(115200);

  WiFi.disconnect();
  //Set device in STA mode to begin with

  WiFi.mode(WIFI_STA);
  
  Serial.println("ESPNow/Basic/Master Example");
  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());

  TRY_ESP_ACTION( esp_wifi_stop(), "stop WIFI");
  
  TRY_ESP_ACTION( esp_wifi_deinit(), "De init");

  wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
  my_config.ampdu_tx_enable = 0;
  
  TRY_ESP_ACTION( esp_wifi_init(&my_config), "Disable AMPDU");
  
  TRY_ESP_ACTION( esp_wifi_start(), "Restart WiFi");

  //TRY_ESP_ACTION( esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE), "Set channel");
  /*
  TRY_ESP_ACTION( esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config), "Get config");
  
  wifi_config.sta.channel = CHANNEL;
  
  TRY_ESP_ACTION( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config), "Set config (channel)");
  */
  TRY_ESP_ACTION( esp_wifi_internal_set_fix_rate(ESP_IF_WIFI_STA, true, DATARATE), "Fixed rate set up");

  TRY_ESP_ACTION( esp_now_init(), "ESPNow Init");

  TRY_ESP_ACTION(  esp_now_register_send_cb(OnDataSent), "Attach send callback");
  
  TRY_ESP_ACTION( esp_now_register_recv_cb(OnDataRecv), "Attach recv callback");
  
  memset(&peer, 0, sizeof(peer));

  for (int ii = 0; ii < 6; ++ii ) {
    peer.peer_addr[ii] = (uint8_t) dest_mac[ii];
  }
  
  peer.channel = 0; // pick a channel
  peer.encrypt = 0; // no encryption
      
  TRY_ESP_ACTION( esp_now_add_peer(&peer), "Add peer");

  n_sent = 0;
  init_data();
  blinker.attach(0.001, sendData);
}

void loop() {
  if(n_sent >= N_BATCH) {
    delay(500);
    
    print_histo();
    init_histo();
    
    n_sent=0;
    batchStart = micros();
    
  } else {
    yield();
  }
}
