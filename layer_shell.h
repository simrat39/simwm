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

void on_new_layer_surface(struct wl_listener *, void *);
