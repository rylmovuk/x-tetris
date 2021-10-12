.PHONY: all clean

CFLAGS=-ansi -pedantic
CDEFINES=-DUTF8_SUPPORT

tetris: tetris.c graphics.h
	$(CC) $(CFLAGS) $(CDEFINES) $@.c -o $@

all: tetris

clean:
	rm tetris