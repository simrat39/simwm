#include <includes.h>
#include <output.h>

struct simwm_workspace {
  struct wl_list link;

  char *name;
  struct wlr_scene_tree *scene;
};

void seat_kb_notify_enter(struct wlr_surface *surface);

// Workspace
struct simwm_workspace *seat_add_workspace(char *name,
                                           struct simwm_output *output);
struct simwm_workspace *
seat_get_current_workspace(struct simwm_output *output);
