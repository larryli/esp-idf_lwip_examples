/* Raw Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/def.h"
#include "lwip/err.h"
#include "lwip/udp.h"

#define PORT CONFIG_EXAMPLE_PORT

static const char *TAG = "example";

static void raw_udp_recv_proc(void *arg, struct udp_pcb *upcb, struct pbuf *pb,
                              const ip_addr_t *addr, uint16_t port)
{
    char *addr_str = ipaddr_ntoa(addr);
    while (pb != NULL) {
        struct pbuf *this_pb = pb;
        pb = pb->next;
        this_pb->next = NULL;

        ESP_LOGI(TAG, "Received %d bytes from %s:", this_pb->len, addr_str);
        ESP_LOGI(TAG, "%s", (char *)this_pb->payload);

        udp_sendto(upcb, this_pb, addr, port);
        pbuf_free(this_pb);
    }
}

esp_err_t raw_udp_server_init(void)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Init raw udp server: %d", PORT);

    udp_init();
    struct udp_pcb *pcb = udp_new();
    if (pcb == NULL) {
        ESP_LOGE(TAG, "Init raw udp server failed!");
        return ESP_ERR_NO_MEM;
    }
    err_t ret = udp_bind(pcb, IP_ADDR_ANY, PORT);
    if (ret != ERR_OK) {
        ESP_LOGE(TAG, "Bind raw udp server failed: %d", ret);
        err = ESP_ERR_INVALID_STATE;
        goto failed;
    }
    udp_recv(pcb, &raw_udp_recv_proc, NULL);

    return ESP_OK;
    
failed:
    udp_remove(pcb);
    return err;
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    ESP_ERROR_CHECK(raw_udp_server_init());
}
