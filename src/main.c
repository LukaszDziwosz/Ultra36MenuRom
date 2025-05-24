#include <stdio.h>
#include <conio.h>
#include <peekpoke.h>
#include <c128.h>
#include <accelerator.h>

// Forward declaration
int mainmenu();

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
    int selected = 0;
    int previous = -1;
    unsigned char key;
    unsigned char i;

    // Title Bar
    gotoxy(0, 0);
    revers(1);
    textcolor(COLOR_CYAN);
    for (i = 0; i < SCREENW; i++) cputc(' ');
    gotoxy((SCREENW - 20) / 2, 0);
    cputs("Ultra-36 ROM Menu");
    revers(0);
    textcolor(COLOR_WHITE);

    gotoxy(0, 1);
    cputs("Select ROM bank:");

    while (1) {
        if (selected != previous) {
            for (i = 0; i < NUM_ROMS; i++) {
                gotoxy(2, i + 3);
                cclear(SCREENW - 4);

                if (i == selected) {
                    textcolor(COLOR_YELLOW);
                    revers(1);
                }

                gotoxy(2, i + 3);
                cprintf("%d. %s", i + 1, romNames[i]);

                revers(0);
                textcolor(COLOR_WHITE);
            }
            previous = selected;
        }

        key = cgetc();

        switch (key) {
            case CH_CURS_UP:
                if (selected > 0) selected--;
                break;
            case CH_CURS_DOWN:
                if (selected < NUM_ROMS - 1) selected++;
                break;
            case CH_ENTER:
                return selected;
        }
    }
}
