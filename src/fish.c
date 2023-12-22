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
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                fish->exit_requested = 1;
            } 
        } break;
    }
}

void ProcessState() {

}

void UpdateRenderer() {

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
    if (LoadRom(argv[1], &state.memory[ROM_START]) != 0) {
        puts("You are also stupid (file error)");
    }

    if (!InitSDL()) {
        return 1;
    }

    // Enter SDL loop?
    while (!state.exit_requested) {
        InputHandler(&state);
        EmulateCpu(&state);
        ProcessState();
        UpdateRenderer();
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void InitFish(Fish* state) {
    state->display = &state->memory[DISPLAY_START];
    state->pc = ROM_START;
    state->sp = 0;
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

    fread(memory, file_size, 1, rom);
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
                             DISPLAY_WIDTH * 10, DISPLAY_HEIGHT * 10, 
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