CFLAGS=-std=c2x -Wall -Werror -Wextra -O2

all:
	@if [ ! -d "build" ]; then \
		echo "Build directory does not exist. Creating..." ; \
		mkdir build ; \
	fi

	gcc src/fish.c src/cpu.c -o build/fish8 $(CFLAGS) `pkg-config --cflags --libs sdl2`

clean:
	rm -rf build/
	make all