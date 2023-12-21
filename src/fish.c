#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define MAX_MEMORY 4096
#define ROM_START 0x200
#define DISPLAY_START 0xF00

typedef struct {
    // 4Kb of main system memory organised in a byte array.
    uint8_t memory[MAX_MEMORY];

    // Visual display of 64x32 resolution (monochrome).
    // TODO: Replace with "Pixel" type?
    uint8_t* display;
    // Program counter is a pointer to the current instruction to execute.
    uint16_t pc;

    // "I" register. 16-bit pointer to memory.
    uint16_t i_reg;

    // Stack. Can store up to 16, 16-bit addresses in LIFO fashion.
    uint16_t stack[16];

    // Stack Pointer. Points to top most level of the stack.
    uint16_t sp;

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

void InitFish(Fish* state) {
    state->display = &state->memory[DISPLAY_START];
    state->pc = ROM_START;
}

int LoadRom(char* file_name, uint8_t* memory) {
    FILE* rom = fopen(file_name, "rb");
    if (rom == NULL) {
        return 1;
    }

    fseek(rom, 0L, SEEK_END);
    int file_size = ftell(rom);
    fseek(rom, 0L, SEEK_SET);

    fread(&memory[ROM_START], file_size, 1, rom);
    fclose(rom);

    return 0;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        puts("you are stupid.");
        return 1;
    }
    
    // Setup device
    Fish state = {0};
    InitFish(&state);

    // Load ROM file into state memory.
    if (LoadRom(argv[1], &state.memory) != 0) {
        puts("You are also stupid (file error)");
    }
    // Enter SDL loop

    // Cleanup
    return 0;
}