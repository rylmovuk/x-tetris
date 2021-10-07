.PHONY: all

CFLAGS=-ansi -pedantic
tetris: tetris.c
	$(CC) $(CFLAGS) $? -o $@

all: tetris