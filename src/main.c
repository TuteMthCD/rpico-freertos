#include "FreeRTOS.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "task.h"
#include <queue.h>
#include <stdbool.h>
#include <stdio.h>

static QueueHandle_t Queue = NULL;

bool pin = 0;

void usb_task() {
    while(true) {
        bool recive = NULL;
        if(xQueueReceive(Queue, &recive, 500)) {
            printf("%d\n", recive);
        } else {
            printf("llook at queue\n");
        }
    }
}

void led_task() {
    while(true) {
        pin = !pin;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, pin);

        xQueueSend(Queue, &pin, 1000);

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

    Queue = xQueueCreate(1, sizeof(bool));

    xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
    xTaskCreate(usb_task, "usb_task", 256, NULL, 1, NULL);

    vTaskStartScheduler();
}
