CFLAGS=-std=c2x -Wall -Werror -Wextra

all:
	@if [ ! -d "build" ]; then \
		echo "Build directory does not exist. Creating..." ; \
		mkdir build ; \
	fi

	gcc src/fish.c -o build/fish8 $(CFLAGS) `sdl2-config --cflags --libs`