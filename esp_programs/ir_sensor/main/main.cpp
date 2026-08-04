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


// defining constants
#define TX 43
#define RX 44
#define UART_PORT_NUM UART_NUM_1
#define BAUD_RATE 921600
#define UART_BUF_SIZE (2048)

// initialising the uart driver.
// uart is used for communication with the main mcu (the teensy 4.1)
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

// sends whatever is in the array through uart
void send_float_array_as_bytes(float* arr, size_t len) {
    size_t total_bytes = len * sizeof(float);

    uart_write_bytes(UART_PORT_NUM, (const char*)arr, total_bytes);
}

// main function
extern "C" void app_main(void) {
    // initialise things.
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);


    // host Wifi Access Point for debugging purposes.
    wifi_config_t ap_config = {};
    strcpy((char*)ap_config.ap.ssid, "LEGACY_IR");
    // hey if you see this, dont leak the password!! big security threat.
    // I'll trust the robocup commitee :)
    strcpy((char*)ap_config.ap.password, "legacy_robot"); 
    ap_config.ap.ssid_len = strlen("LEGACY_IR");
    ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    ap_config.ap.max_connection = 1; // only 1 device can connect.
    ap_config.ap.channel = 1;

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_start();

    init_uart();

    // set up debugging led.
    // if led is on, the program is running,
    // it also means its not in bootloader mode
    Led debug_led = Led((gpio_num_t)45);
    debug_led.set_level(1);
    SensorRing irseeker = SensorRing();




    struct sockaddr_in dest_addr;
    
    dest_addr.sin_addr.s_addr = inet_addr("192.168.4.2"); // set debugging info to user laptop.
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
        
        irseeker.update(); 


        irseeker.get_sorted_vectors();
        std::array<float, 4> output = irseeker.advancedRead();

        // read stuff and then send.

        // dumping std array into traditional array
        // i was too lazy to make it compatible in the first place so boom here it is.
        float send_array[4]; // average of reading history
        for (uint8_t i = 0; i < 4; i ++) {
            send_array[i] = output[i];
        }


        // ========================================================
        float sensor_values[16] = {0};
        for (uint8_t i = 0; i < 16; i++) {
            sensor_values[i] = irseeker.sensor_vectors[i].m;
        } // used for debugging
        uart_write_bytes(UART_PORT_NUM, "e", 1);
        send_float_array_as_bytes(send_array, 4); // send stuff to teensy
        uart_wait_tx_done(UART_PORT_NUM, pdMS_TO_TICKS(100));


        sendto(sock,
                    sensor_values,
                    sizeof(sensor_values),
                    0,
                    (struct sockaddr *)&dest_addr,
                    sizeof(dest_addr)); // send stuff through WIFI UDP
        // =========================================================



    }
}