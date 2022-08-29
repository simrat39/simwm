#pragma once

#include <wayland-server-core.h>

enum simwm_anchor {
  SIMWM_ANCHOR_ALL,
  SIMWM_ANCHOR_TOP,
  SIMWM_ANCHOR_BOTTOM,
  SIMWM_ANCHOR_LEFT,
  SIMWM_ANCHOR_RIGHT,
  SIMWM_ANCHOR_TOP_LEFT,
  SIMWM_ANCHOR_TOP_RIGHT,
  SIMWM_ANCHOR_BOTTOM_LEFT,
  SIMWM_ANCHOR_BOTTOM_RIGHT,
  SIMWM_ANCHOR_NONE,
};

enum simwm_layer {
  LAYER_BG,
  LAYER_BOTTOM,
  LAYER_TOP,
  LAYER_OVERLAY,
  LAYER_TILE,
  LAYER_COUNT,
};

struct simwm_layer_surface {
  // the view related to this layer surface.
  struct simwm_view *view;

  struct simwm_output *output;
  struct wlr_scene_layer_surface_v1 *scene;
  struct wlr_scene_tree *popup_scene;
  
  bool mapped;

  struct wl_listener commit;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;
  struct wl_listener new_popup;
};

void on_new_layer_surface(struct wl_listener *, void *);
