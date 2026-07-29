//   _____  ___________              _______________
//   __  / / /__  /_  /_____________ __|__  /_  ___/
//   _  / / /__  /_  __/_  ___/  __ `/__/_ <_  __ \
//   / /_/ / _  / / /_ _  /   / /_/ /____/ // /_/ /
//   \____/  /_/  \__/ /_/    \__,_/ /____/ \____/
// Ultra-36 Rom Switcher for Commodore 128 - C128 Menu Program
// Free for personal use.
// Commercial use or resale (in whole or part) prohibited without permission.
// (c) 2025 Lukasz Dziwosz / LukasSoft. All Rights Reserved.

#include <conio.h>
#include <peekpoke.h>
#include <stdio.h>
#include <c128.h>
#include "sid_info_screen.h"

#define SID1_BASE       0xD400

#define SID_6581        1
#define SID_8580        2
#define SID_UNKNOWN     3

#define SID_FREQ_LO     0x00
#define SID_FREQ_HI     0x01
#define SID_PW_LO       0x02
#define SID_PW_HI       0x03
#define SID_CONTROL     0x04
#define SID_ATTACK_DECAY 0x05
#define SID_SUSTAIN_RELEASE 0x06
#define SID_FILTER_LO   0x15
#define SID_FILTER_HI   0x16
#define SID_RESONANCE   0x17
#define SID_MODE_VOLUME 0x18
#define SID_OSC3        0x1B

#define SID2_ADDRESS_COUNT 4
#define SID2_FIRST_ROW     9

static const unsigned int sid2_addresses[SID2_ADDRESS_COUNT] = {
    0xD420, 0xD700, 0xDE00, 0xDF00
};

static const char* sid2_labels[SID2_ADDRESS_COUNT] = {
    "$D420  Stereo / SIDKick",
    "$D700  C128 internal",
    "$DE00  IO1 / MSSIAH",
    "$DF00  IO2 / Cynthcart"
};

static const unsigned char voice_offsets[3] = {0x00, 0x07, 0x0E};

/* Four chords used by the audible SID check: C, F, G and A minor. */
static const unsigned int voice1_notes[4] = {
    0x08B4, 0x0BA4, 0x0D0C, 0x0EA7
};
static const unsigned int voice2_notes[4] = {
    0x0AF8, 0x0EA7, 0x0F83, 0x1168
};
static const unsigned int voice3_notes[4] = {
    0x0D0C, 0x1168, 0x138A, 0x15F0
};

static unsigned char sid2_selected = 1;

static void reset_sid(unsigned int base)
{
    unsigned char i;

    for (i = 0; i < 25; ++i)
        POKE(base + i, 0x00);
}

/*
 * Wait for the VIC raster to reach line 240. This keeps the test tempo stable
 * in both 1 MHz VIC mode and 2 MHz VDC mode without depending on CPU loops.
 */
static void wait_sid_frame(void)
{
    while (PEEK(0xD012) == 0xF0) {}
    while (PEEK(0xD012) != 0xF0) {}
}

static void wait_sid_frames(unsigned char count)
{
    while (count != 0) {
        wait_sid_frame();
        --count;
    }
}

static void set_sid_frequency(unsigned int base, unsigned char voice,
                              unsigned int frequency)
{
    unsigned char offset = voice_offsets[voice];

    POKE(base + offset + SID_FREQ_LO, (unsigned char)frequency);
    POKE(base + offset + SID_FREQ_HI, (unsigned char)(frequency >> 8));
}

static void set_sid_cutoff(unsigned int base, unsigned int cutoff)
{
    /*
     * SID cutoff is an 11-bit value. Register $15 contains only bits 0-2;
     * register $16 contains bits 3-10.
     */
    POKE(base + SID_FILTER_LO, (unsigned char)(cutoff & 0x07));
    POKE(base + SID_FILTER_HI, (unsigned char)(cutoff >> 3));
}

