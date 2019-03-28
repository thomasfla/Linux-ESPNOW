#define TRY_ESP_ACTION(action, name) if(action == ESP_OK) {Serial.println("\t+ "+String(name));} else {Serial.println("----------Error while " + String(name) + " !---------------");}


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

  TRY_ESP_ACTION( esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE), "Set channel");

  TRY_ESP_ACTION( esp_wifi_internal_set_fix_rate(ESP_IF_WIFI_STA, true, DATARATE), "Fixed rate set up");

  TRY_ESP_ACTION( esp_now_init(), "ESPNow Init");

  TRY_ESP_ACTION(  esp_now_register_send_cb(OnDataSent), "Attach send callback");
  
  TRY_ESP_ACTION( esp_now_register_recv_cb(OnDataRecv), "Attach recv callback");
  
  memset(&peer, 0, sizeof(peer));

  for (int ii = 0; ii < 6; ++ii ) {
    peer.peer_addr[ii] = (uint8_t) dest_mac[ii];
  }
  
  peer.channel = CHANNEL; // pick a channel
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
