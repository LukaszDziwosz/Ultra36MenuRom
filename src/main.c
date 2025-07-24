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
#include <conio.h>
#include <peekpoke.h>
#include <c128.h>

#include "vdc_info_screen.h"
#include "sid_info_screen.h"

#define APP_VERSION "0.0.4"
#define IO_TINY_COMMAND 0xD700

typedef int bool;
#define true 1
#define false 0

// Forward declarations
int mainmenu();
void send_tiny_command(unsigned char command);
void delay_ms(unsigned int ms, unsigned char isFast);
void draw_title_bar(void);
void draw_fkey_bar(void);
void draw_content_area(const char *title, const char *options[], int count, int selected);
void draw_options_initial(const char *options[], int count, int selected);
void draw_options_colors(int count, int selected);
void update_option_color(int option_num, int is_selected, unsigned char line_x, unsigned char line_y);
void get_item_position(unsigned char item_index, int total_count, unsigned char *x, unsigned char *y);
int handle_selection(int selected, int max_items, unsigned char key);
void draw_rom_screen(int selected);
void draw_jiffy_screen(int selected);
void draw_info_screen(void);
void show_status_message(const char *message);
void on_screen_instructions(const bool isJiffy);
void draw_util_bar(void);

// Global variables
unsigned char SCREENW;
int current_screen = 0; // 0=ROM, 1=JiffyDOS, 2=Info
int previous_screen = 0;

const char *romNames[] = {ROM_NAMES_INIT};

const char *jiffyOptions[] = {
    "JiffyDOS ON",
    "JiffyDOS OFF"};

const char *fkeyLabels[] = {
    "F1: ROM Select",
    "F2: JiffyDOS",
    "F3: Info"};

int main(void)
{
    int result;

    // Detect screen width (VIC or VDC)
    if (PEEK(0x00EE) == 79)
    {
        SCREENW = 80;
        fast();
        bgcolor(COLOR_GRAY2);
    }
    else
    {
        SCREENW = 40;
        bordercolor(COLOR_GRAY1);
    }

    clrscr();

    result = mainmenu();

    // Clean up before exit
    clrscr();
    slow();
    return result;
}

void delay_ms(unsigned int ms, unsigned char isFast)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < (isFast ? 60 : 30); j++)
        {
            __asm__("nop");
        }
    }
}

