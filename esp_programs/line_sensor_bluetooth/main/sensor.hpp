#pragma once
extern "C" {
    #include "esp_adc/adc_continuous.h"
    #include "esp_log.h"
    #include "esp_err.h"
    #include <stdio.h>
    #include <string.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    // #include "hal/adc_hal.h"
}
#include <constants.hpp>

static adc_channel_t adc1_channels[] = {ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_3, ADC_CHANNEL_4, ADC_CHANNEL_5, ADC_CHANNEL_6, ADC_CHANNEL_7, ADC_CHANNEL_8, ADC_CHANNEL_9};
static adc_channel_t adc2_channels[] = {ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_3, ADC_CHANNEL_4, ADC_CHANNEL_5};

static adc_continuous_handle_t adc_handle = NULL;

class SensorRing {
    public:
    SensorRing();
    void readRaw(uint8_t result[EXAMPLE_READ_LEN], int32_t adc_values[2][16], adc_digi_output_data_t *output_data);
};