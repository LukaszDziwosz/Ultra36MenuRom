//   _____  ___________              _______________
//   __  / / /__  /_  /_____________ __|__  /_  ___/
//   _  / / /__  /_  __/_  ___/  __ `/__/_ <_  __ \
//   / /_/ / _  / / /_ _  /   / /_/ /____/ // /_/ /
//   \____/  /_/  \__/ /_/    \__,_/ /____/ \____/
// Ultra-36 Rom Switcher for Commodore 128 - C128 Menu Program - sid_info_screen.c
// Free for personal use.
// Commercial use or resale (in whole or part) prohibited without permission.
// (c) 2025 Lukasz Dziwosz / LukasSoft. All Rights Reserved.

#include <conio.h>
#include <peekpoke.h>
#include <stdio.h>
#include <c128.h>
#include "sid_info_screen.h"

// SID base addresses
#define SID1_BASE       0xD400
#define SID2_MSSIAH     0xDE00
#define SID2_CYNTHCART  0xDF00

// SID type constants
#define SID_NONE        0
#define SID_6581        1
#define SID_8580        2
#define SID_UNKNOWN     3

// Global SID state
unsigned char SID1 = SID_UNKNOWN;
unsigned char SID2 = SID_NONE;
unsigned int SID2_ADDR = 0;

// Reset SID chip
void reset_sid_short(unsigned int base) {
    unsigned char i;
    for (i = 0; i < 25; i++) {
        POKE(base + i, 0x00);
    }
}

// Unified SID detection: presence + model in one routine
// Returns: 0=none, 1=6581, 2=8580, 3=unknown
unsigned char detect_sid_model(unsigned int base) {
    unsigned char result;
    unsigned char i;
    
    // Clear SID first
    for (i = 0; i < 25; i++) {
        POKE(base + i, 0x00);
    }
    
    // Quick presence test - try to write/read volume register
    POKE(base + 0x18, 0x0F);
    if ((PEEK(base + 0x18) & 0x0F) != 0x0F) {
        return SID_NONE;  // No SID chip present
    }
    
    // SID is present, now determine model
    POKE(base + 0x18, 0x00);  // Clear volume
    
    // Model detection sequence
    POKE(base + 0x0E, 0xFF);  // Voice 3 freq low
    POKE(base + 0x0F, 0xFF);  // Voice 3 freq high
    POKE(base + 0x12, 0xFF);  // Test bit + other bits
    POKE(base + 0x12, 0x20);  // Sawtooth, gate off
    
    result = PEEK(base + 0x1B);  // Read OSC3
    
    // Clean up
    for (i = 0; i < 25; i++) {
        POKE(base + i, 0x00);
    }
    
    // Model determination (flipped for real hardware)
    if (result & 0x01) return SID_8580;
    else if (result == 0x00) return SID_UNKNOWN;  // Unclear result
    else return SID_6581;
}

// Play comprehensive filter sweep test
void play_sid_filter_sweep(unsigned int base) {
    unsigned int cutoff, i;
    unsigned char filter_type;
    unsigned char filter_modes[] = {0x10, 0x20, 0x40, 0x50}; // LP, BP, HP, LP+HP (notch)
    const char* filter_names[] = {"Low-pass", "Band-pass", "High-pass", "LP+HP (Notch)"};

    reset_sid_short(base);

    // Set up voice 1 with sawtooth wave
    POKE(base + 0x00, 0x00);  // freq low - low note for better filter demo
    POKE(base + 0x01, 0x08);  // freq hi - lower frequency
    POKE(base + 0x02, 0x00);  // pulse width low
    POKE(base + 0x03, 0x08);  // pulse width high
    POKE(base + 0x05, 0x00);  // attack/decay - instant attack
    POKE(base + 0x06, 0xF0);  // sustain/release - full sustain, fast release

    // Cycle through different filter types
    for (filter_type = 0; filter_type < 4; filter_type++) {
        // Display current filter type
        gotoxy(0, 15);
        cprintf("Filter: %s           ", filter_names[filter_type]);

        // Start the note with sawtooth wave
        POKE(base + 0x04, 0x21);  // gate + sawtooth

        // Set filter mode and enable voice 1 in filter
        POKE(base + 0x17, 0x81);  // High resonance + voice 1 filtered
        POKE(base + 0x18, filter_modes[filter_type] | 0x0F);  // Filter mode + volume

        // Sweep filter cutoff from low to high
        for (cutoff = 0; cutoff < 2048; cutoff += 8) {
            POKE(base + 0x15, cutoff & 0xFF);        // FC LO
            POKE(base + 0x16, (cutoff >> 8) & 0x07); // FC HI (only 3 bits used)

            // Small delay to hear the sweep
            for (i = 0; i < 200; i++) {
                // Delay loop
            }
        }

        // Gate off between filter types
        POKE(base + 0x04, 0x20);  // gate off

        // Pause between filter types
        for (i = 0; i < 10000; i++) {
            // Pause
        }
    }

    // Demo with different resonance levels
    gotoxy(0, 15);
    cputs("Resonance demo...        ");

    // Fixed filter settings for resonance demo
    POKE(base + 0x15, 0x00);  // Low cutoff
    POKE(base + 0x16, 0x02);  //
    POKE(base + 0x18, 0x1F);  // Low-pass filter + volume

    // Sweep through resonance levels
    for (i = 0; i < 16; i++) {
        POKE(base + 0x04, 0x21);  // gate + sawtooth
        POKE(base + 0x17, (i << 4) | 0x01);  // Resonance + voice 1 filtered

        // Hold each resonance level
        for (cutoff = 0; cutoff < 5000; cutoff++) {
            // Hold note
        }

        POKE(base + 0x04, 0x20);  // gate off

        // Brief pause
        for (cutoff = 0; cutoff < 1000; cutoff++) {
            // Pause
        }
    }

    reset_sid_short(base);

    gotoxy(0, 15);
    cputs("Filter demo complete.    ");
}

