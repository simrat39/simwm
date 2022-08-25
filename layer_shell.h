#pragma once

#include "output.h"
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

struct simwm_layer_surface {
  struct wlr_layer_surface_v1 *layer_surface;

  struct simwm_output *output;

  struct wlr_scene_tree *scene;

  struct wl_listener map;
  struct wl_listener commit;
};

void on_new_layer_surface(struct wl_listener *, void *);
