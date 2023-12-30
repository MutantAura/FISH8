#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#include "fish.h"
#include "cpu.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

void InputHandler(Fish* fish) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT: fish->exit_requested = 1; break;
        case SDL_KEYDOWN: {
            printf("%s\n", SDL_GetKeyName(event.key.keysym.sym));
            switch(event.key.keysym.sym) {
                // General functions
                case SDLK_ESCAPE: fish->exit_requested = 1; break;

                // Emulated keypad
                case SDLK_0: fish->keypad[0x0] = 1; break;
                case SDLK_1: fish->keypad[0x1] = 1; break;
                case SDLK_2: fish->keypad[0x2] = 1; break;
                case SDLK_3: fish->keypad[0x3] = 1; break;
                case SDLK_4: fish->keypad[0x4] = 1; break;
                case SDLK_5: fish->keypad[0x5] = 1; break;
                case SDLK_6: fish->keypad[0x6] = 1; break;
                case SDLK_7: fish->keypad[0x7] = 1; break;
                case SDLK_8: fish->keypad[0x8] = 1; break;
                case SDLK_9: fish->keypad[0x9] = 1; break;
                case SDLK_a: fish->keypad[0xA] = 1; break;
                case SDLK_b: fish->keypad[0xB] = 1; break;
                case SDLK_c: fish->keypad[0xC] = 1; break;
                case SDLK_d: fish->keypad[0xD] = 1; break;
                case SDLK_e: fish->keypad[0xE] = 1; break;
                case SDLK_f: fish->keypad[0xF] = 1; break;
            }
        } break;
        case SDL_KEYUP: {
            switch(event.key.keysym.sym) {
                // Emulated keypad
                case SDLK_0: fish->keypad[0x0] = 0; break;
                case SDLK_1: fish->keypad[0x1] = 0; break;
                case SDLK_2: fish->keypad[0x2] = 0; break;
                case SDLK_3: fish->keypad[0x3] = 0; break;
                case SDLK_4: fish->keypad[0x4] = 0; break;
                case SDLK_5: fish->keypad[0x5] = 0; break;
                case SDLK_6: fish->keypad[0x6] = 0; break;
                case SDLK_7: fish->keypad[0x7] = 0; break;
                case SDLK_8: fish->keypad[0x8] = 0; break;
                case SDLK_9: fish->keypad[0x9] = 0; break;
                case SDLK_a: fish->keypad[0xA] = 0; break;
                case SDLK_b: fish->keypad[0xB] = 0; break;
                case SDLK_c: fish->keypad[0xC] = 0; break;
                case SDLK_d: fish->keypad[0xD] = 0; break;
                case SDLK_e: fish->keypad[0xE] = 0; break;
                case SDLK_f: fish->keypad[0xF] = 0; break;
            }
        } break;
    }
}

void UpdateRenderer(Fish* state) {
    state->request_draw = 0;
    ClearScreen();

    SDL_Rect temp;
    uint8_t display_bit;

    for (int i = 0; i < DISPLAY_HEIGHT; i++) {
        for (int j = 0; j < DISPLAY_WIDTH; j++) {
            display_bit = state->display[i][j] * UINT8_MAX;

            temp.x = j * DISPLAY_SCALE;
            temp.y = i * DISPLAY_SCALE;
            temp.w = DISPLAY_SCALE;
            temp.h = DISPLAY_SCALE;
                
            SDL_SetRenderDrawColor(renderer, display_bit, display_bit, display_bit, 0xFF);
            SDL_RenderFillRect(renderer, &temp);
        }
    }
    SDL_RenderPresent(renderer);
}

void ClearScreen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        puts("you are stupid.");
        return 1;
    }

    // Setup device
    Fish state = {0};
    InitFish(&state);

    if (!InitSDL()) { return 1; }

    // Load ROM file into device memory.
    if (LoadRom(argv[1], &state.memory[ROM_START]) != 0) {
        puts("You are also stupid (file error)");
        return 1;
    }

    ClearScreen();

    // Enter SDL loop?
    while (!state.exit_requested) {
        InputHandler(&state);
        EmulateCpu(&state);
        if (state.request_draw) {
            UpdateRenderer(&state);
        }
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void InitFish(Fish* state) {
    memset(state->display, 0, sizeof(state->display));
    state->pc = ROM_START;
    state->sp = 0;
    state->frequency = REFRESH_RATE;
    state->exit_requested = 0;

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

    memcpy(&state->memory[FONT_START], &font_array, sizeof(font_array));
}

int LoadRom(char* file_name, uint8_t* memory) {
    FILE* rom = fopen(file_name, "rb");
    if (rom == NULL) {
        return 1;
    }

    fseek(rom, 0L, SEEK_END);
    int file_size = ftell(rom);
    fseek(rom, 0L, SEEK_SET);

    if (fread(memory, file_size, 1, rom) != 1) {
        return 1;
    }

    fclose(rom);

    return 0;
}

int InitSDL() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        puts("SDL initialization error...");
        return 0;
    }

    window = SDL_CreateWindow("Fish8 - 0.0.1", 
                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                             DISPLAY_WIDTH * DISPLAY_SCALE, DISPLAY_HEIGHT * DISPLAY_SCALE, 
                             0);
    if (window == NULL) {
        puts("Failed to create an SDL window...");
        return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        puts("Failed to create SDL renderer...");
        return 0;
    }

    return 1;
}