#pragma once

#include <view.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>

struct simwm_xdg_surface {
  // the view related to this xdg surface.
  struct simwm_view *view;

  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;

  struct wlr_scene_tree *scene;
  struct wlr_xdg_toplevel *toplevel;

  struct wl_listener request_move;
  struct wl_listener request_resize;
  struct wl_listener request_maximize;
};

void on_new_xdg_surface(struct wl_listener *, void *);
