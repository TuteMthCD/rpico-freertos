#include "FreeRTOS.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "task.h"
#include <cstdint>
#include <stdbool.h>
#include <stdio.h>
#include <vector>

#define SDA 0
#define SCL 1

#define NUM_PWM 16


void i2c_task(void*) {
    i2c_init(i2c0, 400000);
    gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_set_function(SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SDA);
    gpio_pull_up(SCL);

    std::vector<uint8_t> buff = {
        0x00 | 0x80, // select reg mode0 auto increment

        0x00, // mode 1
        0x00, // mode 2
        //
        0x00, // pwm 0
        0x00, // pwm 1
        0x00, // pwm 2
        0x00, // pwm 3
        0x00, // pwm 4
        0x00, // pwm 5
        0x00, // pwm 6
        0x00, // pwm 7
        0x00, // pwm 8
        0x00, // pwm 9
        0x00, // pwm 10
        0x00, // pwm 11
        0x00, // pwm 12
        0x00, // pwm 13
        0x00, // pwm 14
        0x00, // pwm 15
        //
        0x00, // GRPPWM
        0x00, // GRPFREQ
        //
        0xAA, // LEDOUT 0
        0xAA, // LEDOUT 1
        0xAA, // LEDOUT 2
        0xAA, // LEDOUT 3
        //
        0x00, // SUBADDR1
        0x00, // SUBADDR2
        0x00, // SUBADDR3
        0x00, // ALLCALLADDR
        //
        0xFF, // IREF
        0x00, // EFLAG 1
        0x00  // EFLAG 2
    };

    i2c_write_blocking(i2c0, 0x60, buff.data(), buff.size(), false);

    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    while(true) {

        uint8_t pwm_value = adc_read() >> 4;

        std::vector<uint8_t> set_pwm = { 0x02 | 0x80 }; // select reg 0x02, auto-increment
        for(int i = 0; i < NUM_PWM; i++) {
            set_pwm.push_back(pwm_value);
        }
        i2c_write_blocking(i2c0, 0x60, set_pwm.data(), set_pwm.size(), false);


        vTaskDelay(10);
    }
}

void led_task(void*) {
    bool pin = 0;
    while(true) {
        pin = !pin;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, pin);
        vTaskDelay(1000);
    }
}

int main() {
    stdio_init_all();

    if(cyw43_arch_init()) {
        printf("cyw43 error on init");
        return -1;
    } else
        printf("cyw43 init correctly");

    xTaskCreate(&led_task, "LED_Task", 256, NULL, 1, NULL);
    xTaskCreate(&i2c_task, "usb_task", 256, NULL, 1, NULL);

    vTaskStartScheduler();
}
