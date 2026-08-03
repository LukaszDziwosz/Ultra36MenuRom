//   _____  ___________              _______________
//   __  / / /__  /_  /_____________ __|__  /_  ___/
//   _  / / /__  /_  __/_  ___/  __ `/__/_ <_  __ \
//   / /_/ / _  / / /_ _  /   / /_/ /____/ // /_/ /
//   \____/  /_/  \__/ /_/    \__,_/ /____/ \____/
// Ultra-36 Rom Switcher for Commodore 128 - C128 Menu Program - main.c
// Free for personal use.
// Commercial use or resale (in whole or part) prohibited without permission.
// (c) 2025 Lukasz Dziwosz / LukasSoft. All Rights Reserved.

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <peekpoke.h>
#include <c128.h>

#include "vdc_info_screen.h"
#include "sid_info_screen.h"

#define APP_VERSION "1.0.0"

// CIA2 User Port registers and synchronous serial pins
#define CIA2_PRB 0xDD01
#define CIA2_DDRB 0xDD03
#define SERIAL_DATA_MASK 0x01  // User Port C / PB0 -> Tiny PA1
#define SERIAL_CLOCK_MASK 0x02 // User Port D / PB1 -> Tiny PB2 / INT0

// Ultra36 one-byte command protocol
#define SERIAL_OPCODE_BANK 0x01
#define SERIAL_OPCODE_JIFFY 0x02
#define SERIAL_OPCODE_TEMP_BANK 0x03
#define CMD_BANK_PREFIX 0xA0
#define CMD_JIFFY_PREFIX 0xB0
#define CMD_TEMP_BANK_PREFIX 0xD0
#define HALF_CYCLE_DELAY 20
#define ACK_TIMEOUT_STEPS 1500

typedef int bool;
#define true 1
#define false 0

// Forward declarations
int mainmenu();
bool send_tiny_command(unsigned char opcode, unsigned char value);
void send_byte(unsigned char value, unsigned char *port_value);
bool send_command(unsigned char command);
void delay_units(unsigned int count);
void draw_title_bar(void);
void draw_fkey_bar(void);
void draw_content_area(const char *title, const char *options[], int count, int selected);
void draw_options_initial(const char *options[], int count, int selected);
void draw_options_colors(int count, int selected);
void draw_option(int option_num, int total_count, int is_selected);
void get_item_position(unsigned char item_index, int total_count, unsigned char *x, unsigned char *y);
int handle_selection(int selected, int max_items, unsigned char key);
void draw_rom_screen(int selected);
void draw_jiffy_screen(int selected);
void draw_info_screen(void);
void show_status_message(const char *message, unsigned char color,
                         unsigned char seconds);
void on_screen_instructions(const bool isJiffy);
void draw_util_bar(void);
void fill_line(unsigned char y, unsigned char color, unsigned char reversed);
void clear_menu_transition_rows(void);
void draw_main_frame(const char *title);
void draw_frame_rule(unsigned char y);

// Global variables
unsigned char SCREENW;
int current_screen = 0; // 0=ROM, 1=JiffyDOS, 2=Info
int previous_screen = 0;
bool basic_reset_armed = false;

#ifdef ONLINE_BUILD
#include "online_rom_config.h"
#endif

#ifndef USER_ROM_NAMES_INIT
#error USER_ROM_NAMES_INIT must define the 6 or 14 user-selectable ROM names
#endif

#ifndef NUM_USER_ROMS
#error NUM_USER_ROMS must be set to 6 or 14
#endif

#if NUM_USER_ROMS != 6 && NUM_USER_ROMS != 14
#error NUM_USER_ROMS must be exactly 6 (8 banks) or 14 (16 banks)
#endif

/*
 * Bank 0 contains this menu program. Bank 1 must always be the empty bank,
 * so its label is a firmware invariant rather than part of build input.
 */
#define NUM_ROMS (NUM_USER_ROMS + 1)
const char *romNames[] = {"Empty_Bank", USER_ROM_NAMES_INIT};

const char *jiffyOptions[] = {
    "JiffyDOS ON",
    "JiffyDOS OFF"};

const char *fkeyLabels[] = {
    " F1 ROMS ",
    " F2 JIFFY ",
    " F3 INFO "};

