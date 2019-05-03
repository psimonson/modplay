CC?=gcc
CFLAGS=-std=c89 -Wall -Wextra -Wno-unused-function -g -I./include $(shell pkg-config sdl --cflags)
LDFLAGS=$(shell pkg-config sdl --libs) -lSDL_ttf -lSDL_mixer

SRCDIR=$(shell pwd)
DESTDIR=
PREFIX=/usr/local

OBJS=main.o
TRGT=modplay

.PHONY: all clean
all: $(TRGT)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

$(TRGT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(SRCDIR)/*~ $(SRCDIR)/*.log $(SRCDIR)/$(OBJS) $(SRCDIR)/$(TRGT)
