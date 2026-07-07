extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "esp_log.h"
    #include "esp_err.h"
    #include "esp_now.h"
    #include "esp_wifi.h"
    #include "esp_event.h"
    #include "esp_mac.h"
    #include "esp_timer.h"
    #include "nvs_flash.h"

    #include <stdio.h>
    #include <string.h>
}



struct Vector {
    float i;
    float j;
};
struct BotData {
    float heading;
    Vector pos_vector;
    float ball_strength;
    float ball_angle;
    Vector line_vector;
    Vector velocity;
    int yellow_x;
};

BotData incoming_data = {0, {0, 0}, 0, 0, {0, 0}, {0, 0}};
bool data_received = false;
uint8_t incoming_mac[6] = {0, 0, 0, 0, 0, 0};

void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // printf("message sent -> %s \n", status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
    
}
void on_data_recv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (len == sizeof(incoming_data)) {
        memcpy(&incoming_data, data, sizeof(incoming_data));
        data_received = true;
        memcpy(&incoming_mac, recv_info->src_addr, sizeof(incoming_mac));
        // incoming_mac = recv_info->src_addr;
        // printf("received data from %02X:%02X:%02X:%02X:%02X:%02X\n",
        //        mac_addr[0], mac_addr[1], mac_addr[2],
        //        mac_addr[3], mac_addr[4], mac_addr[5]);
    }
}

void init_esp_now() {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Set Wi-Fi channel to 1 (or the channel your receiver listens to)
    ESP_ERROR_CHECK(esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(on_data_sent));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_recv));

}
extern "C" void app_main(void)
{
    init_esp_now();
    BotData received_data = {
        .heading = 0.0,
        .pos_vector = {0.0, 0.0},
        .ball_strength = 0.0,
        .ball_angle = 0.0,
        .line_vector = {0.0, 0.0},
        .velocity = {0.0, 0.0}
    };

    int not_received = 0;

    while (true) {
        if (data_received) {
            received_data = incoming_data;
            data_received = false;
            not_received = 0;
        }
        else {
            not_received++;
        }

        if (not_received > 1000) {
            received_data = {
                .heading = 0.0,
                .pos_vector = {0.0, 0.0},
                .ball_strength = 0.0,
                .ball_angle = 0.0,
                .line_vector = {0.0, 0.0},
                .velocity = {0.0, 0.0}
            };
            not_received = 1000;
            // printf("not receiving anything.");
        }

        const char header[3] = {'e', 'e', 'e'};
        const char footer[3] = {'f', 'f', 'f'};
        const char mac_last_num = incoming_mac[5];
        fwrite(&header, sizeof(char), 3, stdout);
        fwrite(&received_data, sizeof(uint8_t), sizeof(received_data), stdout);
        fwrite(&incoming_mac[5], sizeof(uint8_t), 1, stdout);
        fwrite(&footer, sizeof(char), 3, stdout);
        // printf("%.2f\n", received_data.heading);
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(1));


    };
}
