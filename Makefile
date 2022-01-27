.PHONY: all clean debug release prep remake

CFLAGS = -std=c89 -pedantic

SRCS = tetris.c util.c iohandler.c opponentai.c
OBJS = $(SRCS:.c=.o)
EXE = x-tetris

#
# Debug build settings
#
DBGDIR = build/debug
DBGEXE = $(DBGDIR)/$(EXE)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGCFLAGS = -g -O0 -DDEBUG

#
# Release build settings
#
RELDIR = build/release
RELEXE = $(RELDIR)/$(EXE)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELCFLAGS = -O3 -DNDEBUG

all: RELEXE = $(EXE)
all: prep release

$(DBGDIR)/tetris.o: tetris.h iohandler.h opponentai.h
$(DBGDIR)/iohandler.o: iohandler.h tetris.h util.h
$(DBGDIR)/opponentai.o: opponentai.h tetris.h util.h

$(RELDIR)/tetris.o: tetris.h iohandler.h opponentai.h
$(RELDIR)/iohandler.o: iohandler.h tetris.h util.h
$(RELDIR)/opponentai.o: opponentai.h tetris.h util.h

$(OBJS): constants.h 

#
# Debug rules
#
debug: $(DBGEXE)

$(DBGEXE): $(DBGOBJS)
	$(CC) $(CFLAGS) $(DBGCFLAGS) -o $(DBGEXE) $^

$(DBGDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(DBGCFLAGS) -o $@ $<

#
# Release rules
#
release: $(RELEXE)

$(RELEXE): $(RELOBJS)
	$(CC) $(CFLAGS) $(RELCFLAGS) -o $(RELEXE) $^

$(RELDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(RELCFLAGS) -o $@ $<

#
# Other rules
#
prep:
	@mkdir -p $(DBGDIR) $(RELDIR)

remake: clean all

clean:
	rm -f $(EXE) $(RELEXE) $(RELOBJS) $(DBGEXE) $(DBGOBJS)
