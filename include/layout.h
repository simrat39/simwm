#pragma once

#include <wayland-util.h>
#include <output.h>
#include <seat.h>

struct simwm_layout {
  struct wl_list link;

  const char *name;
  int arrange;
  int on_new_view;
};

void layout_arrange(struct simwm_workspace *workspace);
void on_new_view(struct simwm_workspace *workspace, struct simwm_view *view);
