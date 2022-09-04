#include "layer_shell.h"
#include <assert.h>
#include <includes.h>

#include <seat.h>
#include <server.h>
#include <view.h>

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

  switch (view->type) {
  case SIMWM_VIEW_XDG:
    wlr_scene_node_raise_to_top(&view->xdg->scene->node);

    wlr_xdg_toplevel_set_activated(view->xdg->toplevel, true);
    seat_kb_notify_enter(view->xdg->toplevel->base->surface);
    view->xdg->workspace->last_focused_view = view;
    server->focused_output = view->xdg->workspace->output;
    break;

  case SIMWM_VIEW_LAYER:
    if (view->layer->scene->layer_surface->current.keyboard_interactive) {
      seat_kb_notify_enter(view->layer->scene->layer_surface->surface);
    }
    break;

  case SIMWM_VIEW_POPUP:
    wlr_scene_node_raise_to_top(&view->popup->scene->node);
    seat_kb_notify_enter(view->popup->wlr_popup->base->surface);
    break;
  }
}