int main(void)
{
    int result;

    // Detect screen width (VIC or VDC)
    if (PEEK(0x00EE) == 79)
    {
        SCREENW = 80;
        fast();
        bgcolor(COLOR_BLUE);
    }
    else
    {
        SCREENW = 40;
        bgcolor(COLOR_BLUE);
        bordercolor(COLOR_BLUE);
    }

    clrscr();

    result = mainmenu();

    // Clean up before exit
    clrscr();
    slow();
    return result;
}

void delay_units(unsigned int count)
{
    unsigned int i;

    for (i = 0; i < count; i++)
    {
        __asm__("nop");
    }
}

bool send_tiny_command(unsigned char opcode, unsigned char value)
{
    unsigned char command;

    if (opcode == SERIAL_OPCODE_BANK && value <= 15)
        command = CMD_BANK_PREFIX | value;
    else if (opcode == SERIAL_OPCODE_JIFFY && value <= 1)
        command = CMD_JIFFY_PREFIX | value;
    else if (opcode == SERIAL_OPCODE_TEMP_BANK && value == 1)
        command = CMD_TEMP_BANK_PREFIX | value;
    else
        return false;

    return send_command(command);
}

bool send_command(unsigned char command)
{
    unsigned char saved_port;
    unsigned char saved_ddr;
    unsigned char port_value;
    unsigned int timeout;
    bool acknowledged = false;
    bool data_released = false;

    saved_port = PEEK(CIA2_PRB);
    saved_ddr = PEEK(CIA2_DDRB);

    port_value = saved_port | SERIAL_DATA_MASK | SERIAL_CLOCK_MASK;
    POKE(CIA2_PRB, port_value);
    POKE(CIA2_DDRB, saved_ddr | SERIAL_DATA_MASK | SERIAL_CLOCK_MASK);
    delay_units(HALF_CYCLE_DELAY * 10);

    send_byte(command, &port_value);

    port_value &= (unsigned char)~SERIAL_CLOCK_MASK;
    POKE(CIA2_PRB, port_value);
    POKE(CIA2_DDRB,
         (saved_ddr | SERIAL_CLOCK_MASK) &
         (unsigned char)~SERIAL_DATA_MASK);

    for (timeout = 0; timeout < 300; timeout++)
    {
        if (PEEK(CIA2_PRB) & SERIAL_DATA_MASK)
        {
            data_released = true;
            break;
        }
        delay_units(HALF_CYCLE_DELAY);
    }

    if (!data_released)
    {
        POKE(CIA2_PRB, saved_port);
        POKE(CIA2_DDRB, saved_ddr);
        return false;
    }

    for (timeout = 0; timeout < ACK_TIMEOUT_STEPS; timeout++)
    {
        if ((PEEK(CIA2_PRB) & SERIAL_DATA_MASK) == 0)
        {
            acknowledged = true;
            break;
        }
        delay_units(HALF_CYCLE_DELAY);
    }

    if (acknowledged)
    {
        for (timeout = 0; timeout < ACK_TIMEOUT_STEPS; timeout++)
        {
            if (PEEK(CIA2_PRB) & SERIAL_DATA_MASK)
                break;
            delay_units(HALF_CYCLE_DELAY);
        }
    }

    POKE(CIA2_PRB, saved_port);
    POKE(CIA2_DDRB, saved_ddr);

    return acknowledged;
}

void send_byte(unsigned char value, unsigned char *port_value)
{
    unsigned char mask;

    for (mask = 0x80; mask != 0; mask >>= 1)
    {
        *port_value &= (unsigned char)~SERIAL_CLOCK_MASK;
        POKE(CIA2_PRB, *port_value);

        if (value & mask)
            *port_value |= SERIAL_DATA_MASK;
        else
            *port_value &= (unsigned char)~SERIAL_DATA_MASK;
        POKE(CIA2_PRB, *port_value);
        delay_units(HALF_CYCLE_DELAY);

        *port_value |= SERIAL_CLOCK_MASK;
        POKE(CIA2_PRB, *port_value);
        delay_units(HALF_CYCLE_DELAY);
    }
}