void draw_sub_title_bar(unsigned char screen_width) {
    unsigned char i;

    gotoxy(0, 1);
    revers(1);
    textcolor(COLOR_LIGHTRED);

    // Fill entire line with spaces
    for (i = 0; i < screen_width; i++) cputc(' ');

    cputsxy(0, 1, "SID Info");
    gotoxy(screen_width - 8, 1);
    cputs("F8: Exit");
    revers(0);
}

void draw_sid_info_screen(unsigned char screen_width) {
    unsigned char i;
    unsigned char key;

    const char* sid_model_name[] = {
        "Not detected",    // SID_NONE
        "MOS 6581",        // SID_6581
        "MOS 8580",        // SID_8580
        "Unknown model"    // SID_UNKNOWN
    };

    const char* sid_comment[] = {
        "",
        "6581! You like noise?",
        "8580! You like clean?",
        "A mystery SID?!"
    };

    // Clear content area
    for (i = 2; i < 25; i++) {
        cclearxy(0, i, screen_width);
    }

    draw_sub_title_bar(screen_width);
    textcolor(COLOR_WHITE);
    cputsxy(0, 3, "SID Chip Detection");

    // Detect SID 1
    SID1 = detect_sid_model(SID1_BASE);
    gotoxy(0, 5);
    cprintf("SID 1: %s", sid_model_name[SID1]);

    // Detect SID 2
    SID2 = detect_sid_model(SID2_MSSIAH);
    if (SID2 >= SID_6581 && SID2 <= SID_UNKNOWN) {
        SID2_ADDR = SID2_MSSIAH;
        gotoxy(0, 6);
        cprintf("SID 2: %s at $DE00", sid_model_name[SID2]);
    } else {
        SID2 = detect_sid_model(SID2_CYNTHCART);
        if (SID2 >= SID_6581 && SID2 <= SID_UNKNOWN) {
            SID2_ADDR = SID2_CYNTHCART;
            gotoxy(0, 6);
            cprintf("SID 2: %s at $DF00", sid_model_name[SID2]);
        } else {
            gotoxy(0, 6);
            cputs("SID 2: Not detected");
        }
    }

    // Show SID commentary
    if (SID1 >= SID_6581 && SID2 == SID_NONE) {
        gotoxy(0, 8);
        cputs(sid_comment[SID1]);
    } else if (SID1 >= SID_6581 && SID2 >= SID_6581 && SID1 == SID2) {
        gotoxy(0, 8);
        cputs(sid_comment[SID1]);
    } else if (SID1 >= SID_6581 && SID2 >= SID_6581 && SID1 != SID2) {
        gotoxy(0, 8);
        cputs("Different SID's? Someone was naughty!");
    }

    // Instructions
    cputsxy(0, 19, "Model detection works best on cold boot");
    cputsxy(0, 20, "Unknown = failed to determine SID type");
    if (SID1 >= SID_6581) cputsxy(0, 10, "Press F1 to play filter sweep on SID 1");
    if (SID2 >= SID_6581) cputsxy(0, 11, "Press F2 to play filter sweep on SID 2");
    cputsxy(0, 23, "$DE00 PIN 7  MSSIAH / PROPHET64 CARTS");
    cputsxy(0, 24, "$DF00 PIN 10 on expansion port CYNTHCART");

    // Key input loop
    while (1) {
        key = cgetc();
        if (key == CH_F1 && SID1 >= SID_6581) {
            cputsxy(0, 13, "Playing SID 1 test...");
            play_sid_filter_sweep(SID1_BASE);
            cclearxy(0, 13, screen_width);
        } else if (key == CH_F2 && SID2 >= SID_6581) {
            cputsxy(0, 14, "Playing SID 2 test...");
            play_sid_filter_sweep(SID2_ADDR);
            cclearxy(0, 14, screen_width);
        } else if (key == CH_F8) {
            break; // return to main screen
        }
    }
}
