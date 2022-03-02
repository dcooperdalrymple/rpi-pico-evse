#include "leds.hpp"

uint8_t LEDs::cur_state = 0;
const uint LEDs::pins[LED_NUM] = {LED_1, LED_2, LED_3};
uint LEDs::slice[LED_NUM] = {0, 0, 0};
