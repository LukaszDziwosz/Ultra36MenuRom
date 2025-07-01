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

#define SID1_BASE 0xD400
#define SID2_MSSIAH 0xDE00
#define SID2_CYNTHCART 0xDF00

// Reset SID chip
void reset_sid_short(unsigned int base) {
    unsigned char i;
    for (i = 0; i < 25; i++) {
        POKE(base + i, 0x00);
    }
}

// Check if a SID chip is present at the given address
unsigned char is_sid_present(unsigned int base) {
    unsigned char original_val, test_val1, test_val2;
    unsigned char tests_passed = 0;

    // Test 1: Volume register (0x18) - ORIGINAL VERSION
    original_val = PEEK(base + 0x18);
    POKE(base + 0x18, 0x0F);
    test_val1 = PEEK(base + 0x18) & 0x0F;
    POKE(base + 0x18, 0x00);
    test_val2 = PEEK(base + 0x18) & 0x0F;
    POKE(base + 0x18, original_val);
    
    if (test_val1 == 0x0F && test_val2 == 0x00) tests_passed++;

    // Test 2: Attack/Decay register (writable) - ORIGINAL VERSION
    original_val = PEEK(base + 0x05);
    POKE(base + 0x05, 0xAA);
    test_val1 = PEEK(base + 0x05);
    POKE(base + 0x05, 0x55);
    test_val2 = PEEK(base + 0x05);
    POKE(base + 0x05, original_val);
    
    if (test_val1 == 0xAA && test_val2 == 0x55) tests_passed++;

    // Test 3: Consistency check - ORIGINAL VERSION
    test_val1 = PEEK(base + 0x19);  // POT X
    test_val2 = PEEK(base + 0x1A);  // POT Y
    
    if (test_val1 == PEEK(base + 0x19) && test_val2 == PEEK(base + 0x1A)) {
        tests_passed++;
    }

    return (tests_passed >= 2);  // Original requirement
}

// Detect SID model at a given address (returns 0 for none, 1=6581, 2=8580)
unsigned char detect_sid_model(unsigned int base) {
    unsigned char sample, max = 0;
    unsigned int i;
    unsigned char attempts;

    // First check if SID is actually present
    if (!is_sid_present(base)) {
        return 0;
    }

    // Try detection multiple times
    for (attempts = 0; attempts < 5; attempts++) {  // More attempts
        max = 0;

        // Turn off screen for stable reads
        POKE(0xD011, 0x0B);

        // Additional clearing just before test
        for (i = 0; i < 25; i++) {
            POKE(base + i, 0x00);
        }

        // Much longer settling time
        for (i = 0; i < 2000; i++) {
            // Long delay for analog settling
        }

        // Your working oscillator settings
        POKE(base + 0x0E, 0x20);
        POKE(base + 0x0F, 0x20);
        POKE(base + 0x12, 0x31);

        // Even longer oscillator startup time
        for (i = 0; i < 1000; i++) {
            // Long startup delay
        }

        // Sample more extensively
        for (i = 0; i < 512; ++i) {  // More samples
            sample = PEEK(base + 0x1B);
            if (sample > max) max = sample;
        }

        // Stop oscillator
        POKE(base + 0x12, 0x30);

        // Clear registers
        reset_sid_short(base);

        // Restore screen
        POKE(0xD011, 0x1B);

        // If we got a good reading, break early
        if (max > 0x20) break;  // Lower threshold for "good enough"
    }

    // Your original thresholds
    if (max >= 0x80) return 2;      // 8580
    else if (max >= 0x20) return 1; // 6581
    else return 0;
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
    unsigned char sid1;
    unsigned char sid2;
    unsigned int sid2_addr = 0;
    const char* sid_model_name[] = { "Not detected", "MOS 6581", "MOS 8580" };
    const char* sid_comment[] = { "", "6581! You like noise?", "8580! You like clean?" };

    // Clear content area
    for (i = 2; i < 25; i++) {
        cclearxy(0, i, screen_width);
    }

    draw_sub_title_bar(screen_width);
    textcolor(COLOR_WHITE);
    cputsxy(0, 3, "SID Chip Detection");

    // Detect SID 1
    sid1 = detect_sid_model(SID1_BASE);
    gotoxy(0, 5);
    cprintf("SID 1: %s", sid_model_name[sid1]);

    // Detect SID 2 at DE00 first
    sid2 = detect_sid_model(SID2_MSSIAH);
    if (sid2 == 1 || sid2 == 2) {
        sid2_addr = SID2_MSSIAH;
        gotoxy(0, 6);
        cprintf("SID 2: %s at $DE00", sid_model_name[sid2]);
    } else {
        // Try DF00 if DE00 failed
        sid2 = detect_sid_model(SID2_CYNTHCART);
        if (sid2 == 1 || sid2 == 2) {
            sid2_addr = SID2_CYNTHCART;
            gotoxy(0, 6);
            cprintf("SID 2: %s at $DF00", sid_model_name[sid2]);
        } else {
            cputsxy(0, 6, "SID 2: Not detected");
        }
    }

    // Show personality-based comment
    if ((sid1 == 1 || sid1 == 2) && sid2 == 0) {
        gotoxy(0, 8);
        cprintf("%s", sid_comment[sid1]);
    } else if (sid1 && sid2 && sid1 == sid2) {
        gotoxy(0, 8);
        cprintf("%s", sid_comment[sid1]);
    } else if (sid1 && sid2 && sid1 != sid2) {
        cputsxy(0, 8, "Different SID's? Someone was naughty!");
    }

    // Instructions
    cputsxy(0, 19, "Note: Detection only works on cold boot!");
    cputsxy(0, 20, "Power off and hold reset for few seconds");
    if (sid1) cputsxy(0, 10, "Press F1 to play filter sweep on SID 1");
    if (sid2) cputsxy(0, 11, "Press F2 to play filter sweep on SID 2");
    cputsxy(0, 23, "$DE00 PIN 7  MSSIAH / PROPHET64 CARTS");
    cputsxy(0, 24, "$DF00 PIN 10 on expansion port CYNTHCART");

    while (1) {
        key = cgetc();
        if (key == CH_F1 && sid1) {
            cputsxy(0, 13, "Playing SID 1 test...");
            play_sid_filter_sweep(SID1_BASE);
            cclearxy(0, 13, screen_width);
        } else if (key == CH_F2 && sid2) {
            cputsxy(0, 14, "Playing SID 2 test...");
            play_sid_filter_sweep(sid2_addr);
            cclearxy(0, 14, screen_width);
        } else if (key == CH_F8) {
            break; // return to main screen
        }
    }
}
