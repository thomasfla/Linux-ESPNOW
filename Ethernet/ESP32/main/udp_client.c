#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include "sdkconfig.h"
#include "esp_eth.h"

#include "eth_phy/phy_lan8720.h"






#if CONFIG_DATARATE_6
  #define CONFIG_DATARATE WIFI_PHY_RATE_6M
#elif CONFIG_DATARATE_9
  #define CONFIG_DATARATE WIFI_PHY_RATE_9M
#elif CONFIG_DATARATE_12
  #define CONFIG_DATARATE WIFI_PHY_RATE_12M
#elif CONFIG_DATARATE_18
  #define CONFIG_DATARATE WIFI_PHY_RATE_18M
#elif CONFIG_DATARATE_24
  #define CONFIG_DATARATE WIFI_PHY_RATE_24M
#elif CONFIG_DATARATE_36
  #define CONFIG_DATARATE WIFI_PHY_RATE_36M
#elif CONFIG_DATARATE_48
  #define CONFIG_DATARATE WIFI_PHY_RATE_48M
#elif CONFIG_DATARATE_56
  #define CONFIG_DATARATE WIFI_PHY_RATE_56M
#else
  #define CONFIG_DATARATE WIFI_PHY_RATE_ERROR
#endif




#define recv_list_len CONFIG_BATCH_SIZE/6
#define packet_received(p) recv_list[p/6] |= 1<<p%6

char base[64] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

uint8_t recv_list[recv_list_len];
int histogram[CONFIG_HISTO_N_STEP];
int histogram_higher;
int histogram_lower;
int error_recv;
int receiveNb;
int error_send;
double avg;

long batchStart =0;

long lastMsgSent = 0;

unsigned long n_sent;



#define DEVICE_IP       "192.168.1.0"
#define DEVICE_NETMASK  "255.255.255.0"
#define DEVICE_GW       "192.168.1.1"

#define CONFIG_PHY_CLOCK_MODE 3
#define ETHERNET_PHY_CONFIG phy_lan8720_default_ethernet_config
#define PIN_SMI_MDC 23
#define PIN_SMI_MDIO 18

#define WIFI_CONNECTED_BIT BIT0

static uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};//{0x00, 0x04, 0x23, 0xc5, 0xa8, 0x6b};
static uint8_t my_mac[6] = {0xb4, 0xe6, 0x2d, 0xb5, 0x9f, 0x88};

static const char *TAG = "example";

static const char *payload = "Hello World";

typedef struct {
  uint8_t dst_mac[6];
  uint8_t src_mac[6];
  uint16_t ethertype;

  /* Custom payload*/
  uint16_t data_len;
  char data[127];

} eth_frame;

eth_frame my_frame;

/* FreeRTOS event group to signal when we are connected to WiFi and ready to start UDP comm*/
EventGroupHandle_t udp_event_group;




void init_data() {
  my_frame.ethertype = 0xb588;
  memcpy(my_frame.dst_mac, dest_mac, sizeof(uint8_t) * 6);  
  memcpy(my_frame.src_mac, my_mac, sizeof(uint8_t) * 6);
}

void init_histo() {
  for(int i=0;i<recv_list_len;i++) {
    recv_list[i] = 0;
  }
  for(int i=0;i<CONFIG_HISTO_N_STEP;i++) {
    histogram[i] = 0;
  }
  histogram_higher = 0;
  histogram_lower = 0;
  error_recv=0;
  error_send=0;
  receiveNb=0;
  avg=0;
}

void fill_histo(long value, int packet_nb) {
  int index = (value-CONFIG_HISTO_INF) * CONFIG_HISTO_N_STEP / (CONFIG_HISTO_SUP - CONFIG_HISTO_INF);
  if(index >= CONFIG_HISTO_N_STEP) {
    histogram_higher++;
  } else if(index < 0) {
    histogram_lower++;
  } else {
    histogram[index]++;
  }

  packet_received(packet_nb);
  receiveNb++;
  avg += value;
}

void print_histo() {
  printf("----------\n");
  printf("Units :\tus");
  printf("\n");
  printf("Bounds :\t%d\t%d", CONFIG_HISTO_INF, CONFIG_HISTO_SUP);
  printf("\n");
  printf("Nb_values :\t%d", CONFIG_HISTO_N_STEP);
  printf("\n");
  for(int i=0;i<CONFIG_HISTO_N_STEP;i++) {
    printf("%d\t", histogram[i]);
  }
  printf("%d",histogram_higher);
  
  printf("\n");
  printf("Average :\t%d\t", receiveNb != 0 ? (int) avg/receiveNb : -1);
  printf("\n");
  printf("Received :\t%d", receiveNb);
  printf("\n");
  printf("Sent :\t%ld", n_sent-error_send);
  printf("\n");
  for(int i=0;i<recv_list_len;i++) {
    printf("%c",base[recv_list[i]]);
  }
  printf("\n");
}




