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
#define VDC_REG_MEMORY_MODE 28
#define VDC_REG_HIGH_ADDR   18
#define VDC_REG_LOW_ADDR    19
#define VDC_REG_DATA        31

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
    const char* color_names[16] = {
        "Black", "White", "Red", "Cyan",
        "Purple", "Green", "Blue", "Yellow",
        "Orange", "Brown", "LightRed", "Gray1",
        "Gray2", "LightGreen", "LightBlue", "Gray3"
    };
    unsigned char i, j;
    const unsigned char label_width = 9;  // Max name length for alignment

    for (i = 0; i < 16; ++i) {
        unsigned char y = y_offset + i;

        gotoxy(0, y);
        textcolor(COLOR_WHITE);
        revers(0);
        // Print fixed-width label (left-aligned, padded with spaces)
        cprintf("%-10s", color_names[i]);

        // Print the color bar after the label
        textcolor(i);
        revers(1);
        for (j = 0; j < width - label_width - 1; ++j) {
            cputc(' ');
        }
        revers(0);
    }

    textcolor(COLOR_WHITE);
}

void draw_vdc_info_screen(unsigned char screen_width) {
    unsigned char oldval, result, i;

    // Clear content area (avoid utility bar)
    for (i = 3; i < 23; i++) {
        cclearxy(0, i, screen_width);
    }

    textcolor(COLOR_CYAN);
    cputsxy((screen_width - 20) / 2, 2, "VDC RAM Test Utility");

    // MARK: - VDC RAM check routine
    oldval = vdc_read(VDC_REG_MEMORY_MODE);

    // Enable 64KB mode by setting bit 4 of reg 28
    vdc_write(VDC_REG_MEMORY_MODE, oldval | 0x10);

    // Write 0x00 to $1FFF
    vdc_write(VDC_REG_HIGH_ADDR, 0x1F);  // high byte
    vdc_write(VDC_REG_LOW_ADDR, 0xFF);  // low byte
    vdc_write(VDC_REG_DATA, 0x00);

    // Write 0xFF to $9FFF
    vdc_write(VDC_REG_HIGH_ADDR, 0x9F);
    vdc_write(VDC_REG_LOW_ADDR, 0xFF);
    vdc_write(VDC_REG_DATA, 0xFF);

    // Read back from $1FFF
    vdc_write(VDC_REG_HIGH_ADDR, 0x1F);
    vdc_write(VDC_REG_LOW_ADDR, 0xFF);
    result = vdc_read(VDC_REG_DATA);

    // Restore original register 28
    vdc_write(VDC_REG_MEMORY_MODE, oldval);

    // Interpret result
    textcolor(COLOR_WHITE);
    if (result == 0x00) {
        cputsxy(0, 3, "Detected VDC RAM: 64 KB");
    } else {
        cputsxy(0, 3, "Detected VDC RAM: 16 KB");
    }

    if (screen_width == 80) {
        cputsxy(10, 5, "VDC Available Colors:");
    } else {
        cputsxy(10, 5, "VIC-II Available Colors:");
    }
    draw_color_test_bar(6, screen_width);
}
