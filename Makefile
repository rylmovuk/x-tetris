.PHONY: all clean

CFLAGS=-ansi -pedantic

tetris: tetris.c graphics.h
	$(CC) $(CFLAGS) -DUTF8_SUPPORT tetris.c -o $@

tetris-compat: tetris.c graphics.h
	$(CC) $(CFLAGS) tetris.c -o $@

all: tetris tetris-compat

clean:
	rm tetris tetris-compat