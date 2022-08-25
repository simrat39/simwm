#pragma once

#include "output.h"
#include <wayland-server-core.h>

struct simwm_layer_surface {
  struct wlr_layer_surface_v1 *layer_surface;

  struct simwm_output *output;

  struct wlr_scene_tree *scene;

  struct wl_listener map;
  struct wl_listener commit;
};

void on_new_layer_surface(struct wl_listener *, void *);
