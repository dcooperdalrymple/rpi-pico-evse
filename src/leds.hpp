#pragma once
#include <stdarg.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hw_config.h"
#include "pilot.hpp"

class LEDs {

private:

    static uint8_t cur_state;
    static int timer;

    const static uint pins[LED_NUM];
    static uint slice[LED_NUM];

    static void pwm_update() {
        timer++;
        switch (cur_state) {
            case PILOT_STATE_READY:
                if (timer < 256) {
                    set_levels(255, timer, 0);
                } else if (timer < 512) {
                    set_levels(255, 255 - (timer - 256), 0);
                } else {
                    timer = -1;
                }
                break;
            case PILOT_STATE_CHARGE:
                if (timer < 256 * 1) {
                    set_levels(timer, 0, 0);
                } else if (timer < 256 * 2) {
                    set_levels(255, timer - 256, 0);
                } else if (timer < 256 * 3) {
                    set_levels(255, 255, timer - 256 * 2);
                } else if (timer < 256 * 4) {
                    set_levels(255 - (timer - 256 * 3), 255, 255);
                } else if (timer < 256 * 5) {
                    set_levels(0, 255 - (timer - 256 * 4), 255);
                } else if (timer < 256 * 6) {
                    set_levels(0, 0, 255 - (timer - 256 * 5));
                } else if (timer > 256 * 8) {
                    timer = -1;
                }
                break;
            case PILOT_STATE_VENT:
                if (timer < 128) {
                    set_levels(0, 0, 255);
                } else if (timer < 256) {
                    set_levels(0, 0, 0);
                } else {
                    timer = -1;
                }
                break;
        }

        pwm_clear_irq(slice[0]);
    };

    static void set_levels(uint16_t val1, uint16_t val2, uint16_t val3) {
        uint16_t value[LED_NUM] = {val1, val2, val3};
        uint8_t i;
        for (i = 0; i < LED_NUM; i++) {
            pwm_set_gpio_level(pins[i], value[i] * value[i]); // square to make brightness appear linear
        }
    };

public:

    LEDs() {
        pwm_config config = pwm_get_default_config();
        pwm_config_set_clkdiv(&config, 4.f);

        uint8_t i;
        for (i = 0; i < LED_NUM; i++) {
            gpio_set_function(pins[i], GPIO_FUNC_PWM);
            slice[i] = pwm_gpio_to_slice_num(pins[i]);
            pwm_init(slice[i], &config, true);
        }

        pwm_clear_irq(slice[0]);
        pwm_set_irq_enabled(slice[0], true);
        irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_update);
        irq_set_enabled(PWM_IRQ_WRAP, true);

        update(PILOT_STATE_WAIT, true);
    };

    void update(uint8_t state, bool force = false) {
        if (cur_state == state && !force) return;
        switch (state) {
            case PILOT_STATE_WAIT:
            default:
                set_levels(127, 0, 0);
                break;
        }
        cur_state = state;
        timer = -1;
    };

};
