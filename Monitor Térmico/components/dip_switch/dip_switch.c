
#include "dip_switch.h"
#include "driver/gpio.h"
#include "app_config.h"

void DIP_Switch_Init(void) {
    gpio_set_direction(PIN_DIP_SW1, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_DIP_SW2, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_DIP_SW3, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_DIP_SW4, GPIO_MODE_INPUT);
    // Con pull-downs externos fisicos de 10k
}


uint8_t Leer_DipSwitch(void) {
    uint8_t sw1 = gpio_get_level(PIN_DIP_SW1);
    uint8_t sw2 = gpio_get_level(PIN_DIP_SW2);

    return sw1 + (sw2 * 2);
}