void send_tiny_command(unsigned char command)
{
    unsigned char i;
    unsigned char isFast = (SCREENW == 80); // 2 MHz = fast

    for (i = 0; i < command; i++)
    {
        POKE(IO_TINY_COMMAND, 0xFF);
        if (isFast)
        {
            bgcolor(COLOR_RED);
        }
        else
        {
            bordercolor(COLOR_RED);
        }
        delay_ms(2, isFast); // ~2 ms visible flash
        if (isFast)
        {
            bgcolor(COLOR_GRAY2);
        }
        else
        {
            bordercolor(COLOR_GRAY1);
        }
        delay_ms(2, isFast); // ~2 ms spacing
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
            show_status_message("Switching to C64 Mode...");
            clrscr();
            c64mode(); // Goodbay folks
            break;
        case CH_F5:
            show_status_message("Restarting the Menu program..");
            return EXIT_SUCCESS;
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
                sprintf(buffer, "%s selected", romNames[rom_selected]);
                show_status_message(buffer);
                send_tiny_command(rom_selected + 4); // Bank 0 becomes 1, up to 16
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
                show_status_message(jiffy_selected == 0 ? "JiffyDOS enabled!" : "JiffyDOS disabled!");

                // jiffy_selected == 0 → ON → pass 1
                // jiffy_selected == 1 → OFF → pass 0
                send_tiny_command(jiffy_selected == 0 ? 2 : 3);
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
}

void draw_title_bar(void)
{
    unsigned char i;

    gotoxy(0, 0);
    revers(1);
    textcolor(COLOR_CYAN);

    // Fill entire line with spaces
    for (i = 0; i < SCREENW; i++)
        cputc(' ');

    // Center the main title
    gotoxy((SCREENW - 17) / 2, 0); // 18 is length of "Ultra-36 ROM Menu"
    cputs("Ultra-36 ROM Menu");

    // Place version at the right
    gotoxy(SCREENW - 5, 0); // 5 is length of "0.0.1"
    cputs(APP_VERSION);

    revers(0);
}

void draw_fkey_bar(void)
{
    unsigned char i;

    gotoxy(0, 1);
    revers(1);
    textcolor(COLOR_LIGHTRED);

    // Fill entire line with spaces
    for (i = 0; i < SCREENW; i++)
        cputc(' ');

    // Draw F-key labels
    gotoxy(0, 1);
    for (i = 0; i < 3; i++)
    {
        // Highlight current screen
        if (i == current_screen)
        {
            textcolor(COLOR_YELLOW);
        }
        else
        {
            textcolor(COLOR_LIGHTRED);
        }
        cprintf("%s", fkeyLabels[i]);
        textcolor(COLOR_LIGHTRED); // Highlight only text
        cputs("  ");               // keep the space red
    }

    revers(0);
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

    // Clear content area (lines 3-21) - avoid utility bar at 22-23
    for (i = 3; i < 23; i++)
    {
        cclearxy(0, i, SCREENW);
    }

    // Draw title
    textcolor(COLOR_WHITE);
    cputsxy(0, 4, title);

    // Draw options using the initial draw function
    draw_options_initial(options, count, selected);

    on_screen_instructions(count == 2);
}

void on_screen_instructions(const bool isJiffy)
{
    cputsxy(1, 15, "Use UP/DOWN to select, ENTER to apply.");
    cputsxy(1, 16, "Wait for blinking to complete.");
    cputsxy(1, 17, "Reboot or reset to take effect!");
    textcolor(COLOR_LIGHTGREEN);
    cputsxy(1, 18, "Hold reset for 3s to return to Menu");
    textcolor(COLOR_WHITE);
    if (isJiffy == false) {
        cputsxy(1, 19, "Empty bank ensures clean C128 state.");
    }
}

void draw_options_initial(const char *options[], int count, int selected)
{
    unsigned char i;
    unsigned char items_per_column;
    unsigned char use_two_columns = 0;
    unsigned char max_lines;
    unsigned char col_x, col_y;

    // Determine layout: single column for <=7 ROMs, two columns for >7 ROMs
    if (count > 7)
    {
        use_two_columns = 1;
        items_per_column = (count + 1) / 2; // Round up for left column
    }
    else
    {
        items_per_column = count;
    }

    // Clear the options area - need to clear more lines for two columns
    max_lines = use_two_columns ? items_per_column : count;
    for (i = 0; i < max_lines; i++)
    {
        gotoxy(1, 6 + i); // Start at x=1 (moved left one space)
        cclear(SCREENW - 1);
    }

    // Draw all option text (without colors yet)
    textcolor(COLOR_WHITE);
    revers(0);

    for (i = 0; i < count; i++)
    {
        if (use_two_columns && i >= items_per_column)
        {
            // Right column
            col_x = SCREENW / 2 + 1;
            col_y = 6 + (i - items_per_column);
        }
        else
        {
            // Left column (or single column)
            col_x = 1;
            col_y = 6 + i;
        }

        cputsxy(col_x + 1, col_y, options[i]); // here if I change to 1 problem
    }

    // Now apply colors for the selected item
    draw_options_colors(count, selected);
}

void draw_options_colors(int count, int selected)
{
    static int last_selected = -1;
    static int last_screen = -1;
    unsigned char old_x, old_y;
    unsigned char i;
    unsigned char item_x, item_y;

    // Save current cursor position
    old_x = wherex();
    old_y = wherey();

    // If this is the first call or we switched screens, update all items
    if (last_selected == -1 || last_screen != current_screen)
    {
        for (i = 0; i < count; i++)
        {
            get_item_position(i, count, &item_x, &item_y);
            update_option_color(i, i == selected, item_x, item_y);
        }
        last_screen = current_screen;
    }
    else
    {
        // Only update the previously selected item (turn off highlight)
        if (last_selected != selected && last_selected < count)
        {
            get_item_position(last_selected, count, &item_x, &item_y);
            update_option_color(last_selected, 0, item_x, item_y);
        }

        // Update the newly selected item (turn on highlight)
        get_item_position(selected, count, &item_x, &item_y);
        update_option_color(selected, 1, item_x, item_y);
    }

    last_selected = selected;

    // Restore cursor position and reset attributes
    gotoxy(old_x, old_y);
    textcolor(COLOR_WHITE);
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
            *y = 6 + (item_index - items_per_column);
        }
        else
        {
            // Left column
            *x = 1;
            *y = 6 + item_index;
        }
    }
    else
    {
        // Single column
        *x = 1;
        *y = 6 + item_index;
    }
}

