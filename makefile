CFLAGS=-std=c2x -Wall -Werror -Wextra

all:
	gcc src/fish.c -o build/fish8 $(CFLAGS) `sdl2-config --cflags --libs`
	gcc src/fish_diasm.c -o build/fish-diassembler $(CFLAGS)