#pragma once

#include <wayland-util.h>
#include <output.h>
#include <seat.h>

struct simwm_layout {
  struct wl_list link;

  const char *name;
  int arrange;
};

void layout_arrange(struct simwm_workspace *workspace);
