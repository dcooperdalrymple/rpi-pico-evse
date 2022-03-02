#pragma once
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hw_config.h"
#include "ev_config.h"

#define PILOT_STATE_WAIT    0
#define PILOT_STATE_READY   1
#define PILOT_STATE_CHARGE  2
#define PILOT_STATE_VENT    3
#define PILOT_STATES        4

#define PILOT_STATE_CHANGE  0
#define PILOT_RELAY_ON      1
#define PILOT_RELAY_OFF     2

typedef void(* pilot_callback_t) (uint8_t event);

class Pilot {

private:

    uint pwm_slice;
    uint pwm_channel;

    float cur_amp, last_watts;
    uint16_t last_pwm, cur_pwm;
    uint8_t last_relay;
    uint8_t cur_state;

    absolute_time_t relay_timestamp, start_timestamp, end_timestamp, watts_timestamp;

    pilot_callback_t cb = NULL;

    // Updates & Setters

    bool set_state(uint8_t state, bool force = false) {
        if (cur_state == state && force == false) return false;
        cur_state = state;
        if (state == PILOT_STATE_CHARGE) relay_timestamp = get_absolute_time();
        if (cb) cb(PILOT_STATE_CHANGE);
        return true;
    };
    void update_state() {
        uint8_t state = PILOT_STATE_WAIT;
        if (gpio_get(PILOT_RDY) == true) {
            state = PILOT_STATE_READY;
            if (pulseIn(PILOT_CRG, true, 2000) > 0) {
                state = PILOT_STATE_CHARGE;
                if (pulseIn(PILOT_VNT, true, 2000) > 0) {
                    state = PILOT_STATE_VENT;
                }
            }
        }
        set_state(state);
    };

    bool set_pwm(uint16_t level, bool force = false) {
        if (last_pwm == level && force == false) return false;
        pwm_set_chan_level(pwm_slice, pwm_channel, level);
        last_pwm = level;
        return true;
    }
    void update_pwm() {
        switch (cur_state) {
            case PILOT_STATE_READY:
            case PILOT_STATE_CHARGE:
                set_pwm(cur_pwm);
                break;
            case PILOT_STATE_VENT:
                set_pwm(PILOT_PWM_VENT);
                break;
            case PILOT_STATE_WAIT:
            default:
                set_pwm(PILOT_PWM_WAIT);
                break;
        }
    };

    bool set_relay(uint8_t value, bool force = false) {
        if (last_relay == value && force == false) return false;
        gpio_put(RELAY_A, value);
        gpio_put(RELAY_B, value);
        last_relay = value;
        if (value == 1) {
            start_timestamp = get_absolute_time();
            end_timestamp = nil_time;
            reset_watts();
            if (cb) cb(PILOT_RELAY_ON);
        } else {
            end_timestamp = get_absolute_time();
            if (cb) cb(PILOT_RELAY_OFF);
        }
        return true;
    };
    void update_relay() {
        switch (cur_state) {
            case PILOT_STATE_CHARGE:
                if (!is_nil_time(relay_timestamp) && absolute_time_diff_us(relay_timestamp, get_absolute_time()) / 1000 > RELAY_DELAY) {
                    set_relay(1);
                    relay_timestamp = nil_time;
                }
                break;
            default:
                set_relay(0);
                relay_timestamp = nil_time;
                break;
        }
    };

    // Custom PulseIn (similar to Arduino)

    int pulseIn(uint pin, bool level, uint timeout) { // timeout in us
        absolute_time_t t0, t1;

        t0 = get_absolute_time();
        while (gpio_get(pin) != level) {
            if (absolute_time_diff_us(t0, get_absolute_time()) > timeout) return 0;
        }

        t1 = get_absolute_time();
        while (gpio_get(pin) == level) {
            if (absolute_time_diff_us(t0, get_absolute_time()) > timeout) return 0;
        }

        return absolute_time_diff_us(t1, get_absolute_time());
    };

    // Amperage Calculations and Setter

    float calculate_amp(uint16_t pwm_level) {
        // 0.6A per 10us up to 850us, min 100us
        // >850us, (in us - 640us) * 2.5
        float micros = (float)pwm_level / PILOT_PWM_WAIT * 1000.0;
        if (micros > 850.0) {
            return (micros - 640.0) / 10.0 * 2.5;
        } else {
            return 0.6 * micros / 10.0;
        }
    };

