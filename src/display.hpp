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
        oled.set_contrast(127);
    };

    void draw_status(uint8_t state, float amp) {
        if (rc == OLED_NOT_FOUND) return;

        char *state_msg = getConstMsg(state_messages[state]);
        char amp_msg[6];
        snprintf(amp_msg, sizeof(amp_msg), "%.1fA", amp);

        oled.set_cursor(0, 0);
        oled.set_textWrap(false);
        oled.draw_rectangle(0, 0, OLED_WIDTH-1, 8, 0, 1);
        oled.write_string(0, -1, -1, state_msg, FONT_6x8, 0, 0);
        oled.write_string(0, 128 - strlen(amp_msg) * 8, -1, amp_msg, FONT_8x8, 0, 0);
    };

    void draw_title(const char *c_msg) {
        if (rc == OLED_NOT_FOUND) return;
        char *msg = getConstMsg(c_msg);
        oled.set_cursor(0, 1);
        oled.set_textWrap(false);
        oled.draw_rectangle(0, 8, OLED_WIDTH-1, 24, 1, 1);
        oled.write_string(0, -1, 8, msg, FONT_16x16, 1, 0);
    };

    void draw_value(char *msg) {
        if (rc == OLED_NOT_FOUND) return;
        oled.set_cursor(0, 3);
        oled.set_textWrap(false);
        oled.draw_rectangle(0, 24, OLED_WIDTH-1, 56, 0, 1);
        oled.write_string(0, 64 - strlen(msg) * 8 - 1, 24, msg, FONT_16x32, 0, 0);
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
    void draw_time(float mins) {
        char msg[7];
        snprintf(msg, sizeof(msg), "%2dh%2dm", floorf(mins / 60.0), floorf(mins - floorf(mins / 60.0) * 60.0));
        draw_value(msg);
    };

    void draw_name() {
        if (rc == OLED_NOT_FOUND) return;
        oled.set_cursor(0, 7);
        oled.set_textWrap(false);
        oled.draw_rectangle(0, 56, OLED_WIDTH-1, 64, 1, 1);
        char *msg = getConstMsg(name_message);
        oled.write_string(0, 64 - strlen(msg) * 4 - 1, 56, msg, FONT_8x8, 0, 0);
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

};