// Updated update_option_color function with explicit x,y parameters
void update_option_color(int option_num, int is_selected, unsigned char line_x, unsigned char line_y)
{
    gotoxy(line_x + 1, line_y); // Match the +3 offset from draw_options_initial

    if (is_selected)
    {
        textcolor(COLOR_YELLOW);
        revers(1);
    }
    else
    {
        textcolor(COLOR_WHITE);
        revers(0);
    }

    // Just redraw the text instead of using cpeekc/cputc
    if (current_screen == 0)
    {
        cputs(romNames[option_num]);
    }
    else if (current_screen == 1)
    {
        cputs(jiffyOptions[option_num]);
    }

    // Reset attributes
    revers(0);
    textcolor(COLOR_WHITE);
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

// Update info screen to not overwrite utility bar
void draw_info_screen(void)
{
    unsigned char i;

    // Clear content area (avoid utility bar)
    for (i = 3; i < 22; i++)
    {
        cclearxy(0, i, SCREENW);
    }

    // Draw info content
    textcolor(COLOR_WHITE);
    cputsxy(0, 3, "Ultra-36 ROM Switcher Information");
    cputsxy(0, 4, "Version: ");
    cputsxy(9, 4, APP_VERSION);
    cputsxy(14, 4, " - Author: Lukasz Dziwosz");
    cputsxy(0, 6, "Features:");
    cputsxy(2, 7, "- Switch between 8/16 ROM banks");
    cputsxy(2, 8, "- Toggle JiffyDOS on/off");
    cputsxy(2, 9, "- VIC-II and VDC support");
    cputsxy(0, 10, "Selection will be remembered.");
    textcolor(COLOR_YELLOW);
    cputsxy(0, 12, "Hold reset for 3 seconds,");
    cputsxy(0, 13, "to return to Menu (Bank 0)");
    textcolor(COLOR_WHITE);
    cputsxy(0, 15, "Controls:");
    cputsxy(2, 16, "F1/F2/F3 - Switch between screens");
    cputsxy(2, 17, "UP/DOWN  - Navigate options");
    cputsxy(2, 18, "ENTER    - Select/Apply");

    cputsxy(0, 20, "Thanks to:  Jim Brain, Jani");
    cputsxy(0, 21, "Xander Mol, Maciej Witkowiak");
}

void draw_util_bar(void)
{
    unsigned char i;

    // Clear the utility bar area first
    for (i = 23; i <= 24; i++)
    {
        cclearxy(0, i, SCREENW);
    }

    // Left side - F4: Go 64
    gotoxy(1, 23);
    revers(1);
    textcolor(COLOR_CYAN);
    cputs("F4:");
    revers(0);
    cputs(" Go 64");

    // Right side - F5: Go 128
    gotoxy(SCREENW / 2 + 1, 23); // <-- Inline calculation
    revers(1);
    cputs("F5:");
    revers(0);
    cputs(" Restart");

    // Left side - F6: VDC Info
    gotoxy(1, 24);
    revers(1);
    cputs("F6:");
    revers(0);
    cputs(" VDC Info");

    // Right side - F7: SID Info
    gotoxy(SCREENW / 2 + 1, 24); // <-- Inline calculation
    revers(1);
    cputs("F7:");
    revers(0);
    cputs(" SID Info");
}

void show_status_message(const char *message)
{
    cclearxy(1, 21, SCREENW);
    textcolor(COLOR_LIGHTGREEN);
    cputsxy(1, 21, message);
    textcolor(COLOR_WHITE);
    // Brief pause to show the message
    sleep(1);
    // Clear the status line
    cclearxy(1, 21, SCREENW);
}
