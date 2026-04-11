#include "Led.hpp"

Led::Led(gpio_num_t gpio_num) {
    led_pin = gpio_num;
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << led_pin),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&config);
}

void Led::set_level(bool level) {
    gpio_set_level(this->led_pin, level);
}

void Led::toggle() {
    set_level(!gpio_get_level(this->led_pin));
}
