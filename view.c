#include "layer_shell.h"
#include <assert.h>
#include <includes.h>

#include <server.h>
#include <view.h>
#include <wlr/util/log.h>

struct simwm_view *view_at(double lx, double ly, struct wlr_surface **surface,
                           double *sx, double *sy) {

  struct wlr_scene_tree *tree = &server->scene->tree;
  struct wlr_scene_node *node = wlr_scene_node_at(&tree->node, lx, ly, sx, sy);
  if (!node || node->type != WLR_SCENE_NODE_BUFFER) {
    return NULL;
  }

  struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
  struct wlr_scene_surface *scene_surface =
      wlr_scene_surface_from_buffer(scene_buffer);
  if (!scene_surface) {
    return NULL;
  }

  *surface = scene_surface->surface;

  struct wlr_scene_tree *parent = node->parent;

  while (parent && !parent->node.data) {
    parent = parent->node.parent;
  }

  return parent->node.data;
}

void focus_view(struct simwm_view *view, struct wlr_surface *surface) {
  assert(view);

  struct wlr_surface *prev_surface =
      server->seat->keyboard_state.focused_surface;

  if (prev_surface == surface) {
    return;
  }

  if (prev_surface && wlr_surface_is_xdg_surface(prev_surface)) {
    struct wlr_xdg_surface *prev_xdg_surface =
        wlr_xdg_surface_from_wlr_surface(prev_surface);
    assert(prev_xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
    wlr_xdg_toplevel_set_activated(prev_xdg_surface->toplevel, false);
  }

  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);

  switch (view->type) {
  case SIMWM_VIEW_XDG:
    wlr_log(WLR_INFO, "HERE LOL XDG");
    wlr_scene_node_raise_to_top(&view->xdg->scene->node);

    wlr_xdg_toplevel_set_activated(view->xdg->toplevel, true);
    if (keyboard != NULL) {
      wlr_seat_keyboard_notify_enter(
          server->seat, view->xdg->toplevel->base->surface, keyboard->keycodes,
          keyboard->num_keycodes, &keyboard->modifiers);
    }
    break;
  case SIMWM_VIEW_LAYER:
    if (keyboard != NULL &&
        view->layer->scene->layer_surface->current.keyboard_interactive) {
      wlr_seat_keyboard_notify_enter(
          server->seat, view->layer->scene->layer_surface->surface,
          keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
    }
    break;
  case SIMWM_VIEW_POPUP:
    wlr_log(WLR_INFO, "HERE LOL POPUP");

    wlr_scene_node_raise_to_top(&view->popup->scene->node);
    if (keyboard != NULL) {
      wlr_seat_keyboard_notify_enter(
          server->seat, view->popup->wlr_popup->base->surface,
          keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
    }

    break;
  }
}
