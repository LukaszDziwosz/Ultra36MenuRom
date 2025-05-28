;
; Startup code for cc65 (C128 32K cartridge version)
;

.export _exit
.export __STARTUP__ : absolute = 1      ; Mark as startup
.import initlib, donelib
.import zerobss
.import callmain, pushax
.import RESTOR, BSOUT, CLRCH
.import __RAM_START__, __RAM_SIZE__     ; Linker generated
.import _cgetc, _puts, _memcpy
.import __DATA_LOAD__, __DATA_RUN__, __DATA_SIZE__

.include "zeropage.inc"
.include "c128.inc"

.segment "STARTUP"

; C128 cartridge header structure
startup:
        jmp coldstart       ; cold-start vector at $8000
        jmp warmstart       ; warm-start vector at $8003
        .byte $01           ; identifier byte at $8006 (autostart immediately)
        .byte $43,$42,$4D   ; "CBM" string at $8007-$8009

coldstart:
warmstart:
        ; Disable interrupts and set up stack
        sei
        ldx #$ff
        txs 
        cld 
        
        ; Set up C128 processor ports (similar to C64 but C128 specific)
        lda #$e3
        sta $01
        lda #$37
        sta $00

        ; Configure MMU for 32K cartridge operation
        ; BIT 0   : $D000-$DFFF (0 = I/O Block)
        ; BIT 1   : $4000-$7FFF (1 = RAM)
        ; BIT 2/3 : $8000-$BFFF (10 = External ROM)
        ; BIT 4/5 : $C000-$CFFF/$E000-$FFFF (10 = External ROM) - 32K mode
        ; BIT 6/7 : RAM used. (00 = RAM 0)
        lda #%00101010     ; Note: bits 4/5 = 10 for External ROM in HIGH area too
        sta $ff00          ; MMU Configuration Register

        ; Initialize C128 system
        jsr $ff8a          ; Restore Vectors
        jsr $ff84          ; Init I/O Devices, Ports & Timers
        jsr $ff81          ; Init Editor & Video Chips

        ; Switch to second charset
        lda #14
        jsr BSOUT

        ; Clear the BSS data
        jsr zerobss

        ; Set up argument stack pointer
        lda    #<(__RAM_START__ + __RAM_SIZE__)
        sta sp
        lda #>(__RAM_START__ + __RAM_SIZE__)
        sta sp+1

        ; Copy initialized data from ROM to RAM
        lda #<__DATA_RUN__
        ldx #>__DATA_RUN__
        jsr pushax
        lda #<__DATA_LOAD__
        ldx #>__DATA_LOAD__
        jsr pushax
        lda #<__DATA_SIZE__
        ldx #>__DATA_SIZE__
        jsr _memcpy

        ; Call module constructors
        jsr initlib

        ; Push arguments and call main
        jsr     callmain

; Back from main (This is also the _exit entry). Run module destructors
_exit:
    jsr donelib

    ; Display exit message
    lda #<exitmsg
    ldx #>exitmsg
    jsr _puts
    jsr _cgetc

    ; Return to BASIC prompt
    jmp $e422

.rodata
exitmsg:
        .byte "Your program has ended.",13
        .byte "Press any key to continue...",0

; Pad to end of LOW ROM (16K block)
.segment "PADLO"
        .byte 0

; Optional high ROM code area
.segment "HICODE"
; You can place additional code here that will be loaded in $C000-$FEFF area
; Example:
high_rom_function:
        ; Your high ROM code here
        rts

; Pad to end of HIGH ROM 
.segment "PADHI"
        .byte 0
