#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>

enum simwm_view_type { SIMWM_VIEW_XDG, SIMWM_VIEW_LAYER };

struct simwm_view {
  enum simwm_view_type type;

  struct wl_list layout_link;
  struct wl_list layer_link;

  struct simwm_output *output;
  struct wlr_box usable_area;

  struct wlr_xdg_toplevel *xdg_toplevel;

  union {
    struct wlr_scene_layer_surface_v1 *scene_layer_surface;
    struct wlr_scene_tree *scene_tree;
  };

  struct wlr_scene_tree *popup_scene_tree;

  struct wl_listener commit;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;

  struct wl_listener new_popup;

  struct wl_listener request_move;
  struct wl_listener request_resize;
  struct wl_listener request_maximize;

  bool mapped;

  int x, y;
  int width, height;
};

struct simwm_view *view_at(double lx, double ly, struct wlr_surface **surface,
                           double *sx, double *sy);