int mainmenu()
{
    int rom_selected = 0;
    int jiffy_selected = 0;
    unsigned char key;

    // Draw static elements
    draw_title_bar();
    draw_fkey_bar();
    draw_util_bar();

    // Start with ROM selection screen
    current_screen = 0;
    draw_rom_screen(rom_selected);

    while (1)
    {
        key = cgetc();

        if (basic_reset_armed)
            continue;

        // Handle F-key navigation first
        switch (key)
        {
        case CH_F1:
            if (current_screen != 0)
            {
                current_screen = 0;
                draw_fkey_bar();
                draw_rom_screen(rom_selected);
            }
            continue;
        case CH_F2:
            if (current_screen != 1)
            {
                current_screen = 1;
                draw_fkey_bar();
                draw_jiffy_screen(jiffy_selected);
            }
            continue;
        case CH_F3:
            if (current_screen != 2)
            {
                current_screen = 2;
                draw_fkey_bar();
                draw_info_screen();
            }
            continue;
        case CH_F4:
            show_status_message("Switching to C64 Mode...", COLOR_LIGHTGREEN, 2);
            clrscr();
            c64mode(); // Goodbay folks
            break;
        case CH_F5:
            show_status_message("Preparing BASIC...", COLOR_CYAN, 1);
            if (send_tiny_command(SERIAL_OPCODE_TEMP_BANK, 1))
            {
                basic_reset_armed = true;
                show_status_message("BASIC armed. Press RESET.", COLOR_LIGHTGREEN, 3);
            }
            else
                show_status_message("ERROR: Ultra36 did not acknowledge.", COLOR_LIGHTRED, 3);
            continue;
        case CH_F6:
            current_screen = 3;
            draw_fkey_bar();
            draw_vdc_info_screen(SCREENW);
            break;
        case CH_F7:
            previous_screen = current_screen;
            current_screen = 4;
            draw_sid_info_screen(SCREENW);
            current_screen = previous_screen;
            draw_fkey_bar();
            draw_util_bar();
            switch (current_screen)
            {
            case 0:
                draw_rom_screen(rom_selected);
                break;
            case 1:
                draw_jiffy_screen(jiffy_selected);
                break;
            case 2:
                draw_info_screen();
                break;
            case 3:
                draw_vdc_info_screen(SCREENW);
                break;
            }
            break;
        }

        // Handle screen-specific navigation
        switch (current_screen)
        {
        case 0: // ROM selection
            if (key == CH_ENTER)
            {
                char buffer[40];
                sprintf(buffer, "Sending %s...", romNames[rom_selected]);
                show_status_message(buffer, COLOR_CYAN, 1);
                if (send_tiny_command(SERIAL_OPCODE_BANK, rom_selected + 1))
                    show_status_message("Saved. Reset to activate ROM bank.", COLOR_LIGHTGREEN, 2);
                else
                    show_status_message("ERROR: Ultra36 did not acknowledge.", COLOR_LIGHTRED, 3);
            }
            {
                int old_selected = rom_selected;
                rom_selected = handle_selection(rom_selected, NUM_ROMS, key);
                if (old_selected != rom_selected)
                {
                    draw_options_colors(NUM_ROMS, rom_selected); // Only update colors!
                }
            }
            break;

            // Case 1: JiffyDOS toggle - replace the draw_options call
        case 1: // JiffyDOS toggle
            if (key == CH_ENTER)
            {
                show_status_message("Sending JiffyDOS setting...", COLOR_CYAN, 1);

                // jiffy_selected == 0 → ON → pass 1
                // jiffy_selected == 1 → OFF → pass 0
                if (send_tiny_command(SERIAL_OPCODE_JIFFY, jiffy_selected == 0 ? 1 : 0))
                    show_status_message("Saved. Reset to apply JiffyDOS.", COLOR_LIGHTGREEN, 2);
                else
                    show_status_message("ERROR: Ultra36 did not acknowledge.", COLOR_LIGHTRED, 3);
            }
            {
                int old_selected = jiffy_selected;
                jiffy_selected = handle_selection(jiffy_selected, 2, key);
                if (old_selected != jiffy_selected)
                {
                    draw_options_colors(2, jiffy_selected); // Only update colors!
                }
            }
            break;
        case 2: // Info screen
            // Info screen is static, just wait for F-key navigation
            break;
        }
    }

    return 0;
}

void draw_title_bar(void)
{
    fill_line(0, COLOR_LIGHTBLUE, 0);
    textcolor(COLOR_WHITE);
    gotoxy((SCREENW - 22) / 2, 0);
    cputs("ULTRA-36 ROM MANAGER");
    textcolor(COLOR_LIGHTBLUE);
    gotoxy(SCREENW - 5, 0); // 5 is length of "0.0.1"
    cputs(APP_VERSION);
    textcolor(COLOR_GRAY3);
}

