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

#endif
