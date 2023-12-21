#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define ROM_START 0x200

void FishDiasm(uint8_t* buffer, int pc) {
    uint8_t* code = &buffer[pc];

    uint8_t first_nib = (code[0] >> 4);
    uint8_t second_nib = (code[0] & 0x0F);
    uint8_t third_nib = (code[1] >> 4);
    uint8_t fourth_nib = (code[1] & 0x0F);

    printf("%04x %02x %02x ", pc, code[0], code[1]);

    // Opcodes from http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#8xy0
    // Some name changes?
    switch(first_nib) {
        case 0x00: {
            switch (code[1]) {
                case 0xE0: printf("%-10s\n", "CLS"); break;
                case 0xEE: printf("%-10s\n", "RET"); break;
                default: printf("%-10s $%01x%01x%01x\n", "SYS", second_nib, third_nib, fourth_nib);
            }
        } break;
        case 0x1: printf("%-10s $%01x%01x%01x\n", "JMP", second_nib, third_nib, fourth_nib); break;
        case 0x2: printf("%-10s $%01x%01x%01x\n", "CALL", second_nib, third_nib, fourth_nib); break;
        case 0x3: printf("%-10s V%01x, #$%02x\n", "SKIP.CMP", second_nib, code[1]); break;
        case 0x4: printf("%-10s V%01x, #$%02x\n", "SKIP.NCMP", second_nib, code[1]); break;
        case 0x5: printf("%-10s V%01x, V%01x\n", "SKIP.RCMP", second_nib, third_nib); break;
        case 0x6: printf("%-10s V%01X,#$%02x\n", "MVI", second_nib, code[1]); break;
        case 0x7: printf("%-10s V%01X,#$%02x\n", "ADD", second_nib, code[1]); break;
        case 0x8: {
            switch (fourth_nib) {
                case 0x0: printf("%-10s V%01x,V%01x\n", "CPY", second_nib, third_nib); break;
                case 0x1: printf("%-10s V%01x,V%01x\n", "OR", second_nib, third_nib); break;
                case 0x2: printf("%-10s V%01x,V%01x\n", "AND", second_nib, third_nib); break;
                case 0x3: printf("%-10s V%01x,V%01x\n", "XOR", second_nib, third_nib); break;
                case 0x4: printf("%-10s V%01x,V%01x (VF)\n", "ADD", second_nib, third_nib); break;
                case 0x5: printf("%-10s V%01x,V%01x\n", "SUB", second_nib, third_nib); break;
                case 0x6: printf("%-10s V%01x,V%01x (VF)\n", "SHR", second_nib, third_nib); break;
                case 0x7: printf("%-10s V%01x,V%01x (VF)\n", "SUBN", second_nib, third_nib); break;
                case 0xE: printf("%-10s V%01x,V%01x (VF)\n", "SHL", second_nib, third_nib); break;
                default: puts("Unknown `8` opcode."); break;
            }
        } break;
        case 0x9: printf("%-10s V%01x, V%01x\n", "SNE", second_nib, third_nib); break;
        case 0xa: printf("%-10s I,#$%01x%02x\n", "LDI", second_nib, code[1]); break;
        case 0xb: printf("%-10s $%01x%02x + V0\n", "JMP.V", second_nib, code[1]); break;
        case 0xc: printf("%-10s V%01x, #$%02x\n", "RAND", second_nib, code[1]); break;
        case 0xd: printf("%-10s V%01x, V%01x bytes: %01d\n", "DRW", second_nib, third_nib, (int)fourth_nib); break;
        case 0xe: {
            switch (code[1]) {
                case 0x9E: printf("%-10s V%01x\n", "SKIP.KEYX", second_nib); break;
                case 0xA1: printf("%-10s V%01x\n", "SKIPN.KEYX", second_nib); break;
                default: puts("Unknown `e` opcode.");
            }
        } break;
        case 0xf: {
            switch (code[1]) {
                case 0x07: printf("%-10s V%01x, DT\n", "LDX.DT", second_nib); break;
                case 0x0A: printf("%-10s V%01x, KEY\n", "LDX.KEY", second_nib); break;
                case 0x15: printf("%-10s DT, V%01x\n", "LDDT.X", second_nib); break;
                case 0x18: printf("%-10s ST, V%01x\n", "LDST.X", second_nib); break;
                case 0x1E: printf("%-10s I, V%01x\n", "ADDI.X", second_nib); break;
                case 0x29: printf("%-10s I, Sprite: %01x\n", "LDI.FX", second_nib); break;
                case 0x33: printf("%-10s I, (BCD)V%01x\n", "LDB.X", second_nib); break;
                case 0x55: printf("%-10s I, V0 -> V%01x\n", "LDI.ALL", second_nib); break;
                case 0x65: printf("%-10s V0 -> V%01x, I\n", "LDX.ALL", second_nib); break;
                default: puts("Unknown `f` opcode."); break;
            }
        } break;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        puts("Correct usage is: `$ fishasm path_to_chip8_rom`");
        return 1;
    }

    clock_t start = clock();

    // Get file pointer.
    FILE* file = fopen(argv[1], "rb");

    if (file == NULL) {
        puts("Failed to open file.");
        return 2;
    }

    // Get file size for import.
    fseek(file, 0L, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    uint8_t* buffer = malloc(file_size + ROM_START);

    fread(buffer + ROM_START, file_size, 1, file);
    fclose(file);

    int instr_count = 0;
    for (int pc = ROM_START; pc < file_size + ROM_START; pc+=2) {
        FishDiasm(buffer, pc);
        instr_count++;
    }
    
    clock_t end = clock();
    printf("Finished with %d instructions in %f seconds.\n", instr_count, ((double)end-start)/CLOCKS_PER_SEC);

    return 0;
}