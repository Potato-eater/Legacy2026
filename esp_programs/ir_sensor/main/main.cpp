extern "C" {
    #include <stdio.h>
    #include <freertos/FreeRTOS.h>
    #include <freertos/task.h>
    #include <esp_timer.h>
    #include <esp_task_wdt.h>
    #include "math.h"
    #include <driver/uart.h>
    #include "esp_wifi.h"
    #include "esp_event.h"
    #include "esp_mac.h"

    #include "sys/socket.h"
    #include "netinet/in.h"
    #include "arpa/inet.h"
    #include "lwip/err.h"
    #include "lwip/sys.h"
    #include "esp_log.h"
    #include "nvs_flash.h"
}
#include "led.hpp"
#include "sensor.hpp"
#include "calc.hpp"
#include <array>
#include <string>

static const char *TAG = "IR SENSOR";



#define TX 43
#define RX 44
#define UART_PORT_NUM UART_NUM_1
#define BAUD_RATE 921600
#define UART_BUF_SIZE (2048)


void init_uart() {
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_param_config(UART_PORT_NUM, &uart_config);
    uart_set_pin(UART_PORT_NUM, TX, RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0);
}
void send_float_array_as_bytes(float* arr, size_t len) {
    size_t total_bytes = len * sizeof(float);

    uart_write_bytes(UART_PORT_NUM, (const char*)arr, total_bytes);
}
extern "C" void app_main(void) {
    // esp_task_wdt_deinit();

    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t ap_config = {};
    strcpy((char*)ap_config.ap.ssid, "LEGACY_IR");
    strcpy((char*)ap_config.ap.password, "legacy_robot");
    ap_config.ap.ssid_len = strlen("LEGACY_IR");
    ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    ap_config.ap.max_connection = 1;
    ap_config.ap.channel = 1;

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_start();

    init_uart();
    Led debug_led = Led((gpio_num_t)45);
    debug_led.set_level(1);
    SensorRing irseeker = SensorRing();




    struct sockaddr_in dest_addr;
    
    dest_addr.sin_addr.s_addr = inet_addr("192.168.4.2");
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(1234);
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket");
        vTaskDelete(NULL);
        return;
    }



    // std::array<std::array<float, 4>, 5> reading_history = {0};
    // reading_history.fill({0, 0, 0, 0});
    while (1) {
        
        // irseeker.update_mode_2(100);
        // printf("%lld ", esp_timer_get_time() - start_time);
        
        irseeker.update();
        // irseeker.sensor_vectors[0].m = 0;
        // irseeker.sensor_vectors[1].m = 0;
        // irseeker.sensor_vectors[2].m = 0;
        // irseeker.sensor_vectors[3].m = 0;
        // irseeker.sensor_vectors[4].m = 0;
        // irseeker.sensor_vectors[5].m = 0;
        // irseeker.sensor_vectors[6].m = 0;

        // irseeker.sensor_vectors[14].m = 0;
        // irseeker.sensor_vectors[15].m = 0;

        irseeker.get_sorted_vectors();
        std::array<float, 4> output = irseeker.advancedRead();
        float send_array[4]; // average of reading history
        for (uint8_t i = 0; i < 4; i ++) {
            send_array[i] = output[i];
        }


        // ========================================================
        float sensor_values[16] = {0};
        for (uint8_t i = 0; i < 16; i++) {
            sensor_values[i] = irseeker.sensor_vectors[i].m;
        }
        uart_write_bytes(UART_PORT_NUM, "e", 1);
        send_float_array_as_bytes(send_array, 4);
        // send_float_array_as_bytes(sensor_values, 16);
        uart_wait_tx_done(UART_PORT_NUM, pdMS_TO_TICKS(100));
        // send_array[0] = send_array[0] * 180 / M_PI;


        sendto(sock,
                    sensor_values,
                    sizeof(sensor_values),
                    0,
                    (struct sockaddr *)&dest_addr,
                    sizeof(dest_addr));
        // =========================================================



    }
}