#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#include "fish.h"
#include "cpu.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
const uint8_t* keyboard_state = NULL;
int last_frame_ticks = 0;

void InputHandler(Fish* fish, SDL_Event* event) {
    if (event->type == SDL_QUIT) {
        fish->exit_requested = 1;
        return;
    }

    if (keyboard_state[SDL_SCANCODE_ESCAPE]) {
        fish->exit_requested = 1;
        return;
    }
    if (keyboard_state[SDL_SCANCODE_0]) {
            fish->keypad[0x0] = 1;
    } else fish->keypad[0x0] = 0;
    if (keyboard_state[SDL_SCANCODE_1]) {
        fish->keypad[0x1] = 1;
    } else fish->keypad[0x1] = 0;
    if (keyboard_state[SDL_SCANCODE_2]) {
        fish->keypad[0x2] = 1;
    } else fish->keypad[0x2] = 0;
    if (keyboard_state[SDL_SCANCODE_3]) {
        fish->keypad[0x3] = 1;
    } else fish->keypad[0x3] = 0;
    if (keyboard_state[SDL_SCANCODE_4]) {
        fish->keypad[0x4] = 1;
    } else fish->keypad[0x4] = 0;
    if (keyboard_state[SDL_SCANCODE_5]) {
        fish->keypad[0x5] = 1;
    } else fish->keypad[0x5] = 0;
    if (keyboard_state[SDL_SCANCODE_6]) {
        fish->keypad[0x6] = 1;
    } else fish->keypad[0x6] = 0;
    if (keyboard_state[SDL_SCANCODE_7]) {
        fish->keypad[0x7] = 1;
    } else fish->keypad[0x7] = 0;
    if (keyboard_state[SDL_SCANCODE_8]) {
        fish->keypad[0x8] = 1;
    } else fish->keypad[0x8] = 0;
    if (keyboard_state[SDL_SCANCODE_9]) {
        fish->keypad[0x9] = 1;
    } else fish->keypad[0x9] = 0;
    if (keyboard_state[SDL_SCANCODE_A]) {
        fish->keypad[0xA] = 1;
    } else fish->keypad[0xA] = 0;
    if (keyboard_state[SDL_SCANCODE_B]) {
        fish->keypad[0xB] = 1;
    } else fish->keypad[0xB] = 0;
    if (keyboard_state[SDL_SCANCODE_C]) {
        fish->keypad[0xC] = 1;
    } else fish->keypad[0xC] = 0;
    if (keyboard_state[SDL_SCANCODE_D]) {
        fish->keypad[0xD] = 1;
    } else fish->keypad[0xD] = 0;
    if (keyboard_state[SDL_SCANCODE_E]) {
        fish->keypad[0xE] = 1;
    } else fish->keypad[0xE] = 0;
    if (keyboard_state[SDL_SCANCODE_F]) {
        fish->keypad[0xF] = 1;
    } else fish->keypad[0xF] = 0;
}

void UpdateRenderer(Fish* state) {
    SDL_Rect temp;
    uint8_t display_bit;

    for (int i = 0; i < DISPLAY_HEIGHT; i++) {
        for (int j = 0; j < DISPLAY_WIDTH; j++) {
            // If display_bit = 0, drawing colour = black.
            // If display_bit = 1, drawing colour = white.
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

void UpdateTimers(Fish* state) {
    if (state->delay_timer > 0) {
        state->delay_timer--;
    }

    if (state->sound_timer > 0) {
        state->sound_timer--;
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

    if (!InitSDL()) { return 1; }

    // Load ROM file into device memory.
    if (LoadRom(argv[1], &state.memory[ROM_START]) != 0) {
        puts("You are also stupid (file error)");
        return 1;
    }

    ClearScreen();

    // Enter SDL loop?
    while (!state.exit_requested) {
        last_frame_ticks = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            InputHandler(&state, &event);
        }
        
        // Assume each cycle = 1 instruction.
        for (int i = 0; i < state.frequency; i++) {
            EmulateCpu(&state);
        }

        UpdateRenderer(&state);

        int render_cost = SDL_GetTicks() - last_frame_ticks;
        if (render_cost < (1000/REFRESH_RATE)) {
            SDL_Delay((1000/REFRESH_RATE) - render_cost);
        }
        
        UpdateTimers(&state);
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
    state->frequency = 500;
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

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        puts("Failed to create SDL renderer...");
        return 0;
    }

    keyboard_state = SDL_GetKeyboardState(NULL);
    if (keyboard_state == NULL) {
        puts("Failed to get initial SDL keyboard state...");
        return 0;
    }

    last_frame_ticks = SDL_GetTicks();

    return 1;
}