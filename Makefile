.POSIX:

VERSION = 0.1
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

PKG_CONFIG = pkg-config

BCFLAGS = $(CFLAGS)
BLDFLAGS = `$(PKG_CONFIG) --cflags --libs json-c libcurl` -llexbor -lm

SRC_DIR = $(shell pwd)
SRCS = $(wildcard *.cpp)
OBJ = $(SRCS:%.c=%.o)

all: options blurrer

options:
	@echo blurrer build options:
	@echo "CFLAGS   = $(BCFLAGS)"
	@echo "LDFLAGS  = $(BLDFLAGS)"
	@echo "CC       = $(CC)"

.c.o:
	$(CC) -c $(BLDFLAGS) $<

blurrer: $(OBJ)
	$(CC) -o $@ $(OBJ) $(BCFLAGS) $(BLDFLAGS)

clean:
	rm -f blurrer $(OBJ)

install: blurrer
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	mkdir -p $(DESTDIR)$(PREFIX)/share/bash-completion/completions
	mkdir -p $(DESTDIR)$(PREFIX)/share/zsh/site-functions
	cp -f blurrer $(DESTDIR)$(PREFIX)/bin
	cp -f blurrer.1 $(DESDIR)$(MANPREFIX)/man1/blurrer.1
	chmod 775 $(DESTDIR)$(PREFIX)/bin/blurrer
	cp -f completion/bash/blurrer.bash $(DESTDIR)$(PREFIX)/share/bash-completion/completions/timestamp
	cp -f completion/zsh/_blurrer $(DESTDIR)$(PREFIX)/share/zsh/site-functions

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/blurrer
	rm -f $(DESKDIR)$(MANPREFIX)/man1/blurrer.1
	rm -f $(DESTDIR)$(PREFIX)/share/bash-completion/completions/blurrer
	rm -f $(DESTDIR)$(PREFIX)/share/zsh/site-functions/_blurrer

.PHONY: all options blurrer clean install uninstall
