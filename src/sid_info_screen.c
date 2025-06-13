#include <conio.h>
#include <peekpoke.h>
#include <stdio.h>
#include <c128.h>
#include "sid_info_screen.h"

#define SID1_BASE 0xD400
#define SID2_MSSIAH 0xDE00
#define SID2_CYNTHCART 0xDF00

// Detect SID model at a given address (returns 0 for none, 1=6581, 2=8580)
unsigned char detect_sid_model(unsigned int base) {
    unsigned char sample, max = 0;
    unsigned int i;

    // Optional: Turn off screen for more stable reads
    POKE(0xD011, 0x0B);  // disable badlines (blank screen)

    // Init oscillator 3 frequency (2020h)
    POKE(base + 0x0E, 0x20);
    POKE(base + 0x0F, 0x20);

    // Enable triangle + sawtooth waveform, gate ON
    POKE(base + 0x12, 0x31);  // 0b00110001

    // Sample $1B (OSC3 output) 256 times
    for (i = 0; i < 256; ++i) {
        sample = PEEK(base + 0x1B);
        if (sample > max) max = sample;
    }

    // Stop oscillator
    POKE(base + 0x12, 0x30);  // 0b00110000

    // Restore screen
    POKE(0xD011, 0x1B);  // normal screen on

    // Return SID type based on threshold
    if (max >= 0x80) return 2; // 8580
    else return 1;             // 6581
}

// Play basic filter sweep test
void play_sid_filter_sweep(unsigned int base) {
    POKE(base + 0x00, 0x00);  // freq low
    POKE(base + 0x01, 0x10);  // freq hi ~ Middle C
    POKE(base + 0x04, 0x11);  // gate + triangle (safe waveform)
    POKE(base + 0x05, 0xF8);  // fast env
    POKE(base + 0x18, 0x0F);  // volume
}

void draw_sid_info_screen(unsigned char screen_width) {
    unsigned char i;
    unsigned char key;
    unsigned char sid1;
    unsigned char sid2;
    unsigned int sid2_addr = 0;

    // Clear content area
    for (i = 3; i < 22; i++) {
        cclearxy(0, i, screen_width);
    }

    textcolor(COLOR_WHITE);
    cputsxy(0, 3, "SID Chip Detection");

    sid1 = detect_sid_model(SID1_BASE);
    if (sid1 == 1) cputsxy(0, 5, "SID 1: MOS 6581");
    else if (sid1 == 2) cputsxy(0, 5, "SID 1: MOS 8580");
    else cputsxy(0, 5, "SID 1: Not detected");

    sid2 = detect_sid_model(SID2_MSSIAH);
    if (sid2 == 1 || sid2 == 2) {
        sid2_addr = SID2_MSSIAH;
        cputsxy(0, 6, "SID 2: Detected at $DE00");
    } else {
        sid2 = detect_sid_model(SID2_CYNTHCART);
        if (sid2 == 1 || sid2 == 2) {
            sid2_addr = SID2_CYNTHCART;
            cputsxy(0, 6, "SID 2: Detected at $DF00");
        } else {
            cputsxy(0, 6, "SID 2: Not detected");
        }
    }

    cputsxy(0, 8, "Press RETURN to play filter sweep on SID 1");
    if (sid2) cputsxy(0, 9, "Press F8 to play filter sweep on SID 2");

    cputsxy(0, 21, "Press R to re-enable function keys");
    while (1) {
        key = cgetc();
        if (key == CH_ENTER) {
            cputsxy(0, 11, "Playing SID 1 test...");
            play_sid_filter_sweep(SID1_BASE);
        } else if (key == CH_F8 && sid2) {
            cputsxy(0, 12, "Playing SID 2 test...");
            play_sid_filter_sweep(sid2_addr);
        } else if (key == 'R') {
            break; // allow returning via function keys
        }
    }
}
