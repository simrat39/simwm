#pragma once

#include "view.h"
#include <wayland-server-core.h>
#include <wlr/types/wlr_xdg_shell.h>

struct simwm_popup {
  struct wl_listener popup_map;
  struct wl_listener popup_destroy;

  struct simwm_view *parent;
  struct wlr_xdg_popup *xdg_popup;

  struct wlr_scene_tree *scene_tree;
};

void create_popup(struct wlr_xdg_popup *xdg_popup, struct simwm_view *parent);