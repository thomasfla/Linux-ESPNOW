#include <esp_now.h>
#include <WiFi.h>

// Global copy of slave
esp_now_peer_info_t peer;
#define CHANNEL 1

#include <Ticker.h>  //Ticker Library

Ticker blinker;

static byte dest_mac[6] = {0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b}; //computers
//static byte dest_mac[6] ={0x84, 0xF3, 0xEB, 0x73, 0x55, 0x1E}; //ESP8266 (2) Echo mode


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

#define DATA_LEN 250
uint8_t data[DATA_LEN];

void init_data() {
  for(int i=0;i<DATA_LEN;i++) {
    data[i] = 0x14;
  }
}

// send data
void sendData() {
  data[0]++;
  Serial.print("Sending: "); Serial.println(data[0]);
  esp_err_t result = esp_now_send(peer.peer_addr, data, sizeof(data[0])*DATA_LEN);
  if (result != ESP_OK) {
    Serial.println("---------------AN ERROR OCCURED WHILE SENDING---------------");
  }
}

// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
 Serial.println(status == ESP_NOW_SEND_SUCCESS ? "" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("Reiceved from : ");
  Serial.println(macStr);
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
  
  blinker.attach(1, sendData);
}

void loop() {
  yield();
}
