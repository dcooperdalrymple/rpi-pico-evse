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
#include "src/menu_config.h"

static Display display;
static Pilot pilot;
static LEDs leds;
static Rotary rotary(ROTARY_CLK, ROTARY_SW);

static int cur_screen;
static int cur_menu;

static absolute_time_t rotary_timestamp = nil_time, screen_timestamp = nil_time;

void screen_activity(void) {
    screen_timestamp = get_absolute_time();
    if (!display.is_powered()) display.power_on();
    if (!display.is_awake()) display.wake();
}

void rotary_callback(uint8_t event) {
    rotary_timestamp = get_absolute_time();
    screen_activity();
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

void pilot_callback(uint8_t event) {
    screen_activity();
}

void update_screen() {
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
    }
    display.dump();
}

int main() {
    stdio_init_all();

    // picotool declarations
    bi_decl(bi_program_description(name_message));
    bi_decl(bi_2pins_with_func(SDA_PIN, SCL_PIN, GPIO_FUNC_I2C));

    rotary.set_callback(&rotary_callback);

    pilot.set_callback(&pilot_callback);

    cur_screen = SCREEN_MENU;
    cur_menu = SCREEN_AMP;

    absolute_time_t now;
    screen_timestamp = get_absolute_time();

    while (1) {
        pilot.update();
        update_screen();
        leds.update(pilot.get_state());

        now = get_absolute_time();
        if (!is_nil_time(rotary_timestamp) && absolute_time_diff_us(rotary_timestamp, now) / 1000 > ROTARY_RESET) {
            rotary.set_rotation(0);
            rotary_timestamp = nil_time;
        }
        if (!is_nil_time(screen_timestamp) && pilot.get_state() == PILOT_STATE_WAIT) {
            if (display.is_awake() && absolute_time_diff_us(screen_timestamp, now) / 1000 > SCREEN_TIMEOUT) {
                display.sleep();
            } else if (display.is_powered() && absolute_time_diff_us(screen_timestamp, now) / 1000 > SCREEN_SHUTDOWN) {
                display.power_off();
                screen_timestamp = nil_time;
            }
        }
    }

    return 0;
}