    uint16_t calculate_pwm(float amp) {
        if (amp > 52.5) {
            return (uint16_t)((amp / 2.5 * 10.0 + 640.0) / 1000.0 * PILOT_PWM_WAIT);
        } else {
            return (uint16_t)((amp * 10.0 / 0.6) / 1000.0 * PILOT_PWM_WAIT);
        }
    };

    float set_amp(float amp) {
        if (cur_state == PILOT_STATE_CHARGE) update_watts();
        if (amp < EV_AMP_MIN) amp = EV_AMP_MIN;
        if (amp > EV_AMP_MAX) amp = EV_AMP_MAX;
        cur_amp = amp;
        cur_pwm = calculate_pwm(amp);
        return amp;
    };

    void update_watts() {
        absolute_time_t now = get_absolute_time();
        if (is_nil_time(watts_timestamp) && !is_nil_time(start_timestamp)) watts_timestamp = start_timestamp;
        if (!is_nil_time(end_timestamp)) {
            if (!is_nil_time(watts_timestamp) && absolute_time_diff_us(watts_timestamp, end_timestamp) > 0) {
                now = end_timestamp;
            } else {
                return;
            }
        }
        if (!is_nil_time(watts_timestamp)) {
            uint micros = absolute_time_diff_us(watts_timestamp, now);
            last_watts += cur_amp * EV_VOLTS * ((float)micros / 3600000000.0);
        }
        watts_timestamp = now;
    };
    void reset_watts() {
        last_watts = 0;
        watts_timestamp = nil_time;
    };

public:

    Pilot() {
        // Initialize GPIO

        gpio_init(RELAY_A);
        gpio_set_dir(RELAY_A, GPIO_OUT);
        gpio_init(RELAY_B);
        gpio_set_dir(RELAY_B, GPIO_OUT);

        gpio_init(PILOT_RDY);
        gpio_set_dir(PILOT_RDY, GPIO_IN);
        gpio_pull_up(PILOT_RDY);
        gpio_init(PILOT_CRG);
        gpio_set_dir(PILOT_CRG, GPIO_IN);
        gpio_pull_up(PILOT_CRG);
        gpio_init(PILOT_VNT);
        gpio_set_dir(PILOT_VNT, GPIO_IN);
        gpio_pull_up(PILOT_VNT);

        gpio_set_function(PILOT_PWM, GPIO_FUNC_PWM);
        pwm_slice = pwm_gpio_to_slice_num(PILOT_PWM);
        pwm_channel = pwm_gpio_to_channel(PILOT_PWM);

        // Setup 1Khz PWM
        pwm_set_phase_correct(pwm_slice, true); // Halves frequency
        pwm_set_clkdiv(pwm_slice, 1); // Default clock divider (125Mhz)
        pwm_set_wrap(pwm_slice, 62500); // 125000000/2/62500=1000hz
        pwm_set_chan_level(pwm_slice, pwm_channel, PILOT_PWM_WAIT);
        pwm_set_enabled(pwm_slice, true);

        // Begin in Wait State
        set_amp(EV_AMP_MIN); // also sets pwm
        cur_state = PILOT_STATE_WAIT;
        relay_timestamp = nil_time;
        start_timestamp = end_timestamp = nil_time;

        set_relay(0, true);
        set_state(PILOT_STATE_WAIT, true);
        set_pwm(PILOT_PWM_WAIT, true);
    };

    void update() {
        update_state();
        update_relay();
        update_pwm();
    };

    uint8_t get_state() {
        return cur_state;
    };

    float increase_amp() {
        return set_amp(cur_amp + EV_AMP_STEP);
    };
    float decrease_amp() {
        return set_amp(cur_amp - EV_AMP_STEP);
    };
    float get_amp() {
        return cur_amp;
    };

    uint get_time() { // Returns seconds
        if (!is_nil_time(start_timestamp) && is_nil_time(end_timestamp)) {
            return absolute_time_diff_us(start_timestamp, get_absolute_time()) / 1000000;
        } else if (!is_nil_time(start_timestamp) && !is_nil_time(end_timestamp)) {
            return absolute_time_diff_us(start_timestamp, end_timestamp) / 1000000;
        } else {
            return 0.0;
        }
    };

    bool get_relay() {
        return last_relay == 1;
    };

    float get_watts() {
        update_watts();
        return last_watts;
    };

    void set_callback(pilot_callback_t callback) {
        cb = callback;
    };
    void clear_callback(void) {
        cb = NULL;
    };

};
