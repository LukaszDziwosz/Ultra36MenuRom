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

// Returns 1 if SID is likely present, 0 if absent or nosound
unsigned char sid_sound_present(unsigned int base) {
    unsigned char orig, test1, test2;

    // Write known value
    POKE(base + 0x18, 0x0F);
    test1 = PEEK(base + 0x18) & 0x0F;

    // Write another value
    POKE(base + 0x18, 0x00);
    test2 = PEEK(base + 0x18) & 0x0F;

    // Restore (safety)
    POKE(base + 0x18, 0x00);

    // Check if the chip responded to the changes
    if ((test1 == 0x0F) && (test2 == 0x00)) {
        return 1; // Seems like SID
    }

    return 0; // Not a SID - not writable or floating
}

// Unified SID detection: presence + model in one routine
// Returns: 0=none, 1=6581, 2=8580, 3=unknown
unsigned char detect_sid_model(unsigned int base) {
    unsigned char result;
    
    // Clear SID first
    reset_sid_short(base);
    
    // Skip fragile read test for SID1 – assume present unless total failure
    POKE(base + 0x18, 0x0F);
    // delay slightly to let bus settle
    PEEK(base + 0x1B);
    
    // SID is present, now determine model
    POKE(base + 0x18, 0x00);  // Clear volume
    
    // Model detection sequence
    POKE(base + 0x0E, 0xFF);  // Voice 3 freq low
    POKE(base + 0x0F, 0xFF);  // Voice 3 freq high
    POKE(base + 0x12, 0xFF);  // Test bit + other bits
    POKE(base + 0x12, 0x20);  // Sawtooth, gate off
    
    result = PEEK(base + 0x1B);  // Read OSC3
    
    // Clean up
    reset_sid_short(base);
    
    // Model determination (flipped for real hardware)
    if (result & 0x01) return SID_8580;
    else if (result == 0x00) return SID_UNKNOWN;  // Unclear result
    else return SID_6581;
}

void play_sid_filter_sweep(unsigned int base) {
    const unsigned char voice_base[] = {0x00, 0x07, 0x0E}; // voice 1/2/3
    const char* voice_names[] = {"Voice 1", "Voice 2", "Voice 3"};
    const unsigned char voice_mask[] = {0x01, 0x02, 0x04}; // filter voice enable bits
    const unsigned char filter_modes[] = {0x10, 0x20, 0x40}; // LP, BP, HP
    const char* filter_names[] = {"Low-pass", "Band-pass", "High-pass"};

    unsigned char v, f, res;
    unsigned int cutoff, i;

    for (v = 0; v < 3; v++) {
        gotoxy(0, 15);
        cprintf("Testing %s...", voice_names[v]);

        // Set basic voice parameters
        POKE(base + voice_base[v] + 0, 0x00);  // freq lo
        POKE(base + voice_base[v] + 1, 0x08);  // freq hi
        POKE(base + voice_base[v] + 2, 0x00);  // PW lo
        POKE(base + voice_base[v] + 3, 0x08);  // PW hi
        POKE(base + voice_base[v] + 5, 0x00);  // AD
        POKE(base + voice_base[v] + 6, 0xF0);  // SR

        for (f = 0; f < 3; f++) {
            gotoxy(0, 16);
            cprintf("Filter: %s", filter_names[f]);
            
            // Prepare filter mode + full volume
            POKE(base + 0x18, filter_modes[f] | 0x0F);  // Start at half volume
            
            // Start note (sawtooth + gate)
            POKE(base + voice_base[v] + 4, 0x21);
            
            // Let the note play for a bit before sweeping
           // for (i = 0; i < 5000; i++) {}
            
            // Resonance sweep during cutoff sweep
            for (res = 0; res < 4; res++) {
                POKE(base + 0x17, (res << 4) | voice_mask[v]);  // Set resonance + voice
                
                // Sweep cutoff slowly with more steps
                for (cutoff = 0; cutoff < 2048; cutoff += 8) {  // Smaller step = slower sweep
                    POKE(base + 0x15, cutoff & 0xFF);
                    POKE(base + 0x16, (cutoff >> 8) & 0x07);
                    for (i = 0; i < 100; i++) {}  // Smaller delay per step
                }
            }
            
            // Let the note ring for a bit
            for (i = 0; i < 5000; i++) {}
            
            // Gate off smoothly
            POKE(base + voice_base[v] + 4, 0x20);  // gate off
            for (i = 0; i < 5000; i++) {}  // Let release complete
            
            // Mute and clear filter
            POKE(base + 0x17, 0x00);  // no filter
            POKE(base + 0x18, 0x00);  // mute
            
            for (i = 0; i < 10000; i++) {}  // pause between filter types
        }
        // clear between voices
        for (i = 0; i < 20000; i++) {}
    }
    
    gotoxy(0, 17);
    cputs("Filter + resonance test complete.");
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

    // Detect SID 2 - only check if sound responds
    if (sid_sound_present(SID2_MSSIAH)) {
        SID2 = detect_sid_model(SID2_MSSIAH);
        if (SID2 >= SID_6581 && SID2 <= SID_UNKNOWN) {
            SID2_ADDR = SID2_MSSIAH;
            gotoxy(0, 6);
            cprintf("SID 2: %s at $DE00", sid_model_name[SID2]);
        }
    } else if (sid_sound_present(SID2_CYNTHCART)) {
        SID2 = detect_sid_model(SID2_CYNTHCART);
        if (SID2 >= SID_6581 && SID2 <= SID_UNKNOWN) {
            SID2_ADDR = SID2_CYNTHCART;
            gotoxy(0, 6);
            cprintf("SID 2: %s at $DF00", sid_model_name[SID2]);
        }
    } else {
        SID2 = SID_NONE;
        gotoxy(0, 6);
        cputs("SID 2: Not detected");
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
    cputsxy(0, 19, "SID detection works only for real SIDS");
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
            cclearxy(0, 17, screen_width);
            play_sid_filter_sweep(SID1_BASE);
            cclearxy(0, 13, screen_width);
            cclearxy(0, 15, screen_width);
            cclearxy(0, 16, screen_width);
        } else if (key == CH_F2 && SID2 >= SID_6581) {
            cclearxy(0, 17, screen_width);
            cputsxy(0, 14, "Playing SID 2 test...");
            play_sid_filter_sweep(SID2_ADDR);
            cclearxy(0, 14, screen_width);
            cclearxy(0, 15, screen_width);
            cclearxy(0, 16, screen_width);
        } else if (key == CH_F8) {
            break; // return to main screen
        }
    }
}
