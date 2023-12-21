#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

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

    srand(time(NULL));

    while (!device->exit_requested) {
        // Extract opcode data and break down into nibbles.
        current_instr = &device->memory[device->pc];
        instr_nib[0] = current_instr[0] >> 4; 
        instr_nib[1] = current_instr[0] & 0x0F; 
        instr_nib[2] = current_instr[1] >> 4; 
        instr_nib[3] = current_instr[1] & 0x0F;

        printf("%04x %02x %02x ", device->pc, current_instr[0], current_instr[1]);

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
                device->pc = ((uint16_t)instr_nib[1] << 8) | current_instr[1];
            } break;
            case 0x2: {
                printf("%-10s $%01x%01x%01x\n", "CALL", instr_nib[1], instr_nib[2], instr_nib[3]); 

                // "Call subroutine at nnn."
                // "The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn."
                device->sp++;
                device->stack[device->sp - 1] = device->pc; // Put on stack THEN increment? Documentation unclear...
                device->pc = ((uint16_t)instr_nib[1] << 8) | current_instr[1];
            } break;
            case 0x3: {
                printf("%-10s V%01x, #$%02x\n", "SKIP.CMP", instr_nib[1], current_instr[1]); 

                // "Skip next instruction if Vx = kk."
                // "The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2."
                if (device->v[instr_nib[1]] == current_instr[1]) {
                    device->pc += 2;
                }
  
            } break;
            case 0x4: {
                printf("%-10s V%01x, #$%02x\n", "SKIP.NCMP", instr_nib[1], current_instr[1]); 

                // Skip next instruction if Vx != kk.
                // The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
                if (device->v[instr_nib[1]] != current_instr[1]) {
                    device->pc += 2;
                } 
            } break;
            case 0x5: {
                printf("%-10s V%01x, V%01x\n", "SKIP.RCMP", instr_nib[1], instr_nib[2]); 

                // Skip next instruction if Vx = Vy.
                // The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
                if (device->v[instr_nib[1]] == device->v[instr_nib[2]]) {
                    device->pc += 2;
                }
                
            } break;
            case 0x6: {
                printf("%-10s V%01X,#$%02x\n", "MVI", instr_nib[1], current_instr[1]);

                // Set Vx = kk.
                // The interpreter puts the value kk into register Vx.
                device->v[instr_nib[1]] = current_instr[1];
            } break;
            case 0x7: {
                printf("%-10s V%01X,#$%02x\n", "ADD", instr_nib[1], current_instr[1]);

                // Set Vx = Vx + kk.
                // Adds the value kk to the value of register Vx, then stores the result in Vx. 
                device->v[instr_nib[1]] += current_instr[1];
            } break;
            case 0x8: {
                switch (instr_nib[3]) {
                    case 0x0: {
                        printf("%-10s V%01x,V%01x\n", "MOV", instr_nib[1], instr_nib[2]);

                        // Set Vx = Vy.
                        // Stores the value of register Vy in register Vx.
                        device->v[instr_nib[1]] = device->v[instr_nib[2]];
                    } break;
                    case 0x1: {
                        printf("%-10s V%01x,V%01x\n", "OR", instr_nib[1], instr_nib[2]);

                        // Set Vx = Vx OR Vy.
                        // Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. 
                        // A bitwise OR compares the corrseponding bits from two values, and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0.
                        device->v[instr_nib[1]] |= device->v[instr_nib[2]]; 
                    } break; 
                    case 0x2: {
                        printf("%-10s V%01x,V%01x\n", "AND", instr_nib[1], instr_nib[2]);

                        // Set Vx = Vx AND Vy.
                        // Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx. 
                        // A bitwise AND compares the corrseponding bits from two values, and if both bits are 1, then the same bit in the result is also 1. Otherwise, it is 0.
                        device->v[instr_nib[1]] &= device->v[instr_nib[2]];
                    } break;
                    case 0x3: {
                        printf("%-10s V%01x,V%01x\n", "XOR", instr_nib[1], instr_nib[2]);

                        // Set Vx = Vx XOR Vy.
                        // Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx. 
                        // An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same, then the corresponding bit in the result is set to 1. Otherwise, it is 0. 
                        device->v[instr_nib[1]] ^= device->v[instr_nib[2]];
                    } break; 
                    case 0x4: {
                        printf("%-10s V%01x,V%01x (VF)\n", "ADD", instr_nib[1], instr_nib[2]);

                        // Set Vx = Vx + Vy, set VF = carry.
                        //The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. 
                        // Only the lowest 8 bits of the result are kept, and stored in Vx.
                        uint16_t overflow = device->v[instr_nib[1]] + device->v[instr_nib[2]];
                        if (overflow > UINT8_MAX) {
                            device->v[0xF] = 1;
                        }
                        else device->v[0xF] = 0;
                        
                        device->v[instr_nib[1]] = (uint8_t)(overflow & 0x00FF);
                    } break;
                    case 0x5: {
                        printf("%-10s V%01x,V%01x\n", "SUB", instr_nib[1], instr_nib[2]);

                        // Set Vx = Vx - Vy, set VF = NOT borrow.
                        // If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
                        if (device->v[instr_nib[1]] > device->v[instr_nib[2]]) {
                            device->v[0xF] = 1;
                        } else device->v[0xF] = 0;

                        device->v[instr_nib[1]] -= device->v[instr_nib[2]];
                    } break;
                    case 0x6: {
                        printf("%-10s V%01x,V%01x (VF)\n", "SHR", instr_nib[1], instr_nib[2]);

                        // Set Vx = Vx SHR 1.
                        // If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
                        if ((device->v[instr_nib[1]] & 0x01) == 1) {
                            device->v[0xF] = 1;
                        } else device->v[0xF] = 0;

                        device->v[instr_nib[1]] >>= 1;
                    } break;
                    case 0x7: {
                        printf("%-10s V%01x,V%01x (VF)\n", "SUBN", instr_nib[1], instr_nib[2]);

                        // Set Vx = Vy - Vx, set VF = NOT borrow.
                        // If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
                        if (device->v[instr_nib[2]] > device->v[instr_nib[1]]) {
                            device->v[0xF] = 1;
                        } else device ->v[0xF] = 0;

                        device->v[instr_nib[1]] = device->v[instr_nib[2]] - device->v[instr_nib[1]];
                    } break;
                    case 0xE: {
                        printf("%-10s V%01x,V%01x (VF)\n", "SHL", instr_nib[1], instr_nib[2]);
                        
                        // Set Vx = Vx SHL 1.
                        // If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
                        if (((device->v[instr_nib[1]] & 0x80) >> 7) == 1) {
                            device->v[0xF] = 1;
                        } else device->v[0xF] = 0;

                        device->v[instr_nib[1]] <<= 1;
                    } break;
                    default: puts("Unknown `8` opcode."); break;
                }
            } break;
            case 0x9: {
                printf("%-10s V%01x, V%01x\n", "SNE", instr_nib[1], instr_nib[2]);

                // Skip next instruction if Vx != Vy.
                // The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
                if (device->v[instr_nib[1]] != device->v[instr_nib[2]]) {
                    device->pc += 2;
                }
            } break;
            case 0xa: {
                printf("%-10s I,#$%01x%02x\n", "LDI", instr_nib[1], current_instr[1]);

                // Set I = nnn.
                // The value of register I is set to nnn.
                device->i_reg = ((uint16_t)instr_nib[1] << 8) | current_instr[1];
            } break;
            case 0xb: {
                printf("%-10s $%01x%02x + V0\n", "JMP.V", instr_nib[1], current_instr[1]);

                // Jump to location nnn + V0.
                // The program counter is set to nnn plus the value of V0.
                device->pc = (((uint16_t)instr_nib[1] << 8) | current_instr[1]) + device->v[0];
            } break; 
            case 0xc: {
                printf("%-10s V%01x, #$%02x\n", "RAND", instr_nib[1], current_instr[1]);

                // Set Vx = random byte AND kk.
                // The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk. The results are stored in Vx.
                uint8_t random = (rand() % UINT8_MAX) & current_instr[1];
                device->v[instr_nib[1]] = random;
            } break;
            case 0xd: {
                printf("%-10s V%01x, V%01x bytes: %01d STUB\n", "DRW", instr_nib[1], instr_nib[2], (int)instr_nib[3]);

                // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
                // The interpreter reads n bytes from memory, starting at the address stored in I. 
                // These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the existing screen. 
                // If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. 
                // If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen. 
                uint8_t* sprite_buffer = malloc(instr_nib[3]);
                if (sprite_buffer == NULL) {
                    puts("Failed to allocate buffer...");
                    break;
                }

                // i_reg can get massive for some reason and cause a segfault.
                // TODO: Debug this.
                // memcpy(sprite_buffer, &device->memory[device->i_reg], instr_nib[3]);

                // TODO: Draw function?

                free(sprite_buffer);
            } break;
            // TODO: Implement these after keypad is thought about.
            case 0xe: {
                switch (current_instr[1]) {
                    case 0x9E: printf("%-10s V%01x NOT IMPLEMENTED\n", "SKIP.KEYX", instr_nib[1]); break;
                    case 0xA1: printf("%-10s V%01x NOT IMPLEMENTED\n", "SKIPN.KEYX", instr_nib[1]); break;
                    default: puts("Unknown `e` opcode.");
                }
            } break;
            case 0xf: {
                switch (current_instr[1]) {
                    case 0x07: {
                        printf("%-10s V%01x, DT\n", "LDX.DT", instr_nib[1]);

                        // Set Vx = delay timer value.
                        // The value of DT is placed into Vx.
                        device->v[instr_nib[1]] = device->delay_timer;
                    } break;
                    case 0x0A:{
                        printf("%-10s V%01x, KEY NOT IMPLEMENTED\n", "LDX.KEY", instr_nib[1]);
                        
                        //TODO: Keypad stuff
                    } break; 
                    case 0x15: {
                        printf("%-10s DT, V%01x\n", "LDDT.X", instr_nib[1]);

                        // Set delay timer = Vx.
                        // DT is set equal to the value of Vx.
                        device->delay_timer = device->v[instr_nib[1]];
                    } break;
                    case 0x18: {
                        printf("%-10s ST, V%01x\n", "LDST.X", instr_nib[1]);

                        // Set sound timer = Vx.
                        // ST is set equal to the value of Vx.
                        device->sound_timer = device->v[instr_nib[1]];
                    } break;
                    case 0x1E: {
                        printf("%-10s I, V%01x\n", "ADDI.X", instr_nib[1]);

                        // Set I = I + Vx.
                        // The values of I and Vx are added, and the results are stored in I.
                        device->i_reg += device->v[instr_nib[1]];
                    } break;
                    case 0x29: {
                        printf("%-10s I, Sprite: %01x NOT IMPLEMENTED\n", "LDI.FX", instr_nib[1]);

                        // Set I = location of sprite for digit Vx.
                        // The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
                        // TODO: give each font character a memory location/pointer and make it state accessible.
                    } break;
                    case 0x33: {
                        printf("%-10s I, (BCD)V%01x\n", "LDB.X", instr_nib[1]);

                        // Store BCD representation of Vx in memory locations I, I+1, and I+2.
                        // The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
                        device->memory[device->i_reg] = (device->v[instr_nib[1]] / 100);
                        device->memory[device->i_reg + 1] = (device->v[instr_nib[1]] / 10) % 10;
                        device->memory[device->i_reg + 2] = device->v[instr_nib[1]] % 10;
                    } break;
                    case 0x55: {
                        printf("%-10s I, V0 -> V%01x\n", "LDI.ALL", instr_nib[1]);

                        // Store registers V0 through Vx in memory starting at location I.
                        // The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
                        for (int i = 0; i <= instr_nib[1]; i++) {
                            device->memory[device->i_reg + i] = device->v[i];
                        }
                    } break;
                    case 0x65: {
                        printf("%-10s V0 -> V%01x, I NOT IMPLEMENTED\n", "LDX.ALL", instr_nib[1]);

                        // Read registers V0 through Vx from memory starting at location I.
                        // The interpreter reads values from memory starting at location I into registers V0 through Vx.
                        for (int i = 0; i <= instr_nib[1]; i++) {
                            device->v[i] = device->memory[device->i_reg + i];
                        }
                    } break;
                    default: puts("Unknown `f` opcode."); break;
                }
            } break;
        }
        
        // Increment PC by 2 after each instruction call.
        device->pc += 2;

        // Serious fuck up catcher.
        if (device->pc > MAX_MEMORY || device->pc < ROM_START) {
            device->exit_requested = 1;
        }
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