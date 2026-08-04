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
#include "constants.hpp"
#include "led.hpp"
#include "sensor.hpp"
#include "calc.hpp"


#define THRESHOLD 900





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
void find_other_mac_address(uint8_t* this_mac, uint8_t* other_mac) {
    // \xD8\x3B\xDA\xC6\xD1\xC8: isabelle
    // \xD8\x3B\xDA\xC6\xD1\xC0: ryan
    // \xD8\x3B\xDA\xC6\xD1\xCA: debugger


    esp_read_mac(this_mac, ESP_MAC_WIFI_STA);
    if (this_mac[5] == 0xC8) {
        memcpy(other_mac, "\xD8\x3B\xDA\xC6\xD1\xC0", 6); // ryan's mac
    } else {
        memcpy(other_mac, "\xD8\x3B\xDA\xC6\xD1\xC8", 6); // isabelle's mac
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

void init_esp_now(uint8_t* send_mac, uint8_t* debugger_mac) {
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
    esp_now_add_peer(&peerInfo);
    memcpy(peerInfo.peer_addr, debugger_mac, 6);
    esp_now_add_peer(&peerInfo);
}

extern "C" void app_main(void) {
    uint8_t debugger_mac[6];
    memcpy(debugger_mac, "\xD8\x3B\xDA\xC6\xD1\xCA", 6);
    // debugger is another esp32 device that we connect to our laptop to debug. currently hard coded mac address.
    // reason why we have debugger is that we do not want the line sensor device to waste power on hosting a wifi
    // this device is already designed to do communicaiton with the other robot. the debugger just "spies" on the
    // data.



    Led debug_led((gpio_num_t)45);
    init_uart();
    SensorRing ring;

    // defining the angles of the sensors. i should have used radians but oh well im too lazy to change it.
    std::array<float, 16> sensor_angles = {180, -157.5, -135, -112.5, -90, -67.5, -45, -22.5, 0, 22.5, 45, 67.5, 90, 112.5, 135, 157.5};
    std::array<MathVector, 16> sensor_vectors;
    for (u_int8_t i = 0; i < 16; i++) {
        sensor_vectors[i] = MathVectorFromArgument(6.8, sensor_angles[i] * M_PI/180.0);
    }

    // init values
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

    uint8_t this_mac[6];
    uint8_t other_mac[6];
    find_other_mac_address(this_mac, other_mac); // finding the mac address of the device that should receive the data.
    init_esp_now(other_mac, debugger_mac);
    while (true) { // repeat forever. no reason to stop.
        ring.readRaw(result, adc_values, output_data);
        
        int32_t tcrt_values[CHANNEL_NUM] = { // since we need to read from the ADC, and that the pins are mismatched, we have to do this.
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
            if (tcrt_values[i] <= THRESHOLD) {
                led_on = true;
                break;
            }
        }
        if (led_on) {
            debug_led.set_level(1);
        } // if the sensor sees the line, then turn on the led
        else {
            debug_led.set_level(0);
        }

        std::array<bool, CHANNEL_NUM> whites = find_white(tcrt_values, THRESHOLD);
        std::array<float, 2> result = find_line(whites, sensor_vectors); // find the final result

        float send_value[2] = {result[0], result[1]}; // direction and distance
        
        uart_wait_tx_done(UART_PORT_NUM, pdMS_TO_TICKS(1));
        BotData teensy_data; // reading what the main mcu is sending this sensor

        if (read_teensy(&teensy_data)) {
            // do nothing rn, but if we want debugging info, put stuff here.
        }

        
        if (data_received) {
            received_data = incoming_data;

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
        send_float_array_as_bytes(send_value, 2);
        uart_write_bytes(UART_PORT_NUM, (const uint8_t*)&received_data, sizeof(received_data));
        uart_write_bytes(UART_PORT_NUM, (const bool*)&data_received, sizeof(bool));

        esp_err_t send_ret = esp_now_send(NULL, (uint8_t*)&teensy_data, sizeof(teensy_data));
        
        end_time = esp_timer_get_time();
        printf("time: %lld ms ", (end_time - start_time) / 1000);
        for (int i = 0; i < 6; i++) {
            printf("\\%X", this_mac[i]);
        }
        printf(" %d ", data_received);
        printf(" ");
        for (int i = 0; i < CHANNEL_NUM; i++) {
            printf("%1d: %ld ", i + 1, tcrt_values[i]);
        }
        printf("\n");
        fflush(stdout);
        data_received = false;
        start_time = esp_timer_get_time();
    }
}
