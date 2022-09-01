WAYLAND_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell pkg-config --variable=wayland_scanner wayland-scanner)
LIBS=\
	 $(shell pkg-config --cflags --libs wlroots) \
	 $(shell pkg-config --cflags --libs wayland-server) \
	 $(shell pkg-config --cflags --libs xkbcommon) \
	 $(shell pkg-config --cflags --libs lua)

export C_INCLUDE_PATH=./include

# wayland-scanner is a tool which generates C headers and rigging for Wayland
# protocols, which are specified in XML. wlroots requires you to rig these up
# to your build system yourself and provide them in the include path.
xdg-shell-protocol.h:
	$(WAYLAND_SCANNER) server-header \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

wlr-layer-shell-unstable-v1-protocol.h:
	$(WAYLAND_SCANNER) server-header \
		protocols/wlr-layer-shell-unstable-v1.xml $@

protocols: xdg-shell-protocol.h wlr-layer-shell-unstable-v1-protocol.h

simwm: simwm.c output.c server.c input.c cursor.c keyboard.c xdg_shell.c layer_shell.c view.c popup.c seat.c luaS.c
	make protocols
	$(CC) $(CFLAGS) \
		-g -Werror -I. \
		-DWLR_USE_UNSTABLE \
		-o $@ $^ \
		$(LIBS)

clean:
	rm -f simwm *-protocol.h

all: clean simwm

.DEFAULT_GOAL=simwm