void draw_fkey_bar(void)
{
    unsigned char i;
    unsigned char x = 1;

    fill_line(1, COLOR_GRAY3, 1);
    for (i = 0; i < 3; i++)
    {
        gotoxy(x, 1);
        if (i == current_screen)
        {
            textcolor(COLOR_VIOLET);
            revers(1);
        }
        else
        {
            textcolor(COLOR_GRAY3);
            revers(1);
        }
        cputs(fkeyLabels[i]);
        x += strlen(fkeyLabels[i]) + 1;
    }

    revers(0);
    textcolor(COLOR_GRAY3);
}

void draw_rom_screen(int selected)
{
    draw_content_area("Select ROM bank:", romNames, NUM_ROMS, selected);
}

void draw_jiffy_screen(int selected)
{
    draw_content_area("Toggle JiffyDOS setting:", jiffyOptions, 2, selected);
}

void draw_content_area(const char *title, const char *options[], int count, int selected)
{
    unsigned char i;

    clear_menu_transition_rows();

    // Keep a fixed, framed work area on both the 40- and 80-column displays.
    for (i = 3; i <= 20; i++)
    {
        cclearxy(0, i, SCREENW);
    }

    draw_main_frame(title);

    draw_options_initial(options, count, selected);
    on_screen_instructions(count == 2);
}

void on_screen_instructions(const bool isJiffy)
{
    draw_frame_rule(14);
    textcolor(COLOR_GRAY3);
    cputsxy(2, 15, "UP/DOWN selects    ENTER applies");
    cputsxy(2, 16, "Saved settings take effect on reset.");
    textcolor(COLOR_LIGHTGREEN);
    cputsxy(2, 17, "Hold RESET 3 sec: return to menu.");
    textcolor(COLOR_GRAY3);
    if (isJiffy == false) {
        cputsxy(2, 18, "Empty bank gives a clean C128 state.");
    }
}

void draw_options_initial(const char *options[], int count, int selected)
{
    unsigned char i;
    (void)options;

    /* The menu is always centred in the same visual panel.  Two columns
     * preserve a useful selection width even on the 40-column VIC display. */
    for (i = 0; i < count; i++)
        draw_option(i, count, i == selected);
}

void draw_options_colors(int count, int selected)
{
    static int last_selected = -1;
    static int last_screen = -1;
    unsigned char i;

    if (last_selected == -1 || last_screen != current_screen)
    {
        for (i = 0; i < count; i++)
            draw_option(i, count, i == selected);
        last_screen = current_screen;
    }
    else
    {
        if (last_selected != selected && last_selected < count)
            draw_option(last_selected, count, 0);

        draw_option(selected, count, 1);
    }

    last_selected = selected;
    textcolor(COLOR_GRAY3);
    revers(0);
}

// Helper function to calculate item position
void get_item_position(unsigned char item_index, int total_count, unsigned char *x, unsigned char *y)
{
    unsigned char items_per_column;
    unsigned char use_two_columns;

    use_two_columns = (total_count > 7);

    if (use_two_columns)
    {
        items_per_column = (total_count + 1) / 2;

        if (item_index >= items_per_column)
        {
            // Right column
            *x = SCREENW / 2 + 1;
            *y = 5 + (item_index - items_per_column);
        }
        else
        {
            // Left column
            *x = 1;
            *y = 5 + item_index;
        }
    }
    else
    {
        // Single column
        *x = 1;
        *y = 5 + item_index;
    }
}

void draw_option(int option_num, int total_count, int is_selected)
{
    const char *label;
    unsigned char line_x, line_y;
    unsigned char column_width;
    unsigned char i;
    unsigned char label_length;

    get_item_position(option_num, total_count, &line_x, &line_y);
    column_width = (total_count > 7) ? (SCREENW / 2 - 2) : (SCREENW - 4);

    if (current_screen == 0)
        label = romNames[option_num];
    else
        label = jiffyOptions[option_num];

    label_length = strlen(label);
    gotoxy(line_x, line_y);

    if (is_selected)
    {
        textcolor(COLOR_GRAY3);
        revers(1);
    }
    else
    {
        textcolor(COLOR_GRAY3);
        revers(0);
    }

    cputc(is_selected ? '>' : ' ');
    cputc(' ');
    cputs(label);
    for (i = label_length + 2; i < column_width; i++)
        cputc(' ');

    revers(0);
    textcolor(COLOR_GRAY3);
}

int handle_selection(int selected, int max_items, unsigned char key)
{
    switch (key)
    {
    case CH_CURS_UP:
        if (selected > 0)
            selected--;
        break;
    case CH_CURS_DOWN:
        if (selected < max_items - 1)
            selected++;
        break;
    }
    return selected;
}

