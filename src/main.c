// main.c

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <conio.h>
#include <peekpoke.h>
#include <c128.h>
#include <accelerator.h>

// Forward declarations
int mainmenu();
void draw_title_bar(void);
void draw_fkey_bar(void);
void draw_content_area(const char* title, const char* options[], int count, int selected);
void draw_options(const char* options[], int count, int selected);
int handle_navigation(int selected, int max_items, unsigned char key);
void draw_rom_screen(int selected);
void draw_jiffy_screen(int selected);
void draw_info_screen(void);
void show_status_message(const char* message);
void on_screen_instructions(void);
void draw_util_bar(void);

// Global variables
unsigned char SCREENW;
int current_screen = 0; // 0=ROM, 1=JiffyDOS, 2=Info

// Menu definitions
const char* romNames[] = {
    ROM1_NAME, ROM2_NAME, ROM3_NAME, ROM4_NAME,
    ROM5_NAME, ROM6_NAME, ROM7_NAME
};

const char* jiffyOptions[] = {
    "JiffyDOS ON",
    "JiffyDOS OFF"
};

const char* fkeyLabels[] = {
    "F1: ROM Select",
    "F2: JiffyDOS",
    "F3: Info"
};

const char* utilKeyLabels[] = {
    "F4: Go 128",
    "F5: Go 64",
    "F6: VDC Info",
    "F7: SID Info"
};

int main(void) {
    int result;

    // Detect screen width (VIC or VDC)
    if (PEEK(0x00EE) == 79) {
        SCREENW = 80;
        set_c128_speed(SPEED_FAST);
    } else {
        SCREENW = 40;
    }

    clrscr();

    result = mainmenu();

    // Clean up before exit
    clrscr();
    set_c128_speed(SPEED_SLOW);
    return result;
}

int mainmenu() {
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

    while (1) {
        key = cgetc();

        // Handle F-key navigation first
        switch (key) {
            case CH_F1:
                if (current_screen != 0) {
                    current_screen = 0;
                    draw_fkey_bar();
                    draw_rom_screen(rom_selected);
                }
                continue;
            case CH_F2:
                if (current_screen != 1) {
                    current_screen = 1;
                    draw_fkey_bar();
                    draw_jiffy_screen(jiffy_selected);
                }
                continue;
            case CH_F3:
                if (current_screen != 2) {
                    current_screen = 2;
                    draw_fkey_bar();
                    draw_info_screen();
                }
                continue;
            case CH_F4:
                show_status_message("Switching to C64 Mode...");
                // #define EXEC_BOOT 0x10, #define EXEC_RUN64 0x04 ??
                // Find a way to reboot in c64 mode
                break;
            case CH_F5:
                show_status_message("Switching to C128 BASIC...");
                return EXIT_SUCCESS;
            case CH_F6:
                show_status_message("VDC Info: Not Implemented");
                break;
            case CH_F7:
                show_status_message("SID Info: Not Implemented");
                break;
        }

        // Handle screen-specific navigation
        switch (current_screen) {
            case 0: // ROM selection
                if (key == CH_ENTER) {
                     char buffer[40];
                     sprintf(buffer, "%s selected", romNames[rom_selected]);
                     show_status_message(buffer);
                }
                {
                    int old_selected = rom_selected;
                    rom_selected = handle_navigation(rom_selected, NUM_ROMS, key);
                    if (old_selected != rom_selected) {
                        draw_options(romNames, NUM_ROMS, rom_selected);
                        // Send command to Tiny85 here
                    }
                }
                break;

            case 1: // JiffyDOS toggle
                if (key == CH_ENTER) {
                    show_status_message(jiffy_selected == 0 ?
                        "JiffyDOS enabled!" : "JiffyDOS disabled!");
                    // Here you would actually apply the JiffyDOS setting
                }
                {
                    int old_selected = jiffy_selected;
                    jiffy_selected = handle_navigation(jiffy_selected, 2, key);
                    if (old_selected != jiffy_selected) {
                        draw_options(jiffyOptions, 2, jiffy_selected);
                    }
                }
                break;

            case 2: // Info screen
                // Info screen is static, just wait for F-key navigation
                break;
        }
    }
}

void draw_title_bar(void) {
    unsigned char i;
    
    gotoxy(0, 0);
    revers(1);
    textcolor(COLOR_CYAN);
    
    // Fill entire line with spaces
    for (i = 0; i < SCREENW; i++) cputc(' ');
    
    // Center the main title
    gotoxy((SCREENW - 18) / 2, 0);  // 18 is length of "Ultra-36 ROM Menu"
    cputs("Ultra-36 ROM Menu");
    
    // Place version at the right
    gotoxy(SCREENW - 6, 0);  // 6 is length of "v0.0.1"
    cputs("v0.0.1");
    
    revers(0);
}