void sendData()
{
  long mytime = esp_timer_get_time();
  int packet_nb = n_sent - error_send;
  memcpy(my_frame.data, &mytime, sizeof(mytime));
  memcpy(my_frame.data+sizeof(mytime), &packet_nb, sizeof(packet_nb));
  my_frame.data_len = 127;
  int err = esp_eth_tx((uint8_t *) &my_frame, my_frame.data_len + 16 * sizeof(uint8_t));
  if (err < 0) {
    error_send++;
    //ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
  }
  n_sent++;
  lastMsgSent = mytime;
  //ESP_LOGI(TAG, "Message sent");
}

static void eth_gpio_config_rmii(void) {
    // RMII data pins are fixed:
    // TXD0 = GPIO19
    // TXD1 = GPIO22
    // TX_EN = GPIO21
    // RXD0 = GPIO25
    // RXD1 = GPIO26
    // CLK == GPIO0
    phy_rmii_configure_data_interface_pins();
    phy_rmii_smi_configure_pins(PIN_SMI_MDC, PIN_SMI_MDIO);
}


static esp_err_t event_handler(void *ctx, system_event_t *event) {
  switch(event->event_id) {
    case SYSTEM_EVENT_ETH_START:
      ESP_LOGI(TAG, "event_handler:ETH_START!");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      ESP_LOGI(TAG, "event_handler:ETHERNET_GOT_IP!");
      //ESP_LOGI(TAG, "got ip:%s\n",
      //    ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
      xEventGroupSetBits(udp_event_group, WIFI_CONNECTED_BIT);
      break;
    default:
      break;
  }
  return ESP_OK;
}

static esp_err_t *my_recv_func (void *buffer, uint16_t len, void *eb) {
  long receiveTime = esp_timer_get_time();
  long sendTime;
  int packet_nb;

  if(len>sizeof(sendTime)) {
    memcpy(&sendTime, ((eth_frame *) buffer)->data, sizeof(sendTime));
    memcpy(&packet_nb, ((eth_frame *) buffer)->data+sizeof(sendTime), sizeof(packet_nb));

    if(sendTime>batchStart) {
      fill_histo(receiveTime - sendTime, packet_nb);
    }
    //sendData();
  } else {
    error_recv++;
  }

  return ESP_OK;
}


void init_eth() {
    ////////////////////////////////////
    //TCP/IP DRIVER INIT WITH A STATIC IP
    ////////////////////////////////////
        tcpip_adapter_init();
        tcpip_adapter_ip_info_t ipInfo;


    ////////////////////////////////////
    //EVENT HANDLER (CALLBACK)
    ////////////////////////////////////
        //TCP/IP event handling & group (akin to flags and semaphores)
        udp_event_group = xEventGroupCreate();
        ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    ////////////////////////////////////
    //ETHERNET CONFIGURATION & INIT
    ////////////////////////////////////
        //have stop DHCP 
        tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);

        //set the static IP
        ip4addr_aton(DEVICE_IP, &ipInfo.ip);
        ip4addr_aton(DEVICE_GW, &ipInfo.gw);
        ip4addr_aton(DEVICE_NETMASK, &ipInfo.netmask);
        ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &ipInfo));

        eth_config_t config = ETHERNET_PHY_CONFIG;
        config.phy_addr = PHY1;
        config.gpio_config = eth_gpio_config_rmii;
        config.tcpip_input = my_recv_func;
        config.clock_mode = CONFIG_PHY_CLOCK_MODE;

        ESP_ERROR_CHECK(esp_eth_init(&config));
        ESP_ERROR_CHECK(esp_eth_enable());
        ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ipInfo));

    ////////////////////////////////////
    //TCP\IP INFORMATION PRINT
    ////////////////////////////////////
        ESP_LOGI(TAG, "TCP/IP initialization finished.");


        ESP_LOGI(TAG, "TCP|IP \t IP:"IPSTR, IP2STR(&ipInfo.ip));
        ESP_LOGI(TAG, "TCP|IP \t MASK:"IPSTR, IP2STR(&ipInfo.netmask));
        ESP_LOGI(TAG, "TCP|IP \t GW:"IPSTR, IP2STR(&ipInfo.gw));
}

void app_main()
{
    init_eth();

    ESP_LOGI(TAG, "Establishing connetion");

    xEventGroupWaitBits(udp_event_group, WIFI_CONNECTED_BIT,true, true, portMAX_DELAY);

    ESP_LOGI(TAG, "Connected");

    n_sent = 0;
    init_data();

    while(true) {
      if(n_sent >= CONFIG_BATCH_SIZE) {
        vTaskDelay(2000 / portTICK_RATE_MS);;
        
        print_histo();
        init_histo();
        
        n_sent=0;
        batchStart = esp_timer_get_time();
        
      } else {
        if(esp_timer_get_time() - lastMsgSent > 1000) {
          sendData();
        } else {
          //taskYIELD();
        }
      }
    }
}
