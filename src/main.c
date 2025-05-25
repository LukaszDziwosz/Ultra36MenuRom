#include <stdio.h>
#include <conio.h>
#include <peekpoke.h>
#include <c128.h>
#include <accelerator.h>

// Forward declaration
int mainmenu();
void draw_rom_label(void);
int process_menu_key(int selected);

// Global screen width so other functions can use it
unsigned char SCREENW;

int main(void) {
    int selected;

    // Detect screen width (VIC or VDC)
    if (PEEK(0x00EE) == 79) {
        SCREENW = 80;
        set_c128_speed(SPEED_FAST);
    } else {
        SCREENW = 40;
    }

    clrscr();

    selected = mainmenu();

    clrscr();
    cprintf("You selected ROM bank #%d\n", selected + 1);
    cputsxy(0, 2, "Press any key to exit...");
    cgetc();

    set_c128_speed(SPEED_SLOW);
    return 0;
}

int mainmenu() {
    const char* romNames[] = {
        ROM1_NAME, ROM2_NAME, ROM3_NAME, ROM4_NAME,
        ROM5_NAME, ROM6_NAME, ROM7_NAME
    };
    const char* fkeyLabels[] = {
        "F1: ROM Select",
        "F2: JiffyDOS",
        "F3: Info"
    };

    int selected = 0;
    int previous = -1;
    unsigned char key;
    unsigned char i;

    // Title Bar
    gotoxy(0, 0);
    revers(1);
    textcolor(COLOR_CYAN);
    for (i = 0; i < SCREENW; i++) cputc(' ');
    
    // Center the main title
    gotoxy((SCREENW - 18) / 2, 0);  // 18 is length of "Ultra-36 ROM Menu"
    cputs("Ultra-36 ROM Menu");
    
    // Place version at the right
    gotoxy(SCREENW - 6, 0);  // 6 is length of "v0.0.1"
    cputs("v0.0.1");
    
    revers(0);

    // F-key menu bar (line 2)
    gotoxy(0, 1);
    revers(1);
    textcolor(COLOR_LIGHTRED);
    for (i = 0; i < SCREENW; i++) cputc(' ');  // Fill entire line with spaces first
    gotoxy(0, 1);  // Go back to start of line
    for (i = 0; i < 3; i++) {
        cprintf("%s ", fkeyLabels[i]);
        if (i < 2) cputc(' ');
    }
    revers(0);

    // ROM list label
    draw_rom_label();

    while (1) {
        int result;

        if (selected != previous) {
            for (i = 0; i < NUM_ROMS; i++) {
                gotoxy(2, i + 6);
                cclear(SCREENW - 4);

                if (i == selected) {
                    textcolor(COLOR_YELLOW);
                    revers(1);
                }

                gotoxy(2, i + 6);
                cprintf("%d. %s", i + 1, romNames[i]);

                revers(0);
                textcolor(COLOR_WHITE);
            }
            previous = selected;
        }

        result = process_menu_key(selected);
        if (result == -100) return selected;
        selected = result;
    }
}

void draw_rom_label(void) {
    textcolor(COLOR_WHITE);
    gotoxy(0, 4);
    cputs("Select ROM bank:");
}

int process_menu_key(int selected) {
    unsigned char key = cgetc();

    switch (key) {
        case CH_CURS_UP:
            if (selected > 0) selected--;
            break;
        case CH_CURS_DOWN:
            if (selected < NUM_ROMS - 1) selected++;
            break;
        case CH_ENTER:
            return -100; // Special value to signal exit
        case CH_F1:
            gotoxy(0, 24); cclear(SCREENW);
            cputsxy(0, 24, "You pressed F1 - ROM Select");
            break;
        case CH_F2:
            gotoxy(0, 24); cclear(SCREENW);
            cputsxy(0, 24, "You pressed F2 - JiffyDOS");
            break;
        case CH_F3:
            gotoxy(0, 24); cclear(SCREENW);
            cputsxy(0, 24, "You pressed F3 - Info");
            break;
    }

    return selected;
}
