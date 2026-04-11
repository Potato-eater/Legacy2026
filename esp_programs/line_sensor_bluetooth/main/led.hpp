#ifndef LED_HPP
#define LED_HPP
extern "C" {
    #include "driver/gpio.h"
}


class Led {
    public:
        gpio_num_t led_pin;  // GPIO pin number for the LED
        
        // Constructor that initializes the LED on the given GPIO pin
        Led(gpio_num_t gpio_num);

        // Method to set the LED state (on or off)
        void set_level(bool level);

        // Method to toggle the LED state
        void toggle();
};

#endif // LED_HPP
