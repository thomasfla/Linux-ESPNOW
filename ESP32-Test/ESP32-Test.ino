#include <esp_now.h>
#include <WiFi.h>

// Global copy of slave
esp_now_peer_info_t peer;
#define CHANNEL 1

#include <Ticker.h>  //Ticker Library
Ticker blinker;

static byte dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; //broadcast
//static byte dest_mac[6] = {0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b}; //computers
//static byte dest_mac[6] ={0x84, 0xF3, 0xEB, 0x73, 0x55, 0x1E}; //ESP8266 (2) Echo mode

#define DATA_LEN 250
uint8_t txData[DATA_LEN];

void init_data() {
  for(int i=0;i<DATA_LEN;i++) {
    txData[i] = 0x14;
  }
}

#define NB_TRIES 1000

#define HISTO_INF 0
#define HISTO_SUP 10000
#define HISTO_N_STEP 100

int histogram[HISTO_N_STEP];
int histogram_higher;
int histogram_lower;
int error_nb;
int receiveNb;
double avg;

long batchStart =0;

bool sendNew = true;

unsigned long n_sent;

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


bool pairWithPeer() {
  memset(&peer, 0, sizeof(peer));

  for (int ii = 0; ii < 6; ++ii ) {
    peer.peer_addr[ii] = (uint8_t) dest_mac[ii];
  }
  
  peer.channel = CHANNEL; // pick a channel
  peer.encrypt = 0; // no encryption
      
  esp_err_t addStatus = esp_now_add_peer(&peer);
  
  if (addStatus == ESP_OK) {
    // Pair success
    Serial.println("Pair success");
    return true;
  } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW Not Init");
    return false;
  } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
    return false;
  } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
    Serial.println("Peer list full");
    return false;
  } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("Out of memory");
    return false;
  } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
    Serial.println("Peer Exists");
    return true;
  } else {
    Serial.println("Not sure what happened");
    return false;
  }
}

// send data
void sendData() {
  if(n_sent < NB_TRIES) {
    long mytime = micros();
    memcpy(txData, &mytime, sizeof(mytime));
    esp_now_send(peer.peer_addr, txData, sizeof(txData[0])*DATA_LEN);
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

// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
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
      error_nb++;
    }
    sendNew = true;
}

void setup() {
  Serial.begin(115200);

  WiFi.disconnect();
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  
  Serial.println("ESPNow/Basic/Master Example");
  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  } else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
  
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  if(esp_now_register_recv_cb(OnDataRecv) != ESP_OK) {
    Serial.println("ERROR WITH RECEIVE CALLBACK !");
    ESP.restart();
  } else {
    Serial.println("Receive cb OK");
  }
  init_data();
  
  bool isPaired = pairWithPeer();
  if(!isPaired) {
    Serial.println("Slave pair failed!");
    ESP.restart();
  }

  n_sent = 0;
  
  blinker.attach(0.01, sendData);
}

void loop() {
  yield();
}
