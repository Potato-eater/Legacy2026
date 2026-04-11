#include "sensor.hpp"
SensorRing::SensorRing() {
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 512,
        .conv_frame_size = EXAMPLE_READ_LEN,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle));

    adc_continuous_config_t adc_pattern_config = {
        .sample_freq_hz = 1000 * 16,  // Faster rate, since more channels
        .conv_mode = ADC_CONV_BOTH_UNIT,
        .format = EXAMPLE_ADC_OUTPUT_TYPE,
    };

    adc_digi_pattern_config_t pattern[CHANNEL_NUM] = {0};
    int index = 0;

    for (int i = 0; i < 10; i++) {
        pattern[index].atten = ADC_ATTEN_DB_12;
        pattern[index].channel = adc1_channels[i];
        pattern[index].unit = ADC_UNIT_1;
        pattern[index].bit_width = ADC_BITWIDTH_12;
        index++;
    }

    for (int i = 0; i < 6; i++) {
        pattern[index].atten = ADC_ATTEN_DB_12;
        pattern[index].channel = adc2_channels[i];
        pattern[index].unit = ADC_UNIT_2;
        pattern[index].bit_width = ADC_BITWIDTH_12;
        index++;
    }

    adc_pattern_config.pattern_num = CHANNEL_NUM;
    adc_pattern_config.adc_pattern = pattern;

    ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &adc_pattern_config));
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));
}

void SensorRing::readRaw(uint8_t result[EXAMPLE_READ_LEN], int32_t adc_values[2][16], adc_digi_output_data_t *output_data) {
    uint32_t ret_bytes = 0;
    esp_err_t ret = adc_continuous_read(adc_handle, result, EXAMPLE_READ_LEN, &ret_bytes, 100);
    if (ret == ESP_OK) {
        memset(adc_values, -1, sizeof(adc_values));
        for (int i = 0; i < ret_bytes; i += sizeof(adc_digi_output_data_t)) {
            output_data = (adc_digi_output_data_t*)&result[i];
            int unit = output_data->type2.unit;      // 0 = ADC1, 1 = ADC2
            int channel = output_data->type2.channel;
            int data = output_data->type2.data;
            adc_values[unit][channel] = (int32_t)data;
        }
    }
}