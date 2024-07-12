#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "cpu.h"

void EmulateCpu(Fish* device, int is_debug) {
    // Seed RNG
    srand(time(NULL));

    // Extract opcode data and break down into nibbles.
    uint8_t* current_instr = &device->memory[device->pc];
    uint8_t instr_nib[] = {
        current_instr[0] >> 4,
        current_instr[0] & 0x0F,
        current_instr[1] >> 4,
        current_instr[1] & 0x0F
    };

    if (is_debug) { printf("%04x %02x %02x ", device->pc, current_instr[0], current_instr[1]); }

    // Opcodes from http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
    switch(instr_nib[0]) {
        case 0x00: {
            switch (current_instr[1]) {
                case 0xE0: {
                    if (is_debug) { printf("%-10s\n", "CLS"); }

                    // Clear the display.
                    memset(&device->display[0][0], 0, sizeof(device->display));
                } break;
                case 0xEE: {
                    if (is_debug) { printf("%-10s\n", "RET"); }
                    device->sp--;
                    device->pc = device->stack[device->sp];
                } break;
                default: printf("%-10s $%01x%01x%01x\n", "SYS (NOP)", instr_nib[1], instr_nib[2], instr_nib[3]);
            }
        } break;
        case 0x1: {
            if (is_debug) { printf("%-10s $%01x%01x%01x\n", "JMP", instr_nib[1], instr_nib[2], instr_nib[3]); } 

            device->pc = (instr_nib[1] << 8) | current_instr[1];
            device->pc -= 2;
        } break;
        case 0x2: {
            if (is_debug) { printf("%-10s $%01x%01x%01x\n", "CALL", instr_nib[1], instr_nib[2], instr_nib[3]); }

            device->stack[device->sp] = device->pc;
            device->sp++;
            device->pc = (instr_nib[1] << 8) | current_instr[1];
            device->pc -= 2;
        } break;
        case 0x3: {
            if (is_debug) { printf("%-10s V%01x, #$%02x\n", "SKIP.CMP", instr_nib[1], current_instr[1]); }

            if (device->v[instr_nib[1]] == current_instr[1]) {
                device->pc += 2;
            }
        } break;
        case 0x4: {
            if (is_debug) { printf("%-10s V%01x, #$%02x\n", "SKIP.NCMP", instr_nib[1], current_instr[1]); }

            if (device->v[instr_nib[1]] != current_instr[1]) {
                device->pc += 2;
            }
        } break;
        case 0x5: {
            if (is_debug) { printf("%-10s V%01x, V%01x\n", "SKIP.RCMP", instr_nib[1], instr_nib[2]); }

            if (device->v[instr_nib[1]] == device->v[instr_nib[2]]) {
                device->pc += 2;
            }

        } break;
        case 0x6: {
            if (is_debug) { printf("%-10s V%01X,#$%02x\n", "MVI", instr_nib[1], current_instr[1]); }

            device->v[instr_nib[1]] = current_instr[1];
        } break;
        case 0x7: {
            if (is_debug) { printf("%-10s V%01X,#$%02x\n", "ADD", instr_nib[1], current_instr[1]); }

            device->v[instr_nib[1]] += current_instr[1];
        } break;
        case 0x8: {
            switch (instr_nib[3]) {
                case 0x0: {
                    if (is_debug) { printf("%-10s V%01x,V%01x\n", "MOV", instr_nib[1], instr_nib[2]); }

                    device->v[instr_nib[1]] = device->v[instr_nib[2]];
                } break;
                case 0x1: {
                    if (is_debug) { printf("%-10s V%01x,V%01x\n", "OR", instr_nib[1], instr_nib[2]); }

                    device->v[instr_nib[1]] |= device->v[instr_nib[2]];
                } break; 
                case 0x2: {
                    if (is_debug) { printf("%-10s V%01x,V%01x\n", "AND", instr_nib[1], instr_nib[2]); }

                    device->v[instr_nib[1]] &= device->v[instr_nib[2]];
                } break;
                case 0x3: {
                    if (is_debug) { printf("%-10s V%01x,V%01x\n", "XOR", instr_nib[1], instr_nib[2]); }

                    device->v[instr_nib[1]] ^= device->v[instr_nib[2]];
                } break;
                case 0x4: {
                    if (is_debug) { printf("%-10s V%01x,V%01x\n", "ADD", instr_nib[1], instr_nib[2]); }

                    uint16_t overflow = device->v[instr_nib[1]] + device->v[instr_nib[2]];
                    device->v[instr_nib[1]] = (uint8_t)(overflow & 0x00FF);
                    device->v[0xF] = overflow > UINT8_MAX;
                } break;
                case 0x5: {
                    if (is_debug) { printf("%-10s V%01x,V%01x\n", "SUB", instr_nib[1], instr_nib[2]); }

                    uint8_t tempX = device->v[instr_nib[1]];
                    device->v[instr_nib[1]] -= device->v[instr_nib[2]];

                    device->v[0xF] = tempX >= device->v[instr_nib[2]];
                } break;
                case 0x6: {
                    if (is_debug) { printf("%-10s V%01x,V%01x\n", "SHR", instr_nib[1], instr_nib[2]); }

                    uint8_t tempX = device->v[instr_nib[1]];
                    device->v[instr_nib[1]] >>= 1;

                    device->v[0xF] = tempX & 0x01;
                } break;
                case 0x7: {
                    if (is_debug) { printf("%-10s V%01x,V%01x\n", "SUBN", instr_nib[1], instr_nib[2]); }

                    uint8_t tempX = device->v[instr_nib[1]];
                    device->v[instr_nib[1]] = device->v[instr_nib[2]] - device->v[instr_nib[1]];

                    device->v[0xF] = device->v[instr_nib[2]] >= tempX; 
                } break;
                case 0xE: {
                    if (is_debug) { printf("%-10s V%01x,V%01x (VF)\n", "SHL", instr_nib[1], instr_nib[2]); }

                    uint8_t tempX = device->v[instr_nib[1]];
                    device->v[instr_nib[1]] <<= 1;

                    device->v[0xF] = (tempX & 0x80) >> 7; 
                } break;
                default: puts("Unknown `8` opcode."); break;
            }
        } break;
        case 0x9: {
            if (is_debug) { printf("%-10s V%01x, V%01x\n", "SNE", instr_nib[1], instr_nib[2]); }

            if (device->v[instr_nib[1]] != device->v[instr_nib[2]]) {
                device->pc += 2;
            }
        } break;
        case 0xa: {
            if (is_debug) { printf("%-10s I,#$%01x%02x\n", "LDI", instr_nib[1], current_instr[1]); }

            device->i_reg = (instr_nib[1] << 8) | current_instr[1];
        } break;
        case 0xb: {
            if (is_debug) { printf("%-10s $%01x%02x + V0\n", "JMP.V", instr_nib[1], current_instr[1]); }

            device->pc = ((instr_nib[1] << 8) | current_instr[1]) + device->v[0];
        } break;
        case 0xc: {
            if (is_debug) { printf("%-10s V%01x, #$%02x\n", "RAND", instr_nib[1], current_instr[1]); }

            uint8_t random = (rand() % UINT8_MAX) & current_instr[1];
            device->v[instr_nib[1]] = random;
        } break;
        case 0xd: {
            if (is_debug) { printf("%-10s V%01x, V%01x bytes: %01d\n", "DRW", instr_nib[1], instr_nib[2], (int)instr_nib[3]); }
 
            uint8_t x_coord = device->v[instr_nib[1]];
            uint8_t y_coord = device->v[instr_nib[2]];

            device->v[0xF] = 0;
            // Loop over each row from Y -> Y + sprite height.
            for (int row = 0; row < instr_nib[3]; row++) {
                // Loop over each bit of the row byte (8 bits).
                for (int col = 0; col < 8; col++) {
                    int sprite_bit = (device->memory[device->i_reg + row] >> (7 - col)) & 0x1;

                    if (sprite_bit && device->display[y_coord + row][x_coord + col]) {
                        device->v[0xF] = 1;
                    }
                    device->display[y_coord + row][x_coord + col] ^= sprite_bit;
                }
            }

            device->draw_requested = 1;
        } break;
        case 0xe: {
            switch (current_instr[1]) {
                case 0x9E: {
                    if (is_debug) { printf("%-10s V%01x\n", "SKIP.KEYX", instr_nib[1]); }

                    if (device->keypad[device->v[instr_nib[1]]]) {
                        device->pc += 2;
                    }
                } break;
                case 0xA1: {
                    if (is_debug) { printf("%-10s V%01x\n", "SKIPN.KEYX", instr_nib[1]); }

                    if (!device->keypad[device->v[instr_nib[1]]]) {
                        device->pc += 2;
                    }
                } break;
                default: puts("Unknown `e` opcode.");
            }
        } break;
        case 0xf: {
            switch (current_instr[1]) {
                case 0x07: {
                    if (is_debug) { printf("%-10s V%01x, DT\n", "LDX.DT", instr_nib[1]); }

                    device->v[instr_nib[1]] = device->delay_timer;
                } break;
                case 0x0A:{
                    if (is_debug) { printf("%-10s V%01x\n", "LDX.KEY", instr_nib[1]); }

                    for (uint8_t i = 0; i < sizeof(device->keypad); i++) {
                        if (device->keypad[i] == 0 && device->keypad_buffer[i] == 1) {
                            device->v[instr_nib[1]] = i;
                            device->pc += 2;
                            break;
                        }
                    }
                    memcpy(&device->keypad_buffer[0], &device->keypad[0], sizeof(device->keypad));
                    device->pc -= 2;
                } break; 
                case 0x15: {
                    if (is_debug) { printf("%-10s DT, V%01x\n", "LDDT.X", instr_nib[1]); }

                    device->delay_timer = device->v[instr_nib[1]];
                } break;
                case 0x18: {
                    if (is_debug) { printf("%-10s ST, V%01x\n", "LDST.X", instr_nib[1]); }

                    device->sound_timer = device->v[instr_nib[1]];
                } break;
                case 0x1E: {
                    if (is_debug) { printf("%-10s I, V%01x\n", "ADDI.X", instr_nib[1]); }

                    device->i_reg += device->v[instr_nib[1]];
                } break;
                case 0x29: {
                    if (is_debug) { printf("%-10s I, Sprite: %01x\n", "LDI.FX", instr_nib[1]); }

                    device->i_reg = FONT_START + (instr_nib[1] * FONT_STRIDE);
                } break;
                case 0x33: {
                    if (is_debug) { printf("%-10s I, (BCD)V%01x\n", "LDB.X", instr_nib[1]); }

                    device->memory[device->i_reg] = (device->v[instr_nib[1]] / 100);
                    device->memory[device->i_reg + 1] = (device->v[instr_nib[1]] / 10) % 10;
                    device->memory[device->i_reg + 2] = device->v[instr_nib[1]] % 10;
                } break;
                case 0x55: {
                    if (is_debug) { printf("%-10s I, V0 -> V%01x\n", "LDI.ALL", instr_nib[1]); }

                    for (int i = 0; i <= instr_nib[1]; i++) {
                        device->memory[device->i_reg + i] = device->v[i];
                    }
                } break;
                case 0x65: {
                    if (is_debug) { printf("%-10s V0 -> V%01x, I\n", "LDX.ALL", instr_nib[1]); }

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
        puts("fuck up detected... exiting...");
        device->exit_requested = 1;
    }
}