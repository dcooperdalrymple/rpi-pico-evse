#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "ss_oled.hpp"

#include "src/hw_config.h"
#include "src/display.hpp"
#include "src/rotary.hpp"
#include "src/pilot.hpp"
#include "src/leds.hpp"
#include "src/coretalk.hpp"
#include "src/menu_config.h"

static CoreTalk coretalk;
static Pilot pilot;

// Secondary Core (display/rotary/leds)

static Display display;
static LEDs leds;
static Rotary rotary(ROTARY_CLK, ROTARY_SW);
static int cur_screen, cur_menu;
static absolute_time_t rotary_timestamp, screen_timestamp;

void interface_activity(void) {
    screen_timestamp = get_absolute_time();
    if (!display.is_powered()) display.power_on();
    if (!display.is_awake()) display.wake();
}
void interface_rotary_handler(uint8_t event) {
    rotary_timestamp = get_absolute_time();
    interface_activity();
    switch (event) {
        case ROTARY_CW:
            if (rotary.get_rotation() >= ROTARY_BOUNCE) {
                rotary.set_rotation(0);
                switch (cur_screen) {
                    case SCREEN_MENU:
                        if (cur_menu >= MENU_MAX) cur_menu = MENU_MIN;
                        else cur_menu += 1;
                        break;
                    case SCREEN_AMP:
                        pilot.increase_amp();
                        break;
                }
            }
            break;
        case ROTARY_CCW:
            if (rotary.get_rotation() <= -ROTARY_BOUNCE) {
                rotary.set_rotation(0);
                switch (cur_screen) {
                    case SCREEN_MENU:
                        if (cur_menu <= MENU_MIN) cur_menu = MENU_MAX;
                        else cur_menu -= 1;
                        break;
                    case SCREEN_AMP:
                        pilot.decrease_amp();
                        break;
                }
            }
            break;
        //case ROTARY_PRESS:
        //    break;
        case ROTARY_RELEASE:
            switch (cur_screen) {
                case SCREEN_MENU:
                    cur_screen = cur_menu;
                    break;
                default:
                    cur_screen = SCREEN_MENU;
                    break;
            }
            break;
    }
}
void interface_display() {
    interface_activity();

    display.draw_status(pilot.get_state(), pilot.get_amp(), pilot.get_relay());
    display.draw_title(menu_titles[cur_screen]);
    switch (cur_screen) {
        case SCREEN_MENU:
            display.draw_menu(menu_titles[cur_menu]);
            break;
        case SCREEN_AMP:
            display.draw_amp(pilot.get_amp());
            break;
        case SCREEN_TIME:
            display.draw_time(pilot.get_time());
            break;
        case SCREEN_WATT:
            display.draw_watts(pilot.get_watts(false));
            break;
    }
    display.dump();
}
void interface_update() {
    // LED State
    leds.update(pilot.get_state());

    // Screen Update
    interface_display();
}
void interface_core_handler(uint32_t event, uint32_t value) {
    switch (event) {
        case CORE_POKE:
            interface_update();
            break;
        case CORE_DATA:
            break;
    }
}
void interface_core() {
    cur_screen = SCREEN_MENU;
    cur_menu = MENU_MIN;
    rotary_timestamp = screen_timestamp = nil_time;

    // Blocking handshake with core0
    if (!coretalk.handshake()) return;

    coretalk.setup();
    coretalk.set_callback(&interface_core_handler);
    rotary.set_callback(&interface_rotary_handler);

    interface_display();

    absolute_time_t now_timestamp;
    screen_timestamp = get_absolute_time();

    while (1) {
        now_timestamp = get_absolute_time();
        if (!is_nil_time(rotary_timestamp) && absolute_time_diff_us(rotary_timestamp, now_timestamp) / 1000 > ROTARY_RESET) {
            rotary.set_rotation(0);
            rotary_timestamp = nil_time;
        }
        if (!is_nil_time(screen_timestamp) && pilot.get_state() == PILOT_STATE_WAIT) {
            if (display.is_awake() && absolute_time_diff_us(screen_timestamp, now_timestamp) / 1000 > SCREEN_TIMEOUT) {
                display.sleep();
            } else if (display.is_powered() && absolute_time_diff_us(screen_timestamp, now_timestamp) / 1000 > SCREEN_SHUTDOWN) {
                display.power_off();
                screen_timestamp = nil_time;
            }
        }
        sleep_ms(SCREEN_UPDATE);
    }
}

// Main Core (Pilot/EV Handling)

void pilot_callback(uint8_t event) {
    switch (event) {
        case PILOT_STATE_CHANGE:
            coretalk.poke();
            break;
        case PILOT_RELAY_ON:
        case PILOT_RELAY_OFF:
            coretalk.poke();
            break;
    }
}

void pilot_core_handler(uint32_t event, uint32_t value) {
    switch (event) {
        case CORE_POKE:
            break;
        case CORE_DATA:
            break;
    }
}

int main() {
    stdio_init_all();

    // picotool declarations
    bi_decl(bi_program_description(name_message));
    bi_decl(bi_2pins_with_func(SDA_PIN, SCL_PIN, GPIO_FUNC_I2C));

    // Setup core1 and do blocking handshake
    multicore_launch_core1(interface_core);
    if (!coretalk.handshake()) return 1;

    coretalk.setup();
    coretalk.set_callback(&pilot_core_handler);
    pilot.set_callback(&pilot_callback);

    absolute_time_t now_timestamp, update_timestamp;
    update_timestamp = get_absolute_time();

    while (1) {
        pilot.update();

        now_timestamp = get_absolute_time();
        if (pilot.get_state() != PILOT_STATE_WAIT && !is_nil_time(update_timestamp) && absolute_time_diff_us(update_timestamp, now_timestamp) / 1000 > SCREEN_UPDATE) {
            pilot.get_watts(true);
            coretalk.poke();
            update_timestamp = now_timestamp;
        }
    }

    return 0;
}
