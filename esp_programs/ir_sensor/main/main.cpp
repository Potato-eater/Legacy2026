extern "C" {
    #include <stdio.h>
    #include <freertos/FreeRTOS.h>
    #include <freertos/task.h>
    #include <esp_timer.h>
    #include <esp_task_wdt.h>
    #include "math.h"
    #include <driver/uart.h>
}
#include "led.hpp"
#include "sensor.hpp"
#include "calc.hpp"
#include <array>
#include <string>
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
    esp_task_wdt_deinit();
    init_uart();
    Led debug_led = Led((gpio_num_t)45);
    debug_led.set_level(1);
    SensorRing irseeker = SensorRing();
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
        send_float_array_as_bytes(sensor_values, 16);
        uart_wait_tx_done(UART_PORT_NUM, pdMS_TO_TICKS(100));
        send_array[0] = send_array[0] * 180 / M_PI;
        // =========================================================



    }
}