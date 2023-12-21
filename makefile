CFLAGS=-std=c2x -Wall -Werror -Wextra

all:
	mkdir build
	gcc src/fish.c -o build/fish8 $(CFLAGS) `sdl2-config --cflags --libs`