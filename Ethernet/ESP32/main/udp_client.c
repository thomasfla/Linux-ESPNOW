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

static uint8_t my_mac[6] = {0x1,0x2,0x3,0x4,0x5,0x6};

static const char *TAG = "example";
static const char *payload = "Message from ESP32 ";


/* FreeRTOS event group to signal when we are connected to WiFi and ready to start UDP comm*/
EventGroupHandle_t udp_event_group;


static void udp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {

        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT_DEST);

        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT_DEST);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
            perror("Error");
        }

        char broadcast = '1';
        setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast));

        while (1) {

            int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent");

            struct sockaddr_in source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);
            }

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
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
        config.tcpip_input = tcpip_adapter_eth_input;
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
