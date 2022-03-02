#pragma once
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/float.h"
#include "ss_oled.hpp"
#include "hw_config.h"
#include "pilot.hpp"
#include "menu_config.h"

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

class Display {

private:

    int rc;
    picoSSOLED oled;
    uint8_t ucBuffer[1024];

    bool awake = false;
    bool powered = true;

    char *getConstMsg(const char *msg) {
        char *buf = new char[strlen(msg)+1];
        strcpy(buf, msg);
        return buf;
    };

public:

    Display() : oled(OLED_128x64, 0x3c, 0, 0, PICO_I2C, SDA_PIN, SCL_PIN, I2C_SPEED) {
        rc = oled.init();
        oled.set_back_buffer(ucBuffer);
        if (rc == OLED_NOT_FOUND) return;

        oled.fill(0x00, 1);
        wake();
    };

    void draw_status(uint8_t state, float amp) {
        if (rc == OLED_NOT_FOUND) return;

        char *state_msg = getConstMsg(state_messages[state]);
        char amp_msg[6];
        snprintf(amp_msg, sizeof(amp_msg), "%.1fA", amp);
        int amp_x = 128 - strlen(amp_msg) * 8;
        if (amp_x < 0) amp_x = 0;

        oled.set_textWrap(false);
        oled.draw_rectangle(0, 0*8, OLED_WIDTH-1, 1*8, 0, 1);
        oled.set_cursor(0, 0);
        oled.write_string(0, -1, -1, state_msg, FONT_6x8, 0, 0);
        oled.set_cursor(amp_x, 0);
        oled.write_string(0, -1, -1, amp_msg, FONT_8x8, 0, 0);
    };

    void draw_title(const char *c_msg) {
        if (rc == OLED_NOT_FOUND) return;
        char *msg = getConstMsg(c_msg);
        int x = 64 - strlen(msg) * 8 - 1;
        if (x < 0) x = 0;
        oled.set_textWrap(false);
        oled.draw_rectangle(0, 1*8, OLED_WIDTH-1, 3*8, 1, 1);
        oled.set_cursor(x, 1);
        oled.write_string(0, -1, -1, msg, FONT_16x16, 1, 0);
    };

    void draw_value(char *msg) {
        if (rc == OLED_NOT_FOUND) return;
        int x = 64 - strlen(msg) * 8 - 1;
        if (x < 0) x = 0;
        oled.set_textWrap(false);
        oled.draw_rectangle(0, 3*8, OLED_WIDTH-1, 7*8, 0, 1);
        oled.set_cursor(x, 3);
        oled.write_string(0, -1, -1, msg, FONT_16x32, 0, 0);
    };
    void draw_menu(const char *c_msg) {
        char *msg = getConstMsg(c_msg);
        draw_value(msg);
    };
    void draw_amp(float amp) {
        char msg[6];
        snprintf(msg, sizeof(msg), "%.1fA", amp);
        draw_value(msg);
    };
    void draw_time(uint seconds) {
        char msg[7];
        if (seconds < 3600) {
            snprintf(msg, sizeof(msg), "%2dm%2ds", (seconds / 60) % 60, seconds % 60);
        } else if (seconds < 86400) {
            snprintf(msg, sizeof(msg), "%2dh%2dm", seconds / 3600, (seconds / 60) % 60);
        } else {
            snprintf(msg, sizeof(msg), "%2dd%2dh", seconds / 86400, (seconds / 3600) % 24);
        }
        draw_value(msg);
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
