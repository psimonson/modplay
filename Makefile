CC?=gcc
CFLAGS=-std=c89 -Wall -Wextra -Wno-unused-function -g -I./include $(shell pkg-config sdl --cflags)
LDFLAGS=$(shell pkg-config sdl --libs) -lSDL_ttf -lSDL_mixer

SRCDIR=$(shell pwd)
DESTDIR=
PREFIX=usr/local
CFGFILE=config.h

OBJS=main.o
TRGT=modplay

.PHONY: all clean install uninstall
all: $(CFGFILE) $(TRGT)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

$(TRGT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CFGFILE):
	cp $(SRCDIR)/config.def.h $(SRCDIR)/$(CFGFILE)
	
clean:
	rm -f $(SRCDIR)/*~ $(SRCDIR)/*.log $(SRCDIR)/$(OBJS) $(SRCDIR)/$(TRGT)

install:
	mkdir -p $(DESTDIR)/$(PREFIX)/bin
	install $(TRGT) -t $(DESTDIR)/$(PREFIX)/bin
	mkdir -p $(DESTDIR)/$(PREFIX)/share/modplay
	cp -R $(SRCDIR)/music $(DESTDIR)/$(PREFIX)/share/modplay

uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/bin/$(TRGT)
	rm -rf $(DESTDIR)/$(PREFIX)/share/modplay
