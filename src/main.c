#include <stdio.h>
#include <conio.h>
#include <peekpoke.h>
#include <c128.h>
#include <accelerator.h>

// Forward declaration
int mainmenu();

int main(void) {
    unsigned char SCREENW;
    int selected;

    // Detect screen width (VIC or VDC)
    if (PEEK(0x00EE) == 79) {
        SCREENW = 80;
        set_c128_speed(SPEED_FAST);
    } else {
        SCREENW = 40;
    }

    clrscr();
    cputsxy(0, 0, (SCREENW == 80) ? "Hello from ROM (80-col)" : "Hello from ROM (40-col)");

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
    unsigned char key;
    unsigned char i;

    while (1) {
        clrscr();
        gotoxy(0, 0);
        cputs("Select ROM bank:");

        for (i = 0; i < 7; i++) {
            gotoxy(2, i + 2);
            if (i == selected) revers(1);
            cprintf("%d. %s", i + 1, romNames[i]);
            revers(0);
        }

        key = cgetc();

        switch (key) {
            case CH_CURS_UP:
                if (selected > 0) selected--;
                break;
            case CH_CURS_DOWN:
                if (selected < 7) selected++;
                break;
            case CH_ENTER:
                return selected;
        }
    }
}

