DESTDIR   ?=
PREFIX    ?= /usr/local
MANPREFIX ?= $(PREFIX)/man

CFLAGS += -Wall -Wextra
LDLIBS += 

all: mpk

install: mpk mpk.1
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m755 mpk   $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)$(PREFIX)/bin/mpk

clean:
	rm -f mpk

.PHONY: all clean install uninstall
