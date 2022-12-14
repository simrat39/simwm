#pragma once

#include "layer_shell.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include <wayland-server-core.h>
#include <wayland-util.h>

struct simwm_output {
  struct wl_list link;

  struct wlr_output *wlr_output;
  struct wl_listener frame;

  struct simwm_workspace *current_workspace;
};

void server_new_output(struct wl_listener *, void *);
struct simwm_output *simwm_output_from_wlr_output(struct wlr_output *outpout);