static void set_sid_chord(unsigned int base, unsigned char chord)
{
    set_sid_frequency(base, 0, voice1_notes[chord]);
    set_sid_frequency(base, 1, voice2_notes[chord]);
    set_sid_frequency(base, 2, voice3_notes[chord]);
}

/*
 * Play a short three-voice musical phrase through a strongly resonant
 * low-pass filter. Hearing it from the expected output is the SID 2 test;
 * no unreliable register-read address scan is attempted.
 */
static void play_sid_sound_check(unsigned int base)
{
    unsigned int cutoff;
    unsigned char chord;
    unsigned char sweep_step;
    unsigned char volume;

    reset_sid(base);

    set_sid_chord(base, 0);

    /* Voice 1: saw, voice 2: pulse, voice 3: triangle. */
    POKE(base + 0x00 + SID_ATTACK_DECAY, 0x24);
    POKE(base + 0x00 + SID_SUSTAIN_RELEASE, 0xA8);
    POKE(base + 0x07 + SID_PW_LO, 0x00);
    POKE(base + 0x07 + SID_PW_HI, 0x08);
    POKE(base + 0x07 + SID_ATTACK_DECAY, 0x34);
    POKE(base + 0x07 + SID_SUSTAIN_RELEASE, 0x98);
    POKE(base + 0x0E + SID_ATTACK_DECAY, 0x14);
    POKE(base + 0x0E + SID_SUSTAIN_RELEASE, 0xA8);

    set_sid_cutoff(base, 0x0080);
    POKE(base + SID_RESONANCE, 0xD7);   /* resonance 13, all voices */
    POKE(base + SID_MODE_VOLUME, 0x1F); /* low-pass, full volume */

    POKE(base + 0x00 + SID_CONTROL, 0x21);
    POKE(base + 0x07 + SID_CONTROL, 0x41);
    POKE(base + 0x0E + SID_CONTROL, 0x11);

    /*
     * Open the filter while moving through the chord progression. One cutoff
     * step per video frame produces a smooth, repeatable sweep.
     */
    cutoff = 0x0080;
    chord = 0;
    sweep_step = 0;
    while (cutoff < 0x0780) {
        set_sid_cutoff(base, cutoff);
        wait_sid_frame();
        cutoff += 0x18;
        ++sweep_step;
        if (sweep_step == 24) {
            sweep_step = 0;
            chord = (unsigned char)((chord + 1) & 0x03);
            set_sid_chord(base, chord);
        }
    }

    /* Close the filter with maximum resonance for a clear second pass. */
    POKE(base + SID_RESONANCE, 0xF7);
    while (cutoff > 0x0120) {
        cutoff -= 0x20;
        set_sid_cutoff(base, cutoff);
        wait_sid_frame();
    }

    POKE(base + 0x00 + SID_CONTROL, 0x20);
    POKE(base + 0x07 + SID_CONTROL, 0x40);
    POKE(base + 0x0E + SID_CONTROL, 0x10);
    wait_sid_frames(10);

    for (volume = 15; volume != 0; --volume) {
        POKE(base + SID_MODE_VOLUME, (unsigned char)(0x10 | volume));
        wait_sid_frame();
    }
    reset_sid(base);
}

/*
 * SID 1 model detection is intentionally retained only at the fixed $D400
 * address. The audible check is used for every selectable SID 2 address.
 */
static unsigned char detect_sid1_model(void)
{
    unsigned char result;

    reset_sid(SID1_BASE);
    POKE(SID1_BASE + SID_MODE_VOLUME, 0x0F);
    result = PEEK(SID1_BASE + SID_OSC3);
    POKE(SID1_BASE + SID_MODE_VOLUME, 0x00);
    POKE(SID1_BASE + 0x0E, 0xFF);
    POKE(SID1_BASE + 0x0F, 0xFF);
    POKE(SID1_BASE + 0x12, 0xFF);
    POKE(SID1_BASE + 0x12, 0x20);
    result = PEEK(SID1_BASE + SID_OSC3);
    reset_sid(SID1_BASE);

    /*
     * This test normally returns 3 on a 6581 and 2 on an 8580. Bit 0 is
     * therefore set for a 6581 and clear for an 8580.
     */
    if (result & 0x01)
        return SID_6581;
    if (result == 0x00)
        return SID_UNKNOWN;
    return SID_8580;
}

