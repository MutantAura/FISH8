#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

// System constants
#define MAX_MEMORY 4096
#define STACK_SIZE 16

// Memory locations
#define ROM_START 0x200
#define DISPLAY_START 0xF00

typedef struct {
    // 4Kb of main system memory organised in a byte array.
    uint8_t memory[MAX_MEMORY];

    // Visual display of 64x32 resolution (monochrome). Display is located at 0xF00 of memory.
    // TODO: Replace with "Pixel" type?
    uint8_t* display;

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

    // CPU exectution speed (instructions per second to execute).
    uint8_t frequency;

    // Kill option
    int exit_requested;
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
    state->sp = 0;
    state->exit_requested = 0;
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

void EmulateCpu(Fish* device) {
    uint8_t* current_instr;
    uint8_t instr_nib[4];

    while (!device->exit_requested) {
        // Extract opcode data and break down into nibbles.
        current_instr = &device->memory[device->pc];
        instr_nib[0] = current_instr[0] >> 4; 
        instr_nib[1] = current_instr[0] & 0x0F; 
        instr_nib[2] = current_instr[1] >> 4; 
        instr_nib[3] = current_instr[1] & 0x0F;

        // Opcodes from http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#8xy0
        // Some name changes?
        switch(instr_nib[0]) {
            case 0x00: {
                switch (current_instr[1]) {
                    case 0xE0: printf("%-10s NOT IMPLEMENTED\n", "CLS"); break;  // SDL Clear?
                    case 0xEE: printf("%-10s NOT IMPLEMENTED\n", "RET"); break;
                    default: printf("%-10s $%01x%01x%01x\n", "SYS (NOP)", instr_nib[1], instr_nib[2], instr_nib[3]);
                }
            } break;
            case 0x1: {
                printf("%-10s $%01x%01x%01x\n", "JMP", instr_nib[1], instr_nib[2], instr_nib[3]); 
                // "Jump to location nnn."
                // "The interpreter sets the program counter to nnn."

                device->pc = (instr_nib[1] << 8) | current_instr[1];
                break;
            }
            case 0x2: {
                printf("%-10s $%01x%01x%01x\n", "CALL", instr_nib[1], instr_nib[2], instr_nib[3]); 
                // "Call subroutine at nnn."
                // "The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn."

                device->sp++;
                device->stack[device->sp - 1] = device->pc; // Put on stack THEN increment? Documentation unclear...
                device->pc = (instr_nib[1] << 8) | current_instr[1];
                break;
            }
            case 0x3: {
                printf("%-10s V%01x, #$%02x\n", "SKIP.CMP", instr_nib[1], current_instr[1]); 
                // "Skip next instruction if Vx = kk."
                // "The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2."

                if (device->v[instr_nib[1]] == current_instr[1]) {
                    device->pc += 2;
                }
                break;
            }
            case 0x4: {
                printf("%-10s V%01x, #$%02x\n", "SKIP.NCMP", instr_nib[1], current_instr[1]); 
                // Skip next instruction if Vx != kk.
                // The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.

                if (device->v[instr_nib[1]] != current_instr[1]) {
                    device->pc += 2;
                }
                break;
            } 
            case 0x5: {
                printf("%-10s V%01x, V%01x\n", "SKIP.RCMP", instr_nib[1], instr_nib[2]); 
                // Skip next instruction if Vx = Vy.
                // The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.

                if (device->v[instr_nib[1]] == device->v[instr_nib[2]]) {
                    device->pc += 2;
                }
                break;
            }
            case 0x6: printf("%-10s V%01X,#$%02x NOT IMPLEMENTED\n", "MVI", instr_nib[1], current_instr[1]); break;
            case 0x7: printf("%-10s V%01X,#$%02x NOT IMPLEMENTED\n", "ADD", instr_nib[1], current_instr[1]); break;
            case 0x8: {
                switch (instr_nib[3]) {
                    case 0x0: printf("%-10s V%01x,V%01x NOT IMPLEMENTED\n", "CPY", instr_nib[1], instr_nib[2]); break;
                    case 0x1: printf("%-10s V%01x,V%01x NOT IMPLEMENTED\n", "OR", instr_nib[1], instr_nib[2]); break;
                    case 0x2: printf("%-10s V%01x,V%01x NOT IMPLEMENTED\n", "AND", instr_nib[1], instr_nib[2]); break;
                    case 0x3: printf("%-10s V%01x,V%01x NOT IMPLEMENTED\n", "XOR", instr_nib[1], instr_nib[2]); break;
                    case 0x4: printf("%-10s V%01x,V%01x (VF) NOT IMPLEMENTED\n", "ADD", instr_nib[1], instr_nib[2]); break;
                    case 0x5: printf("%-10s V%01x,V%01x NOT IMPLEMENTED\n", "SUB", instr_nib[1], instr_nib[2]); break;
                    case 0x6: printf("%-10s V%01x,V%01x (VF) NOT IMPLEMENTED\n", "SHR", instr_nib[1], instr_nib[2]); break;
                    case 0x7: printf("%-10s V%01x,V%01x (VF) NOT IMPLEMENTED\n", "SUBN", instr_nib[1], instr_nib[2]); break;
                    case 0xE: printf("%-10s V%01x,V%01x (VF) NOT IMPLEMENTED\n", "SHL", instr_nib[1], instr_nib[2]); break;
                    default: puts("Unknown `8` opcode."); break;
                }
            } break;
            case 0x9: printf("%-10s V%01x, V%01x NOT IMPLEMENTED\n", "SNE", instr_nib[1], instr_nib[2]); break;
            case 0xa: printf("%-10s I,#$%01x%02x NOT IMPLEMENTED\n", "LDI", instr_nib[1], current_instr[1]); break;
            case 0xb: printf("%-10s $%01x%02x + V0 NOT IMPLEMENTED\n", "JMP.V", instr_nib[1], current_instr[1]); break;
            case 0xc: printf("%-10s V%01x, #$%02x NOT IMPLEMENTED\n", "RAND", instr_nib[1], current_instr[1]); break;
            case 0xd: printf("%-10s V%01x, V%01x bytes: %01d NOT IMPLEMENTED\n", "DRW", instr_nib[1], instr_nib[2], (int)instr_nib[3]); break;
            case 0xe: {
                switch (current_instr[1]) {
                    case 0x9E: printf("%-10s V%01x NOT IMPLEMENTED\n", "SKIP.KEYX", instr_nib[1]); break;
                    case 0xA1: printf("%-10s V%01x NOT IMPLEMENTED\n", "SKIPN.KEYX", instr_nib[1]); break;
                    default: puts("Unknown `e` opcode.");
                }
            } break;
            case 0xf: {
                switch (current_instr[1]) {
                    case 0x07: printf("%-10s V%01x, DT NOT IMPLEMENTED\n", "LDX.DT", instr_nib[1]); break;
                    case 0x0A: printf("%-10s V%01x, KEY NOT IMPLEMENTED\n", "LDX.KEY", instr_nib[1]); break;
                    case 0x15: printf("%-10s DT, V%01x NOT IMPLEMENTED\n", "LDDT.X", instr_nib[1]); break;
                    case 0x18: printf("%-10s ST, V%01x NOT IMPLEMENTED\n", "LDST.X", instr_nib[1]); break;
                    case 0x1E: printf("%-10s I, V%01x NOT IMPLEMENTED\n", "ADDI.X", instr_nib[1]); break;
                    case 0x29: printf("%-10s I, Sprite: %01x NOT IMPLEMENTED\n", "LDI.FX", instr_nib[1]); break;
                    case 0x33: printf("%-10s I, (BCD)V%01x NOT IMPLEMENTED\n", "LDB.X", instr_nib[1]); break;
                    case 0x55: printf("%-10s I, V0 -> V%01x NOT IMPLEMENTED\n", "LDI.ALL", instr_nib[1]); break;
                    case 0x65: printf("%-10s V0 -> V%01x, I NOT IMPLEMENTED\n", "LDX.ALL", instr_nib[1]); break;
                    default: puts("Unknown `f` opcode."); break;
                }
            } break;
        }
        
        // Increment PC by 2 after each instruction call.
        device->pc += 2;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        puts("you are stupid.");
        return 1;
    }
    
    // Setup device
    Fish state = {0};
    InitFish(&state);

    // Load ROM file into device memory.
    if (LoadRom(argv[1], &state.memory[0]) != 0) {
        puts("You are also stupid (file error)");
    }

    // Start CPU cycles
    EmulateCpu(&state);

    // Enter SDL loop?

    // Cleanup
    return 0;
}