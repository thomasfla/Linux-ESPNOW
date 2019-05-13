/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <string.h>
#include "protocol_examples_common.h"
#include "sdkconfig.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "tcpip_adapter.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "eth_phy/phy_lan8720.h"


#define HOST_IP_ADDR "192.168.142.1" //"192.168.142.255"

#define PORT_DEST 1111

#define DEVICE_IP       "192.168.142.2"
#define DEVICE_NETMASK  "255.255.255.0"
#define DEVICE_GW       "192.168.142.1"

#define CONFIG_PHY_CLOCK_MODE 3
#define ETHERNET_PHY_CONFIG phy_lan8720_default_ethernet_config
#define PIN_SMI_MDC 23
#define PIN_SMI_MDIO 18

#define WIFI_CONNECTED_BIT BIT0

static uint8_t dest_mac[6] = {0x00, 0x04, 0x23, 0xc5, 0xa8, 0x6b};
static uint8_t my_mac[6] = {0xb4, 0xe6, 0x2d, 0xb5, 0x9f, 0x88};

static const char *TAG = "example";
//static const char *payload = {0x1,0x2,0x3,0x4,0x5,0x6, 0x1,0x2,0x3,0x4,0x5,0x6, 'H','E','L','L','O',' ','W','O','R','L','D', '\0'};


static const char *payload = "Hello World";

typedef struct {
  uint8_t dst_mac[6];
  uint8_t src_mac[6];
  uint16_t ethertype;

  /* Custom payload*/
  uint16_t data_len;
  char data[127];

} eth_frame;

/* FreeRTOS event group to signal when we are connected to WiFi and ready to start UDP comm*/
EventGroupHandle_t udp_event_group;


static void udp_client_task(void *pvParameters)
{
    eth_frame my_frame;


    my_frame.ethertype = 0xb588;
    memcpy(my_frame.dst_mac, dest_mac, sizeof(uint8_t) * 6);  
    memcpy(my_frame.src_mac, my_mac, sizeof(uint8_t) * 6);
    my_frame.data_len = sizeof(char) * strlen(payload);
    strncpy(my_frame.data, payload, my_frame.data_len);

    while (1) {

      int err = esp_eth_tx((uint8_t *) &my_frame, my_frame.data_len + 16 * sizeof(uint8_t));
      if (err < 0) {
          ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
          break;
      }
      ESP_LOGI(TAG, "Message sent");

      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
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
    case SYSTEM_EVENT_STA_START:
      esp_wifi_connect();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      esp_wifi_connect();
      xEventGroupClearBits(udp_event_group, WIFI_CONNECTED_BIT);
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGI(TAG, "event_handler:SYSTEM_EVENT_STA_GOT_IP!");
      ESP_LOGI(TAG, "got ip:%s\n",
          ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
      xEventGroupSetBits(udp_event_group, WIFI_CONNECTED_BIT);
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      ESP_LOGI(TAG, "station:"MACSTR" join,AID=%d\n",
          MAC2STR(event->event_info.sta_connected.mac),
          event->event_info.sta_connected.aid);
      xEventGroupSetBits(udp_event_group, WIFI_CONNECTED_BIT);
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      ESP_LOGI(TAG, "station:"MACSTR"leave,AID=%d\n",
          MAC2STR(event->event_info.sta_disconnected.mac),
          event->event_info.sta_disconnected.aid);
      xEventGroupClearBits(udp_event_group, WIFI_CONNECTED_BIT);
      break;
    default:
      break;
  }
  return ESP_OK;
}

esp_err_t *my_recv_func (void *buffer, uint16_t len, void *eb) {
  printf("Data: %s\n" , ((eth_frame *) buffer)->data);
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

    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
}
