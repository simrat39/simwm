#pragma once

#include "layer_shell.h"
#include "layout.h"
#include <lua.h>
#include <wayland-server-core.h>
#include <wayland-util.h>

enum simwl_cursor_mode {
  SIMWL_CURSOR_PASSTHROUGH,
  SIMWL_CURSOR_MOVE,
  SIMWL_CURSOR_RESIZE,
};

struct simwm_server {
  char *name;

  struct wl_display *wl_display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;
  struct wlr_allocator *allocator;
  struct wlr_output_layout *output_layout;
  struct wlr_scene *scene;

  struct wl_list outputs;
  struct wl_listener new_output;

  struct wl_list views;
  struct wlr_xdg_shell *xdg_shell;
  struct wl_listener new_xdg_surface;

  struct wlr_layer_shell_v1 *layer_shell;
  struct wl_listener new_layer_surface;

  struct wlr_scene_tree *layers[LAYER_COUNT];

  struct simwm_view *grabbed_view;
  double grab_x;
  double grab_y;

  enum simwl_cursor_mode cursor_mode;
  struct wlr_cursor *cursor;
  struct wlr_xcursor_manager *cursor_mgr;
  struct wl_listener cursor_motion;
  struct wl_listener cursor_motion_absolute;
  struct wl_listener cursor_button;
  struct wl_listener cursor_frame;

  struct wl_list keyboards;
  struct wl_list keymaps;

  struct wl_listener new_input;

  struct wlr_seat *seat;
  struct wl_listener request_cursor;
  struct wl_list workspaces;

  struct wl_list layouts;
  struct simwm_layout* current_layout;

  struct lua_State *L;
};

extern struct simwm_server *server;
void simwm_server_init();
