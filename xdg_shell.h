#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>

void on_new_xdg_surface(struct wl_listener *, void *);
struct simwm_view *view_at(double lx, double ly, struct wlr_surface **surface,
                           double *sx, double *sy);
void focus_view(struct simwm_view *view, struct wlr_surface *surface);
