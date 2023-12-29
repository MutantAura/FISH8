CFLAGS=-std=c2x -Wall -Werror -Wextra

all:
	@if [ ! -d "build" ]; then \
		echo "Build directory does not exist. Creating..." ; \
		mkdir build ; \
	fi

	gcc src/fish.c src/cpu.c -o build/fish8 $(CFLAGS) `pkg-config --cflags --libs sdl2`