static void draw_sub_title_bar(unsigned char screen_width)
{
    unsigned char i;

    gotoxy(0, 1);
    revers(1);
    textcolor(COLOR_LIGHTRED);
    for (i = 0; i < screen_width; ++i)
        cputc(' ');
    cputsxy(0, 1, "SID Setup");
    gotoxy(screen_width - 8, 1);
    cputs("F8: Exit");
    revers(0);
}

static void draw_sid2_options(unsigned char selected)
{
    unsigned char i;

    for (i = 0; i < SID2_ADDRESS_COUNT; ++i) {
        gotoxy(0, (unsigned char)(SID2_FIRST_ROW + i));
        revers(i == selected);
        textcolor(i == selected ? COLOR_YELLOW : COLOR_WHITE);
        cprintf("%c %s", i == selected ? '>' : ' ', sid2_labels[i]);
        cclear(10);
        revers(0);
    }
    textcolor(COLOR_WHITE);
}

static void show_test_status(unsigned char screen_width, const char* label,
                             unsigned int address)
{
    cclearxy(0, 17, screen_width);
    gotoxy(0, 17);
    textcolor(COLOR_LIGHTGREEN);
    cprintf("Playing %s at $%04X...", label, address);
    textcolor(COLOR_WHITE);
}

void draw_sid_info_screen(unsigned char screen_width)
{
    static const char* sid_model_name[] = {
        "", "MOS 6581", "MOS 8580", "Unknown model"
    };
    unsigned char sid1;
    unsigned char key;
    unsigned char i;
    unsigned int sid2_address;

    for (i = 2; i < 25; ++i)
        cclearxy(0, i, screen_width);

    draw_sub_title_bar(screen_width);
    textcolor(COLOR_WHITE);
    cputsxy(0, 3, "SID 1 model detection ($D400)");

    sid1 = detect_sid1_model();
    gotoxy(0, 5);
    cprintf("SID 1: %s", sid_model_name[sid1]);

    cputsxy(0, 7, "Select the second SID address:");
    draw_sid2_options(sid2_selected);

    cputsxy(0, 14, "UP/DOWN  Select SID 2 address");
    cputsxy(0, 15, "F1       Sound check SID 1");
    cputsxy(0, 16, "F2/RETURN Sound check selected SID 2");
    cputsxy(0, 19, "Listen at each output to confirm SID 2.");
    cputsxy(0, 20, "No SID 2 address scan or model guess.");
    cputsxy(0, 22, "C128: $D500 is MMU, $D600 is VDC.");

    while (1) {
        key = cgetc();

        if (key == CH_CURS_UP) {
            if (sid2_selected == 0)
                sid2_selected = SID2_ADDRESS_COUNT - 1;
            else
                --sid2_selected;
            draw_sid2_options(sid2_selected);
        } else if (key == CH_CURS_DOWN) {
            ++sid2_selected;
            if (sid2_selected == SID2_ADDRESS_COUNT)
                sid2_selected = 0;
            draw_sid2_options(sid2_selected);
        } else if (key == CH_F1) {
            show_test_status(screen_width, "SID 1", SID1_BASE);
            play_sid_sound_check(SID1_BASE);
            cclearxy(0, 17, screen_width);
            cputsxy(0, 17, "SID 1 sound check complete.");
        } else if (key == CH_F2 || key == CH_ENTER) {
            sid2_address = sid2_addresses[sid2_selected];
            show_test_status(screen_width, "SID 2", sid2_address);
            play_sid_sound_check(sid2_address);
            cclearxy(0, 17, screen_width);
            gotoxy(0, 17);
            cprintf("SID 2 $%04X check complete.", sid2_address);
        } else if (key == CH_F8) {
            break;
        }
    }
}
