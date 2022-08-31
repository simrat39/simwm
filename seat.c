#include <includes.h>

#include <layer_shell.h>
#include <output.h>
#include <seat.h>
#include <server.h>

#include <stdlib.h>

void seat_kb_notify_enter(struct wlr_surface *surface) {
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);

  if (!keyboard) {
    return;
  }

  wlr_seat_keyboard_notify_enter(server->seat, surface, keyboard->keycodes,
                                 keyboard->num_keycodes, &keyboard->modifiers);
}

struct simwm_workspace *seat_add_workspace(char *name,
                                           struct simwm_output *output) {
  struct simwm_workspace *workspace =
      calloc(1, sizeof(struct simwm_workspace *));

  workspace->name = name;
  workspace->scene = wlr_scene_tree_create(server->layers[LAYER_TILE]);

  wl_list_insert(&output->workspaces, &workspace->link);

  if (!output->current_workspace) {
    output->current_workspace = workspace;
  }

  return workspace;
}

struct simwm_workspace*
seat_get_current_workspace(struct simwm_output *output) {
  return output->current_workspace;
}
