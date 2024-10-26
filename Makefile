CC=gcc

SRCDIR=src
INCDIR=$(SRCDIR)
BIN=wusel

CFLAGS=-Wall -Wextra -Werror -Wpedantic -pedantic-errors
LOPT=-lm
LOPT+= $(shell pkg-config sdl2 --cflags --libs) -lSDL2_ttf

HEADERS=$(INCDIR)/utils.h $(INCDIR)/vec2.h $(INCDIR)/app.h $(INCDIR)/crt.h $(INCDIR)/pop.h $(INCDIR)/world.h $(INCDIR)/ui.h
OBJECTS=$(SRCDIR)/utils.o $(SRCDIR)/vec2.o $(SRCDIR)/app.o $(SRCDIR)/crt.o $(SRCDIR)/pop.o $(SRCDIR)/world.o $(SRCDIR)/ui.o

TESTDIR=tests
TEST_C=$(wildcard $(TESTDIR)/test.*.c)
TEST_O=$(OBJECTS) $(patsubst %.c, %.o, $(TEST_C))
TEST_H=$(HEADERS) $(TESTDIR)/test.h

all:	$(BIN) test

$(BIN):	$(OBJECTS) $(SRCDIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LOPT)

%.o:	%.c $(HEADERS)
	$(CC) $(COPT)-c $< -o $@ -I$(INCDIR)

test:	$(TEST_O) $(TESTDIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LOPT)

tests/%.o:	%.c $(TEST_H)
	$(CC) $(COPT)-c $< -o $@ -I$(INCDIR) -Itests

clean:
	rm -f $(SRCDIR)/*.o $(TESTDIR)/*.o $(BIN) test
