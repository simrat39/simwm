#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>

enum simwm_view_type { SIMWM_VIEW_XDG, SIMWM_VIEW_LAYER };

struct simwm_view {
  enum simwm_view_type type;

  struct wl_list layout_link;

  struct wlr_layer_surface_v1 *layer_surface;
  struct wlr_output *output;

  struct wlr_xdg_toplevel *xdg_toplevel;

  struct wlr_scene_tree *scene_tree;

  struct wl_listener commit;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;

  struct wl_listener request_move;
  struct wl_listener request_resize;
  struct wl_listener request_maximize;

  int x, y;
};

struct simwm_view *view_at(double lx, double ly, struct wlr_surface **surface,
                           double *sx, double *sy);
