#pragma once

#include <output.h>
#include <seat.h>
#include <wayland-util.h>
#include <xdg_shell.h>

struct simwm_layout {
  struct wl_list link;

  const char *name;
  int arrange;
  int on_new_view;
  int on_view_close;
};

void layout_arrange(struct simwm_workspace *workspace);
void on_new_window(struct simwm_workspace *workspace,
                   struct simwm_xdg_surface *window);
void on_window_close(struct simwm_workspace *workspace,
                     struct simwm_xdg_surface *window);
