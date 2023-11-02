CFLAGS=-std=c2x -Wall -Werror -Wextra

all:
	gcc src/fish.c -o fish8 $(CFLAGS)