void draw_info_screen(void)
{
    unsigned char i;

    clear_menu_transition_rows();

    for (i = 3; i <= 20; i++)
    {
        cclearxy(0, i, SCREENW);
    }

    draw_main_frame("ABOUT ULTRA-36");
    textcolor(COLOR_WHITE);
    cputsxy(2, 5, "ROM switcher for Commodore 128");
    cputsxy(2, 6, "Version ");
    cputsxy(10, 6, APP_VERSION);
    cputsxy(2, 8, "* 8 or 16 switchable ROM banks");
    cputsxy(2, 9, "* JiffyDOS setting stored in flash");
    cputsxy(2, 10, "* VIC-II 40 and VDC 80 columns");
    cputsxy(2, 12, "Selection is remembered by Ultra-36.");
    draw_frame_rule(14);
    textcolor(COLOR_GRAY3);
    cputsxy(2, 15, "F1-F3: sections    F4-F7: tools");
    cputsxy(2, 16, "UP/DOWN: move      ENTER: apply");
    textcolor(COLOR_LIGHTGREEN);
    cputsxy(2, 18, "RESET 3 sec returns to this menu.");
    textcolor(COLOR_GRAY3);
}

void draw_util_bar(void)
{
    fill_line(22, COLOR_LIGHTBLUE, 0);
    fill_line(23, COLOR_BLUE, 0);
    fill_line(24, COLOR_BLUE, 0);

    // Bottom shortcuts deliberately use the same compact key-cap treatment
    // as the menu strip, leaving enough room for VIC 40 columns.
    gotoxy(1, 23);
    revers(1);
    textcolor(COLOR_GRAY3);
    cputs(" F4 ");
    revers(0);
    textcolor(COLOR_CYAN);
    cputs(" C64");

    gotoxy(SCREENW / 2 + 1, 23);
    revers(1);
    textcolor(COLOR_GRAY3);
    cputs(" F5 ");
    revers(0);
    textcolor(COLOR_CYAN);
    cputs(" Restart");

    gotoxy(1, 24);
    revers(1);
    textcolor(COLOR_GRAY3);
    cputs(" F6 ");
    revers(0);
    textcolor(COLOR_CYAN);
    cputs(" VDC Info");

    gotoxy(SCREENW / 2 + 1, 24);
    revers(1);
    textcolor(COLOR_GRAY3);
    cputs(" F7 ");
    revers(0);
    textcolor(COLOR_CYAN);
    cputs(" SID Info");
    textcolor(COLOR_GRAY3);
}

void fill_line(unsigned char y, unsigned char color, unsigned char reversed)
{
    unsigned char i;

    gotoxy(0, y);
    textcolor(color);
    revers(reversed);
    for (i = 0; i < SCREENW; i++)
        cputc(' ');
    revers(0);
}

/* The VDC diagnostic owns its title on row 2 and its last colour test on
 * row 21. Clear those rows whenever a regular menu page takes the screen
 * back, then restore the footer separator. */
void clear_menu_transition_rows(void)
{
    revers(0);
    textcolor(COLOR_GRAY3);
    cclearxy(0, 2, SCREENW);
    cclearxy(0, 21, SCREENW);
    fill_line(22, COLOR_LIGHTBLUE, 0);
}

void draw_frame_rule(unsigned char y)
{
    unsigned char x;

    textcolor(COLOR_LIGHTBLUE);
    cputsxy(0, y, "+");
    for (x = 1; x < SCREENW - 1; x++)
        cputc('-');
    cputc('+');
}

void draw_main_frame(const char *title)
{
    unsigned char y;

    draw_frame_rule(3);
    draw_frame_rule(20);
    textcolor(COLOR_LIGHTBLUE);
    for (y = 4; y < 20; y++)
    {
        cputsxy(0, y, "|");
        cputsxy(SCREENW - 1, y, "|");
    }
    textcolor(COLOR_WHITE);
    cputsxy(3, 3, title);
    textcolor(COLOR_GRAY3);
}

void show_status_message(const char *message, unsigned char color,
                         unsigned char seconds)
{
    fill_line(21, COLOR_BLUE, 0);
    textcolor(color);
    cputsxy(1, 21, message);
    textcolor(COLOR_GRAY3);
    sleep(seconds);
    fill_line(21, COLOR_BLUE, 0);
}
