#pragma once

#include <wayland-server-core.h>
#include <wayland-util.h>

void cursor_init();
void cursor_events_init();

void on_cursor_motion(struct wl_listener *, void *);
void on_cursor_motion_absolute(struct wl_listener *, void *);
void on_cursor_button(struct wl_listener *, void *);
void on_seat_request_cursor(struct wl_listener *, void *);
