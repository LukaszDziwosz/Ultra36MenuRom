#include <stdio.h>
#include <conio.h>
#include <peekpoke.h>
#include <c128.h>

#define CIA2_PRB 0xDD01
#define CIA2_DDRB 0xDD03
#define SERIAL_DATA_MASK 0x01
#define SERIAL_CLOCK_MASK 0x02

#define CMD_BANK_PREFIX 0xA0
#define CMD_JIFFY_PREFIX 0xB0
#define CMD_PING 0xC0

#define HALF_CYCLE_DELAY 20
#define ACK_TIMEOUT_STEPS 1500

typedef int bool;
#define true 1
#define false 0

static void delay_units(unsigned int count);
static void send_byte(unsigned char value, unsigned char *port_value);
static bool send_command(unsigned char command);
static unsigned char key_to_bank(unsigned char key, bool *valid);
static void print_result(const char *label, unsigned char command, bool ok);

int main(void)
{
    unsigned char key;
    bool ok;
    bool valid;
    unsigned char bank;

    fast();
    clrscr();
    bgcolor(COLOR_BLUE);
    bordercolor(COLOR_BLUE);
    textcolor(COLOR_WHITE);

    cputs("ULTRA36 DEBUG PROTOCOL TEST\r\n");
    cputs("---------------------------\r\n");
    cputs("P = ping only (no output change)\r\n");
    cputs("J = jiffy on, K = jiffy off\r\n");
    cputs("0-9/A-F = select bank\r\n");
    cputs("Q = quit\r\n\r\n");

    while (1) {
        cputs("command> ");
        key = cgetc();
        cputc(key);
        cputs("\r\n");

        if (key == 'q' || key == 'Q') {
            break;
        }

        if (key == 'p' || key == 'P') {
            ok = send_command(CMD_PING);
            print_result("PING", CMD_PING, ok);
        } else if (key == 'j' || key == 'J') {
            ok = send_command(CMD_JIFFY_PREFIX | 1);
            print_result("JIFFY ON", CMD_JIFFY_PREFIX | 1, ok);
        } else if (key == 'k' || key == 'K') {
            ok = send_command(CMD_JIFFY_PREFIX | 0);
            print_result("JIFFY OFF", CMD_JIFFY_PREFIX | 0, ok);
        } else {
            bank = key_to_bank(key, &valid);
            if (valid) {
                ok = send_command(CMD_BANK_PREFIX | bank);
                print_result("BANK", CMD_BANK_PREFIX | bank, ok);
            } else {
                cputs("unknown key\r\n");
            }
        }
    }

    slow();
    return 0;
}

static void delay_units(unsigned int count)
{
    unsigned int i;

    for (i = 0; i < count; i++) {
        __asm__("nop");
    }
}

static void send_byte(unsigned char value, unsigned char *port_value)
{
    unsigned char mask;

    for (mask = 0x80; mask != 0; mask >>= 1) {
        *port_value &= (unsigned char)~SERIAL_CLOCK_MASK;
        POKE(CIA2_PRB, *port_value);

        if (value & mask) {
            *port_value |= SERIAL_DATA_MASK;
        } else {
            *port_value &= (unsigned char)~SERIAL_DATA_MASK;
        }
        POKE(CIA2_PRB, *port_value);
        delay_units(HALF_CYCLE_DELAY);

        *port_value |= SERIAL_CLOCK_MASK;
        POKE(CIA2_PRB, *port_value);
        delay_units(HALF_CYCLE_DELAY);
    }
}

static bool send_command(unsigned char command)
{
    unsigned char saved_port;
    unsigned char saved_ddr;
    unsigned char port_value;
    unsigned int timeout;
    bool acknowledged = false;
    bool data_released = false;

    saved_port = PEEK(CIA2_PRB);
    saved_ddr = PEEK(CIA2_DDRB);

    port_value = saved_port | SERIAL_DATA_MASK | SERIAL_CLOCK_MASK;
    POKE(CIA2_PRB, port_value);
    POKE(CIA2_DDRB, saved_ddr | SERIAL_DATA_MASK | SERIAL_CLOCK_MASK);
    delay_units(HALF_CYCLE_DELAY * 10);

    send_byte(command, &port_value);

    port_value &= (unsigned char)~SERIAL_CLOCK_MASK;
    POKE(CIA2_PRB, port_value);
    POKE(CIA2_DDRB,
         (saved_ddr | SERIAL_CLOCK_MASK) &
         (unsigned char)~SERIAL_DATA_MASK);

    for (timeout = 0; timeout < 300; timeout++) {
        if (PEEK(CIA2_PRB) & SERIAL_DATA_MASK) {
            data_released = true;
            break;
        }
        delay_units(HALF_CYCLE_DELAY);
    }

    if (!data_released) {
        POKE(CIA2_PRB, saved_port);
        POKE(CIA2_DDRB, saved_ddr);
        return false;
    }

    for (timeout = 0; timeout < ACK_TIMEOUT_STEPS; timeout++) {
        if ((PEEK(CIA2_PRB) & SERIAL_DATA_MASK) == 0) {
            acknowledged = true;
            break;
        }
        delay_units(HALF_CYCLE_DELAY);
    }

    if (acknowledged) {
        for (timeout = 0; timeout < ACK_TIMEOUT_STEPS; timeout++) {
            if (PEEK(CIA2_PRB) & SERIAL_DATA_MASK) {
                break;
            }
            delay_units(HALF_CYCLE_DELAY);
        }
    }

    POKE(CIA2_PRB, saved_port);
    POKE(CIA2_DDRB, saved_ddr);

    return acknowledged;
}

static unsigned char key_to_bank(unsigned char key, bool *valid)
{
    *valid = true;

    if (key >= '0' && key <= '9') {
        return key - '0';
    }
    if (key >= 'a' && key <= 'f') {
        return key - 'a' + 10;
    }
    if (key >= 'A' && key <= 'F') {
        return key - 'A' + 10;
    }

    *valid = false;
    return 0;
}

static void print_result(const char *label, unsigned char command, bool ok)
{
    cprintf("%s $%02X: %s\r\n\r\n", label, command, ok ? "ACK" : "FAIL");
}
