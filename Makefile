CC=gcc

SRCDIR=src
INCDIR=$(SRCDIR)
BIN=wusel


CFLAGS=-Wall -Wextra -Werror -Wpedantic -pedantic-errors
LOPT=-lm
LOPT+=$(shell pkg-config --libs glfw3) -lGL -lm -lGLU -lGLEW

HEADERS=$(INCDIR)/utils.h $(INCDIR)/vec2.h $(INCDIR)/app.h $(INCDIR)/world.h $(INCDIR)/qtree.h $(INCDIR)/ui.h $(INCDIR)/crt.h $(INCDIR)/nk_glfw3.h
OBJECTS=$(SRCDIR)/utils.o $(SRCDIR)/vec2.o $(SRCDIR)/app.o $(SRCDIR)/world.o $(SRCDIR)/qtree.o $(SRCDIR)/ui.o $(SRCDIR)/crt.o

TESTDIR=tests
TEST_C=$(wildcard $(TESTDIR)/test.*.c)
TEST_O=$(OBJECTS) $(patsubst %.c, %.o, $(TEST_C))
TEST_H=$(HEADERS) $(TESTDIR)/test.h

all:	prepare $(BIN) test

prepare:
	./scripts/make.build.sh

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
