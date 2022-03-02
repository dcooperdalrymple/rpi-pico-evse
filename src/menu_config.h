#ifndef MENU_CONFIG_H_
#define MENU_CONFIG_H_

#define SCREEN_MENU 0
#define SCREEN_AMP  1
#define SCREEN_TIME 2

#define MENU_MIN    1
#define MENU_MAX    2

#define SCREEN_TIMEOUT  10000 // milliseconds
#define SCREEN_SHUTDOWN 30000 // milliseconds

const char* const state_messages[PILOT_STATES] = {
    "Waiting",
    "Connected",
    "Charging",
    "Venting"
};
const char* const name_message = "RPi Pico EVSE v1.0";

const char* const menu_titles[MENU_MAX+1] = {
    "PicoEVSE",
    "Amps",
    "Time"
};

#define SPR_PWR_W   8
#define SPR_PWR_H   8
static uint8_t spr_pwr[] = {
    0b01111100,
    0b01111000,
    0b11110000,
    0b11111100,
    0b00111000,
    0b00110000,
    0b01100000,
    0b11000000
};

#endif