void draw_fkey_bar(void) {
    unsigned char i;
    
    gotoxy(0, 1);
    revers(1);
    textcolor(COLOR_LIGHTRED);
    
    // Fill entire line with spaces
    for (i = 0; i < SCREENW; i++) cputc(' ');
    
    // Draw F-key labels
    gotoxy(0, 1);
    for (i = 0; i < 3; i++) {
        // Highlight current screen
        if (i == current_screen) {
            textcolor(COLOR_YELLOW);
        } else {
            textcolor(COLOR_LIGHTRED);
        }
        cprintf("%s", fkeyLabels[i]);
        textcolor(COLOR_LIGHTRED); // Highlight only text
        cputs("  "); // keep the space red
    }
    
    revers(0);
}

void draw_content_area(const char* title, const char* options[], int count, int selected) {
    unsigned char i;
    
    // Clear content area (lines 3-21) - avoid utility bar at 22-23
    for (i = 3; i < 22; i++) {
        cclearxy(0, i, SCREENW);
    }
    
    // Draw title
    textcolor(COLOR_WHITE);
    cputsxy(0, 4, title);
    
    // Draw options using the separated function
    draw_options(options, count, selected);
    
    textcolor(COLOR_WHITE);
}

void draw_options(const char* options[], int count, int selected) {
    unsigned char i;
    
    // Clear the options area only
    for (i = 0; i < count; i++) {
        gotoxy(2, 6 + i);
        cclear(SCREENW);
    }
    
    // Draw options
    for (i = 0; i < count; i++) {
        gotoxy(2, 6 + i);
        
        if (i == selected) {
            textcolor(COLOR_YELLOW);
            revers(1);
        } else {
            textcolor(COLOR_WHITE);
            revers(0);
        }
        
        cprintf("%d. %s", i + 1, options[i]);
        
        revers(0);
    }
    
    textcolor(COLOR_WHITE);
}

int handle_navigation(int selected, int max_items, unsigned char key) {
    switch (key) {
        case CH_CURS_UP:
            if (selected > 0) selected--;
            break;
        case CH_CURS_DOWN:
            if (selected < max_items - 1) selected++;
            break;
    }
    return selected;
}

void draw_rom_screen(int selected) {
    draw_content_area("Select ROM bank:", romNames, NUM_ROMS, selected);
    
    // Add instructions
    on_screen_instructions();
}

void draw_jiffy_screen(int selected) {
    draw_content_area("Toggle JiffyDOS setting:", jiffyOptions, 2, selected);
    
    // Add instructions
    on_screen_instructions();
}

void on_screen_instructions(void) {
    textcolor(COLOR_LIGHTBLUE);
    cputsxy(1, 16, "Use UP/DOWN to select,");
    cputsxy(1, 17, "Press ENTER to apply,");
    cputsxy(1, 18, "Reboot or reset to take effect!");
}


// Update info screen to not overwrite utility bar
void draw_info_screen(void) {
    unsigned char i;

    // Clear content area (avoid utility bar)
    for (i = 3; i < 22; i++) {
        cclearxy(0, i, SCREENW);
    }

    // Draw info content
    textcolor(COLOR_WHITE);
    cputsxy(0, 3, "Ultra-36 ROM Switcher Information");
    cputsxy(0, 4, "Version: 0.0.1 - Author: Lukasz Dziwosz");

    cputsxy(0, 6, "Features:");
    cputsxy(2, 7, "- Switch between 7 ROM banks");
    cputsxy(2, 8, "- Toggle JiffyDOS on/off");
    cputsxy(2, 9, "- VIC-II and VDC support");

    cputsxy(0, 10, "Selection will be remembered.");
    cputsxy(0, 11, "Hold reset for 3 seconds,");
    cputsxy(0, 12, "to return to Menu");

    cputsxy(0, 14, "Controls:");
    cputsxy(2, 15, "F1/F2/F3 - Switch between screens");
    cputsxy(2, 16, "UP/DOWN  - Navigate options");
    cputsxy(2, 17, "ENTER    - Select/Apply");

    cputsxy(0, 19, "Thanks to Xander Mol, M Witkowiak");
}

void draw_util_bar(void) {
    unsigned char i;
    unsigned char half_width = SCREENW / 2;
    
    // Clear the utility bar area first
    for (i = 23; i <= 24; i++) {
        cclearxy(0, i, SCREENW);
    }
    
    // Left side - F4: Go 64
    gotoxy(1, 23);
    revers(1); // Highlight F4
    textcolor(COLOR_CYAN);
    cputs("F4:");
    revers(0); // Un highlight the rest
    cputs(" Go 64");
    
    // Right side - F5: Go 128
    gotoxy(half_width + 1, 23);
    revers(1);
    cputs("F5:");
    revers(0);
    cputs(" Go 128");
    
    // Left side - F6: VDC Info
    gotoxy(1, 24);
    revers(1);
    cputs("F6:");
    revers(0);
    cputs(" VDC Info");
    
    // Right side - F7: SID Info
    gotoxy(half_width + 1, 24);
    revers(1);
    cputs("F7:");
    revers(0);
    cputs(" SID Info");
}

void show_status_message(const char* message) {
    cclearxy(1, 21, SCREENW);
    textcolor(COLOR_LIGHTGREEN);
    cputsxy(1, 21, message);
    textcolor(COLOR_WHITE);
    // Brief pause to show the message
    sleep(1);
    // Clear the status line
    cclearxy(1, 21, SCREENW);
}
