#include <includes.h>

#include <layer_shell.h>
#include <output.h>
#include <seat.h>
#include <server.h>

#include <stdlib.h>
#include <string.h>
#include <wayland-util.h>
#include <wlr/util/log.h>

void seat_kb_notify_enter(struct wlr_surface *surface) {
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);

  if (!keyboard) {
    return;
  }

  wlr_seat_keyboard_notify_enter(server->seat, surface, keyboard->keycodes,
                                 keyboard->num_keycodes, &keyboard->modifiers);
}

struct simwm_workspace *seat_add_workspace(char *name) {
  struct simwm_workspace *workspace =
      calloc(1, sizeof(struct simwm_workspace *));

  workspace->name = name;
  workspace->scene = wlr_scene_tree_create(server->layers[LAYER_TILE]);
  wl_list_init(&workspace->views);

  wl_list_insert(&server->workspaces, &workspace->link);

  return workspace;
}

struct simwm_workspace *
seat_get_current_workspace(struct simwm_output *output) {
  return output->current_workspace;
}

struct simwm_workspace *seat_get_workspace_from_name(char *name) {
  struct simwm_workspace *ws;

  wl_list_for_each(ws, &server->workspaces, link) {
    if (strcmp(name, ws->name) == 0) {
      return ws;
    }
  }

  return NULL;
}

void seat_set_current_workspace(struct simwm_output *output, char *name) {
  struct simwm_workspace *ws = seat_get_workspace_from_name(name);
  if (!ws) {
    wlr_log(WLR_INFO, "WS %s not found", name);
    return;
  }

  if (output->current_workspace) {
    if (strcmp(output->current_workspace->name, name) == 0) {
      wlr_log(WLR_INFO, "WS %s is already current", name);
      return;
    }

    wlr_scene_node_set_enabled(&output->current_workspace->scene->node, false);
  }

  wlr_scene_node_set_enabled(&ws->scene->node, true);
  wlr_scene_node_raise_to_top(&ws->scene->node);

  output->current_workspace = ws;
}
