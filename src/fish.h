#include <stdint.h>

#ifndef FISH_H_
#define FISH_H_

// System constants
#define MAX_MEMORY 4096
#define STACK_SIZE 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

#define REFRESH_RATE 60
#define DISPLAY_SCALE 20

// Memory locations
#define FONT_START 0x0000
#define FONT_STRIDE 5
#define ROM_START 0x0200

typedef struct {
    // 4Kb of main system memory organised in a byte array.
    uint8_t memory[MAX_MEMORY];

    // Visual display of 64x32 resolution (monochrome).
    uint8_t display[DISPLAY_HEIGHT][DISPLAY_WIDTH];

    // Program counter is a pointer to the current instruction to execute.
    uint16_t pc;

    // "I" register. 16-bit pointer to memory.
    uint16_t i_reg;

    // Stack. Can store up to 16, 16-bit addresses in LIFO fashion.
    uint16_t stack[STACK_SIZE];

    // Stack Pointer. Points to top most level of the stack.
    uint16_t sp;

    // Delay timer. 8-bit timer which decreases at 60Hz until 0.
    uint8_t delay_timer;

    //Sound timer. Same as above but triggers a "beep" sound when the value != 0.
    uint8_t sound_timer;

    // 16 x 8-bit general purpose registers V0-VF.
    // vF specifically can also be used as a "flag" for carry or overflow operations.
    uint8_t v[16];

    // 16 x Keys on a keypad (0-F). Represented as an int (bool) array of ON/OFF.
    uint8_t keypad[16];

    // CPU exectution speed (instructions per second to execute).
    uint16_t frequency;

    // Kill option
    uint8_t exit_requested;

    // Keeps track keypad state on previous frame.
    uint8_t keypad_buffer[16];

    uint8_t draw_requested;
} Fish;

void InitFish(Fish* state);

int InitSDL();
void ClearScreen();

int LoadRom(char* file_name, uint8_t* memory);

#endif // FISH_H