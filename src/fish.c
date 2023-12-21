#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

typedef struct {
    // 4Kb of main system memory organised in a byte array.
    uint8_t memory[4096];

    // Visual display of 64x32 resolution (monochrome).
    // TODO: Replace with "Pixel" type?
    uint8_t display[64][32];

    // Program counter is a pointer to the current instruction to execute.
    uint8_t pc;

    // "I" register. 16-bit pointer to memory.
    uint16_t i_reg;

    // Stack. Can store up to 8, 16-bit addresses in LIFO fashion.
    uint16_t stack[8];

    // Delay timer. 8-bit timer which decreases at 60Hz until 0.
    uint8_t delay_timer;

    //Sound timer. Same as above but triggers a "beep" sound when the value != 0.
    uint8_t sound_timer;

    // 16 x 8-bit general purpose registers V0-VF.
    // vF specifically can also be used as a "flag" for carry or overflow operations.
    uint8_t v0;
    uint8_t v1;
    uint8_t v2;
    uint8_t v3;
    uint8_t v4;
    uint8_t v5;
    uint8_t v6;
    uint8_t v7;
    uint8_t v8;
    uint8_t v9;
    uint8_t vA;
    uint8_t vB;
    uint8_t vC;
    uint8_t vD;
    uint8_t vE;
    uint8_t vF;

    // CPU exectution speed (instructions per second to execute).
    uint8_t frequency;
} Fish;

uint8_t font_array[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

int main(void)
{
    printf("Hello, World.\n");
    return 0;

    // ROM DI-ASM

    // Setup device

    // Init device state

    // Enter SDL loop

    // Cleanup
}