#pragma once
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/double.h"
#include "ss_oled.hpp"
#include "hw_config.h"
#include "pilot.hpp"
#include "menu_config.h"

#define OLED_WIDTH  128
#define OLED_HEIGHT 64

#define STRLEN      32

class Display {

private:

    int rc;
    picoSSOLED oled;
    uint8_t ucBuffer[1024];
    char textBuffer[STRLEN];

    bool awake = false;
    bool powered = true;

    void copyConstStr(const char *msg) {
        if (strlen(msg) >= STRLEN) return;
        strcpy(textBuffer, msg);
    };

    void draw_value() {
        if (rc == OLED_NOT_FOUND) return;
        int x = 64 - strlen(textBuffer) * 8 - 1;
        if (x < 0) x = 0;
        oled.set_textWrap(false);
        oled.draw_rectangle(0, 3*8, OLED_WIDTH-1, 7*8, 0, 1);
        oled.set_cursor(x, 3);
        oled.write_string(0, -1, -1, textBuffer, FONT_16x32, 0, 0);
    };

public:

    Display() : oled(OLED_128x64, 0x3c, 0, 0, PICO_I2C, SDA_PIN, SCL_PIN, I2C_SPEED) {
        rc = oled.init();
        oled.set_back_buffer(ucBuffer);
        if (rc == OLED_NOT_FOUND) return;

        oled.fill(0x00, 1);
        wake();
    };

    void draw_status(uint8_t state, double amp, bool relay) {
        if (rc == OLED_NOT_FOUND) return;

        oled.set_textWrap(false);
        oled.draw_rectangle(0, 0*8, OLED_WIDTH-1, 1*8, 0, 1);
        oled.set_cursor(0, 0);

        copyConstStr(state_messages[state]);
        oled.write_string(0, -1, -1, textBuffer, FONT_6x8, 0, 0);

        snprintf(textBuffer, STRLEN, "%.1lfA", amp);
        int amp_x = 128 - strlen(textBuffer) * 8;
        if (amp_x < 0) amp_x = 0;
        oled.set_cursor(amp_x, 0);
        oled.write_string(0, -1, -1, textBuffer, FONT_8x8, 0, 0);

        if (relay) oled.draw_sprite(spr_pwr, SPR_PWR_W, SPR_PWR_H, 1, amp_x - SPR_PWR_W, 0, 1);
    };

    void draw_title(const char *c_msg) {
        if (rc == OLED_NOT_FOUND) return;
        copyConstStr(c_msg);
        int x = 64 - strlen(textBuffer) * 8 - 1;
        if (x < 0) x = 0;
        oled.set_textWrap(false);
        oled.draw_rectangle(0, 1*8, OLED_WIDTH-1, 3*8, 1, 1);
        oled.set_cursor(x, 1);
        oled.write_string(0, -1, -1, textBuffer, FONT_16x16, 1, 0);
    };

    void draw_menu(const char *c_msg) {
        copyConstStr(c_msg);
        draw_value();
    };
    void draw_amp(double amp) {
        snprintf(textBuffer, STRLEN, "%.1lfA", amp);
        draw_value();
    };
    void draw_time(uint seconds) {
        if (seconds < 3600) {
            snprintf(textBuffer, STRLEN, "%2dm%2ds", (seconds / 60) % 60, seconds % 60);
        } else if (seconds < 86400) {
            snprintf(textBuffer, STRLEN, "%2dh%2dm", seconds / 3600, (seconds / 60) % 60);
        } else {
            snprintf(textBuffer, STRLEN, "%2dd%2dh", seconds / 86400, (seconds / 3600) % 24);
        }
        draw_value();
    };
    void draw_watts(double watts) {
        if (watts < 1000) {
            snprintf(textBuffer, STRLEN, "%.1lfW", watts);
        } else {
            snprintf(textBuffer, STRLEN, "%.2lfkW", watts / 1000.0);
        }
        draw_value();
    };

    void dump() {
        if (rc == OLED_NOT_FOUND) return;
        oled.dump_buffer(NULL);
    };

    void clear() {
        if (rc == OLED_NOT_FOUND) return;
        oled.set_back_buffer(ucBuffer);
        oled.fill(0, 1);
    };

    void sleep(void) {
        if (rc == OLED_NOT_FOUND) return;
        oled.set_contrast(0);
        awake = false;
    };
    void wake(void) {
        if (rc == OLED_NOT_FOUND) return;
        if (!powered) power_on();
        oled.set_contrast(127);
        awake = true;
    };
    bool is_awake(void) {
        return awake;
    };

    void power_off(void) {
        if (rc == OLED_NOT_FOUND) return;
        if (powered == false) return;
        oled.power(false);
        powered = false;
    };
    void power_on(void) {
        if (rc == OLED_NOT_FOUND) return;
        if (powered == true) return;
        oled.power(true);
        powered = true;
        oled.fill(0x00, 1);
        dump();
    };
    bool is_powered(void) {
        return powered;
    };

};
