extern "C" {
    #include <stdio.h>
    #include <string.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "esp_log.h"
    #include "esp_err.h"
    #include "esp_adc/adc_continuous.h"
    #include "driver/uart.h"
    #include "esp_now.h"
    #include "esp_wifi.h"
    #include "esp_event.h"
    #include "esp_mac.h"
    // #include "driver/gpio.h"
    // #include "esp_wifi.h"
    #include "esp_timer.h"
    #include "nvs_flash.h"
}
// #include "pins.hpp"
#include "constants.hpp"
#include "led.hpp"
#include "sensor.hpp"
#include "calc.hpp"
// static const char *TAG = "wifi_ap";


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
void send_int_array_as_bytes(int32_t* arr, size_t len) {
    // Calculate the total size (number of bytes) to send
    size_t total_bytes = len * sizeof(int);

    // Send the array as raw bytes over UART
    uart_write_bytes(UART_PORT_NUM, (const char*)arr, total_bytes);
}

void send_float_array_as_bytes(float* arr, size_t len) {
    size_t total_bytes = len * sizeof(float);

    uart_write_bytes(UART_PORT_NUM, (const char*)arr, total_bytes);
}

bool read_teensy(BotData* teensy_data) {
    uint8_t buffer[256];
    const int payload_bytes = sizeof(BotData);
    int len = uart_read_bytes(UART_PORT_NUM, buffer, sizeof(buffer), pdMS_TO_TICKS(1));
    // printf("read %d bytes\n", len);
    if (len <= 0) {
        return false;  // nothing read
    }

    int start_index = -1;
    for (int i = len - payload_bytes - 1; i >= 0; i--) {
        if (buffer[i] == 'd' && (i + 1 + payload_bytes <= len)) {
            start_index = i;
            break;
        }
    }

    if (start_index == -1) {
        *teensy_data = {0, {0, 0}, 0, 0, {0, 0}, {0, 0}};
        return false;
    }

    const uint8_t* data_ptr = &buffer[start_index + 1];
    memcpy(teensy_data, data_ptr, payload_bytes);
    return true;
}
void find_other_mac_address(uint8_t* other_mac) {
    // david's line sensor mac address: D8:3B:DA:C6:D1:D4
    // bobbler's line sensor mac address: D8:3B:DA:C6:D1:CE

    uint8_t own_mac[6];
    esp_read_mac(own_mac, ESP_MAC_WIFI_STA);
    if (own_mac[5] == 0xD4) {
        memcpy(other_mac, "\xD8\x3B\xDA\xC6\xD1\xCE", 6); // bobbler's mac
    } else {
        memcpy(other_mac, "\xD8\x3B\xDA\xC6\xD1\xD4", 6); // david's mac
    }
}
void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // printf("message sent -> %s \n", status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
    
}
void on_data_recv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (len == sizeof(incoming_data)) {
        memcpy(&incoming_data, data, sizeof(incoming_data));
        data_received = true;
        const uint8_t *mac_addr = recv_info->src_addr;
        // printf("received data from %02X:%02X:%02X:%02X:%02X:%02X\n",
        //        mac_addr[0], mac_addr[1], mac_addr[2],
        //        mac_addr[3], mac_addr[4], mac_addr[5]);
    }
}

void init_esp_now(uint8_t* send_mac) {
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

    // Add broadcast peer explicitly (for compatibility)
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, send_mac, 6);
    peerInfo.channel = 1;  // same channel as set above
    peerInfo.encrypt = false;
    esp_err_t ret = esp_now_add_peer(&peerInfo);
    if (ret == ESP_ERR_ESPNOW_EXIST) {
        printf("sending peer already added");
    } else if (ret != ESP_OK) {
        printf("Failed to add broadcast peer: %d", ret);
        return;
    }
}

