#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>

struct simwm_view {
  struct wl_list link;
  struct wl_list layout_link;

  struct wlr_xdg_toplevel *xdg_toplevel;
  struct wlr_scene_tree *scene_tree;

  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;

  struct wl_listener request_move;
  struct wl_listener request_resize;
  struct wl_listener request_maximize;

  int x, y;
};

void on_new_xdg_surface(struct wl_listener *, void *);
struct simwm_view *view_at(double lx, double ly, struct wlr_surface **surface,
                           double *sx, double *sy);
void focus_view(struct simwm_view *view, struct wlr_surface *surface);
