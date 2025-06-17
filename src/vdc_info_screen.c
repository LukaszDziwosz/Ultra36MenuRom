//   _____  ___________              _______________
//   __  / / /__  /_  /_____________ __|__  /_  ___/
//   _  / / /__  /_  __/_  ___/  __ `/__/_ <_  __ \
//   / /_/ / _  / / /_ _  /   / /_/ /____/ // /_/ /
//   \____/  /_/  \__/ /_/    \__,_/ /____/ \____/
// Ultra-36 Rom Switcher for Commodore 128 - C128 Menu Program - vdc_info_screen.c
// Free for personal use.
// Commercial use or resale (in whole or part) prohibited without permission.
// (c) 2025 Lukasz Dziwosz / LukasSoft. All Rights Reserved.

#include <conio.h>
#include <c128.h>
#include <peekpoke.h>
#include "vdc_info_screen.h"

#define VDC_ADDR_REG 0xD600
#define VDC_DATA_REG 0xD601

// Helper to write to a VDC register
void vdc_write(unsigned char reg, unsigned char value) {
    POKE(VDC_ADDR_REG, reg);
    while (!(PEEK(VDC_ADDR_REG) & 0x80));  // wait for ready
    POKE(VDC_DATA_REG, value);
}

// Helper to read from a VDC register
unsigned char vdc_read(unsigned char reg) {
    POKE(VDC_ADDR_REG, reg);
    while (!(PEEK(VDC_ADDR_REG) & 0x80));  // wait for ready
    return PEEK(VDC_DATA_REG);
}

void draw_color_test_bar(unsigned char y_offset, unsigned char width) {
    unsigned char i, j;
    unsigned char bar_width = (width - 3) / 2; // 1 left + 1 mid + 1 right padding

    for (i = 0; i < 16; ++i) {
        unsigned char x = (i % 2) * (bar_width);         // 2 bars
        unsigned char y = y_offset + (i / 2);            // 8 rows

        gotoxy(x, y);
        textcolor(i);
        revers(1);
        cprintf("Color %2d ", i + 1);

        for (j = 0; j < bar_width - 5; j++) {
            cputc(' ');
        }

        revers(0);
    }

    textcolor(COLOR_WHITE);
}

void draw_vdc_info_screen(unsigned char screen_width) {
    unsigned char oldval, result, i;

    // Clear content area (avoid utility bar)
    for (i = 3; i < 22; i++) {
        cclearxy(0, i, screen_width);
    }
    textcolor(COLOR_WHITE);
    cputsxy(0, 3, "VDC RAM Test Utility");

    // Save original value of register 28
    oldval = vdc_read(28);

    // Enable 64KB mode by setting bit 4 of reg 28
    vdc_write(28, oldval | 0x10);

    // Write 0x00 to $1FFF
    vdc_write(18, 0x1F);  // high byte
    vdc_write(19, 0xFF);  // low byte
    vdc_write(31, 0x00);

    // Write 0xFF to $9FFF
    vdc_write(18, 0x9F);
    vdc_write(19, 0xFF);
    vdc_write(31, 0xFF);

    // Read back from $1FFF
    vdc_write(18, 0x1F);
    vdc_write(19, 0xFF);
    result = vdc_read(31);

    // Restore original register 28
    vdc_write(28, oldval);

    // Interpret result
    if (result == 0x00) {
        cputsxy(0, 5, "Detected VDC RAM: 64 KB");
    } else {
        cputsxy(0, 5, "Detected VDC RAM: 16 KB");
    }

    if (screen_width == 80) {
        cputsxy(0, 7, "VDC Available Colors");
    } else {
        cputsxy(0, 7, "VIC-II Available Colors");
    }
    draw_color_test_bar(9, screen_width);
}