extern "C" void app_main(void) {

    Led debug_led((gpio_num_t)45);
    init_uart();
    SensorRing ring;
    // ESP_ERROR_CHECK(nvs_flash_init());

    std::array<float, 16> sensor_angles = {180, -157.5, -135, -112.5, -90, -67.5, -45, -22.5, 0, 22.5, 45, 67.5, 90, 112.5, 135, 157.5};
    std::array<MathVector, 16> sensor_vectors;
    for (u_int8_t i = 0; i < 16; i++) {
        sensor_vectors[i] = MathVectorFromArgument(6.8, sensor_angles[i] * M_PI/180.0);
    }
    uint8_t result[EXAMPLE_READ_LEN];
    adc_digi_output_data_t *output_data = NULL;
    int32_t adc_values[2][16] = {{-1}}; // [unit][channel], safe with overprovisioned index
    int64_t start_time = esp_timer_get_time();
    int64_t end_time = esp_timer_get_time();
    int32_t not_received = 0;

    BotData received_data = {
        .heading = 0.0,
        .pos_vector = {0.0, 0.0},
        .ball_strength = 0.0,
        .ball_angle = 0.0,
        .line_vector = {0.0, 0.0},
        .velocity = {0.0, 0.0}
    };


    uint8_t other_mac[6];
    find_other_mac_address(other_mac);
    init_esp_now(other_mac);
    while (true) {
        ring.readRaw(result, adc_values, output_data);
        
        int32_t tcrt_values[CHANNEL_NUM] = {
            adc_values[0][TCRT1],
            adc_values[0][TCRT2],
            adc_values[0][TCRT3],
            adc_values[0][TCRT4],
            adc_values[0][TCRT5],
            adc_values[1][TCRT6],
            adc_values[1][TCRT7],
            adc_values[1][TCRT8],
            adc_values[1][TCRT9],
            adc_values[1][TCRT10],
            adc_values[1][TCRT11],
            adc_values[0][TCRT12],
            adc_values[0][TCRT13],
            adc_values[0][TCRT14],
            adc_values[0][TCRT15],
            adc_values[0][TCRT16],
        };
        bool led_on = false;
        for (uint8_t i = 0; i < CHANNEL_NUM; i++) {
            if (tcrt_values[i] <= 800) {
                led_on = true;
                break;
            }
        }
        if (led_on) {
            debug_led.set_level(1);
        }
        else {
            debug_led.set_level(0);
        }
        // while (true) {
        //     char read_value[1];
        //     uart_read_bytes(UART_PORT_NUM, &read_value, 1, pdMS_TO_TICKS(1000));
        //     if (read_value[0] == 'e') {
        //         break;
        //     }
        //     esp_restart();
        // }

        

        std::array<bool, CHANNEL_NUM> whites = find_white(tcrt_values, 800);
        // std::array<bool, CHANNEL_NUM> whites = {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
        std::array<float, 2> result = find_line(whites, sensor_vectors);

        float send_value[2] = {result[0], result[1]};
        
        uart_wait_tx_done(UART_PORT_NUM, pdMS_TO_TICKS(1));
        // printf("%s", "should have writen the stuff");
        // for (int i = 0; i < CHANNEL_NUM; i++) {
        //     printf("%d, ", whites[i]);
        // }
        BotData teensy_data;
        // bool read_teensy_success = false;
        if (read_teensy(&teensy_data)) {
            // printf("pos %.2f %.2f \n", teensy_data.pos_vector.i, teensy_data.pos_vector.j);
        }

        
        if (data_received) {
            received_data = incoming_data;
            data_received = false;
            not_received = 0;
        }
        else {
            not_received++;
        }

        if (not_received > 5) {
            received_data = {
                .heading = 0.0,
                .pos_vector = {0.0, 0.0},
                .ball_strength = 0.0,
                .ball_angle = 0.0,
                .line_vector = {0.0, 0.0},
                .velocity = {0.0, 0.0}
            };
            not_received = 5;
            // printf("probably disconnected");
        }
        uart_write_bytes(UART_PORT_NUM, "e", 1);
        // send_int_array_as_bytes(tcrt_values, CHANNEL_NUM);
        send_float_array_as_bytes(send_value, 2);
        uart_write_bytes(UART_PORT_NUM, (const uint8_t*)&received_data, sizeof(received_data));

        esp_err_t send_ret = esp_now_send(other_mac, (uint8_t*)&teensy_data, sizeof(teensy_data));
        
        // printf("%d", read_teensy(&teensy_data));
        end_time = esp_timer_get_time();
        // printf(" received xy %.2f %.2f not_received %ld ", received_data.pos_vector.i, received_data.pos_vector.j, not_received); // use this to see if it is receiving data
        printf("time: %lld ms", (end_time - start_time) / 1000);
        for (int i = 0; i < CHANNEL_NUM; i++) {
            printf("%1d: %ld ", i + 1, tcrt_values[i]);
        }
        // printf("%f %f ", send_value[0], send_value[1]);
        // printf("angle: %.5f, distance: %4f", result[0], result[1]);
        printf("\n");
        fflush(stdout);
        
        start_time = esp_timer_get_time();
    }
        // vTaskDelay(pdMS_TO_TICKS(15));
}
