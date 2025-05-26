#include <stdio.h>
#include <conio.h>
#include <peekpoke.h>
#include <c128.h>
#include <accelerator.h>

// Forward declarations
int mainmenu();
void draw_title_bar(void);
void draw_fkey_bar(void);
void draw_content_area(const char* title, const char* options[], int count, int selected);
int handle_navigation(int selected, int max_items, unsigned char key);
void draw_rom_screen(int selected);
void draw_jiffy_screen(int selected);
void draw_info_screen(void);
void show_status_message(const char* message);

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

    if (result >= 0) {
        clrscr();
        cprintf("You selected ROM bank #%d\n", result + 1);
        cputsxy(0, 2, "Press any key to exit...");
        cgetc();
    }

    set_c128_speed(SPEED_SLOW);
    return 0;
}

int mainmenu() {
    int rom_selected = 0;
    int jiffy_selected = 0;
    unsigned char key;

    // Draw static elements
    draw_title_bar();
    draw_fkey_bar();

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
            case CH_ESC:
                return -1; // Exit without selection
        }

        // Handle screen-specific navigation
        switch (current_screen) {
            case 0: // ROM selection
                if (key == CH_ENTER) {
                    return rom_selected;
                }
                rom_selected = handle_navigation(rom_selected, NUM_ROMS, key);
                draw_rom_screen(rom_selected);
                break;

            case 1: // JiffyDOS toggle
                if (key == CH_ENTER) {
                    show_status_message(jiffy_selected == 0 ?
                        "JiffyDOS enabled!" : "JiffyDOS disabled!");
                    // Here you would actually apply the JiffyDOS setting
                }
                jiffy_selected = handle_navigation(jiffy_selected, 2, key);
                draw_jiffy_screen(jiffy_selected);
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
        if (i < 2) cputs("  ");
    }
    
    revers(0);
}

void draw_content_area(const char* title, const char* options[], int count, int selected) {
    unsigned char i;
    
    // Clear content area (lines 3-23)
    for (i = 3; i < 23; i++) {
        gotoxy(0, i);
        cclear(SCREENW);
    }
    
    // Draw title
    textcolor(COLOR_WHITE);
    gotoxy(0, 4);
    cputs(title);
    
    // Draw options
    for (i = 0; i < count; i++) {
        gotoxy(2, i + 6);
        
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
    gotoxy(0, 20);
    textcolor(COLOR_LIGHTBLUE);
    cputs("Use UP/DOWN arrows to select, ENTER to confirm");
}

void draw_jiffy_screen(int selected) {
    draw_content_area("Toggle JiffyDOS setting:", jiffyOptions, 2, selected);
    
    // Add instructions
    gotoxy(0, 20);
    textcolor(COLOR_LIGHTBLUE);
    cputs("Use UP/DOWN arrows to select, ENTER to apply");
}

void draw_info_screen(void) {
    unsigned char i;
    
    // Clear content area
    for (i = 3; i < 23; i++) {
        gotoxy(0, i);
        cclear(SCREENW);
    }
    
    // Draw info content
    textcolor(COLOR_WHITE);
    gotoxy(0, 4);
    cputs("Ultra-36 ROM Switcher Information");
    
    gotoxy(0, 6);
    cputs("Version: 0.0.1");
    gotoxy(0, 7);
    cputs("Author: [Your Name]");
    gotoxy(0, 8);
    cputs("Build Date: [Build Date]");
    
    gotoxy(0, 10);
    cputs("Features:");
    gotoxy(2, 11);
    cputs("- Switch between 7 ROM banks");
    gotoxy(2, 12);
    cputs("- Toggle JiffyDOS on/off");
    gotoxy(2, 13);
    cputs("- VIC-II and VDC support");
    
    gotoxy(0, 15);
    cputs("Controls:");
    gotoxy(2, 16);
    cputs("F1/F2/F3 - Switch between screens");
    gotoxy(2, 17);
    cputs("UP/DOWN  - Navigate options");
    gotoxy(2, 18);
    cputs("ENTER    - Select/Apply");
    gotoxy(2, 19);
    cputs("ESC      - Exit program");
}

void show_status_message(const char* message) {
    gotoxy(0, 23);
    cclear(SCREENW);
    textcolor(COLOR_LIGHTGREEN);
    cputsxy(0, 23, message);
    textcolor(COLOR_WHITE);
    
    // Brief pause to show the message
    {
        unsigned int delay;
        for (delay = 0; delay < 30000; delay++) {
            // Simple delay loop
        }
    }
    
    // Clear the status line
    gotoxy(0, 23);
    cclear(SCREENW);
}
