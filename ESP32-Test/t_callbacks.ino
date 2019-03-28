void sendData() {
  if(n_sent < N_BATCH) {
    long mytime = micros();
    int packet_nb = n_sent - error_send;
    memcpy(txData, &mytime, sizeof(mytime));
    memcpy(txData+sizeof(mytime), &packet_nb, sizeof(packet_nb));
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
  int packet_nb;
      
    if(len>sizeof(sendTime)) {
      memcpy(&sendTime, rxData, sizeof(sendTime));
      memcpy(&packet_nb, rxData+sizeof(sendTime), sizeof(packet_nb));

      if(sendTime>batchStart) {
        fill_histo(receiveTime - sendTime, packet_nb);
      }
    } else {
      error_recv++;
    }
}
