#pragma once

#include "layer_shell.h"
#include "popup.h"
#include "xdg_shell.h"
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>

enum simwm_view_type { SIMWM_VIEW_XDG, SIMWM_VIEW_LAYER, SIMWM_VIEW_POPUP };

struct simwm_view {
  enum simwm_view_type type;
  struct wl_list link;

  union {
    struct simwm_xdg_surface *xdg;
    struct simwm_layer_surface *layer;
    struct simwm_popup *popup;
  };

  int x, y;
  int width, height;
};

struct simwm_view *view_at(double lx, double ly, struct wlr_surface **surface,
                           double *sx, double *sy);
void focus_view(struct simwm_view *view, struct wlr_surface *surface);
