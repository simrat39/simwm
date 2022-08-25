#pragma once

#include <view.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>

void on_new_xdg_surface(struct wl_listener *, void *);
void focus_view(struct simwm_view *view, struct wlr_surface *surface);
