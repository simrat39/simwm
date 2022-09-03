#pragma once

#include "view.h"
#include <includes.h>
#include <output.h>

struct simwm_workspace {
  struct wl_list link;

  char *name;
  struct wlr_scene_tree *scene;
  struct wl_list views;
  struct simwm_view *last_focused_view;
  struct simwm_output *output;
};

void seat_kb_notify_enter(struct wlr_surface *surface);
void seat_kb_notify_clear_focus();

// Workspace
struct simwm_workspace *seat_add_workspace(char *name);
struct simwm_workspace *seat_get_current_workspace(struct simwm_output *output);

void seat_set_current_workspace(struct simwm_output *output, char *name);
