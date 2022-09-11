CC = gcc
SRC = $(wildcard *.c luaS/*.c luaS/**/*.c)
OBJ := $(SRC:.c=.o)
INCDIRS = -I. -Iinclude/
CFLAGS = -g -Werror -DWLR_USE_UNSTABLE
LIBS=\
	 $(shell pkg-config --cflags --libs wlroots) \
	 $(shell pkg-config --cflags --libs wayland-server) \
	 $(shell pkg-config --cflags --libs xkbcommon) \
	 $(shell pkg-config --cflags --libs luajit)

WAYLAND_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell pkg-config --variable=wayland_scanner wayland-scanner)

xdg-shell-protocol.h:
	$(WAYLAND_SCANNER) server-header \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

wlr-layer-shell-unstable-v1-protocol.h:
	$(WAYLAND_SCANNER) server-header \
		protocols/wlr-layer-shell-unstable-v1.xml $@

protocols: xdg-shell-protocol.h wlr-layer-shell-unstable-v1-protocol.h

simwm: $(OBJ)
	$(CC) $(LIBS) $(INCDIRS) $(CFLAGS) -o $@ $^ \

%.o:%.c
	$(CC) $(LIBS) $(INCDIRS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f simwm *-protocol.h

all: clean protocols simwm

.DEFAULT_GOAL=simwm
