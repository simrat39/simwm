#include <assert.h>
#include <includes.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-util.h>
#include <wlr/util/log.h>

#include "output.h"
#include "server.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "xdg_shell.h"

void set_layout() {
  struct wlr_output *output =
      wlr_output_layout_get_center_output(server->output_layout);

  int master_width = output->width * 0.8;
  int master_height = output->height;

  if (server->master == NULL) {
    return;
  }

  wlr_log(WLR_ERROR, "%s", server->master->xdg_toplevel->app_id);
  wlr_xdg_toplevel_set_size(server->master->xdg_toplevel, master_width,
                            master_height);

  wlr_scene_node_set_position(&server->master->scene_tree->node, 0, 0);

  int current_child_count = 0;
  int total_child_count = wl_list_length(&server->children);

  if (total_child_count == 0) {
    return;
  }

  int child_height = master_height / total_child_count;
  int child_width = output->width * 0.2;

  int x = master_width;

  struct simwm_view *child_view;
  wl_list_for_each_reverse(child_view, &server->children, layout_link) {
    int y = current_child_count * child_height;

    wlr_xdg_toplevel_set_size(child_view->xdg_toplevel, child_width,
                              child_height);
    wlr_scene_node_set_position(&child_view->scene_tree->node, x, y);

    child_view->x = x;
    child_view->y = y;

    current_child_count++;
  }
}

void on_map(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, map);
  wlr_log(WLR_INFO, "Mapped window: %s", view->xdg_toplevel->title);

  wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);

  /* wl_list_insert(&server->views, &view->link); */

  if (server->master == NULL) {
    server->master = view;
  } else {
    wl_list_insert(&server->children, &view->layout_link);
  }

  set_layout();

  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);
  wlr_seat_keyboard_notify_enter(
      server->seat, view->xdg_toplevel->base->surface, keyboard->keycodes,
      keyboard->num_keycodes, &keyboard->modifiers);
}

void on_unmap(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, unmap);
  wlr_log(WLR_INFO, "Unmapped window: %s", view->xdg_toplevel->title);

  if (view == server->master) {
    int child_count = wl_list_length(&server->children);

    if (child_count == 0) {
      server->master = NULL;
    } else {
      wlr_log(WLR_INFO, "hi");
      struct simwm_view *first_child =
          wl_container_of(server->children.next, first_child, layout_link);

      wl_list_remove(&first_child->layout_link);
      server->master = first_child;
    }
  } else {
    wl_list_remove(&view->layout_link);
  }

  set_layout();
}

void on_destroy(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, destroy);
  wlr_log(WLR_INFO, "Destroyed window: %s", view->xdg_toplevel->title);

  wl_list_remove(&view->map.link);
  wl_list_remove(&view->unmap.link);
  wl_list_remove(&view->destroy.link);

  free(view);
}

void begin_interactive(struct simwm_view *view, enum simwl_cursor_mode mode,
                       uint32_t edges) {
  struct wlr_surface *focused_surface =
      server->seat->pointer_state.focused_surface;

  // Unfocused client requesting mouse drag lul
  if (view->xdg_toplevel->base->surface !=
      wlr_surface_get_root_surface(focused_surface)) {
    return;
  }

  server->grabbed_view = view;
  server->cursor_mode = mode;

  if (mode == SIMWL_CURSOR_MOVE) {
    server->grab_x = server->cursor->x - view->x;
    server->grab_y = server->cursor->y - view->y;
  } else {
    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo_box);
    double border_x =
        (view->x + geo_box.x) + ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);

    double border_y = (view->y + geo_box.y) +
                      ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
  }
}

void on_request_move(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, request_move);
  begin_interactive(view, SIMWL_CURSOR_MOVE, 0);
}

void on_request_resize(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, request_resize);
  struct wlr_xdg_toplevel_resize_event *event = data;

  begin_interactive(view, SIMWL_CURSOR_RESIZE, event->edges);
}

void on_request_maximize(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, request_maximize);

  struct wlr_output *output =
      wlr_output_layout_output_at(server->output_layout, 0, 0);

  wlr_xdg_toplevel_set_size(view->xdg_toplevel, output->width, output->height);

  wlr_scene_node_set_position(&view->scene_tree->node, 0, 0);
  wlr_xdg_toplevel_set_maximized(view->xdg_toplevel, true);
  view->x = 0;
  view->y = 0;
  wlr_xdg_surface_schedule_configure(view->xdg_toplevel->base);
}

void on_new_xdg_surface(struct wl_listener *listener, void *data) {
  struct wlr_xdg_surface *xdg_surface = data;

  /* We must add xdg popups to the scene graph so they get rendered. The
   * wlroots scene graph provides a helper for this, but to use it we must
   * provide the proper parent scene node of the xdg popup. To enable this,
   * we always set the user data field of xdg_surfaces to the corresponding
   * scene node. */
  if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
    struct wlr_xdg_surface *parent =
        wlr_xdg_surface_from_wlr_surface(xdg_surface->popup->parent);
    struct wlr_scene_tree *parent_tree = parent->data;
    xdg_surface->data = wlr_scene_xdg_surface_create(parent_tree, xdg_surface);
    return;
  }

  assert(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

  struct simwm_view *view = calloc(1, sizeof(struct simwm_view));

  struct wlr_output *output =
      wlr_output_layout_get_center_output(server->output_layout);
  struct simwm_output *simwm_output = simwm_output_from_wlr_output(output);

  view->xdg_toplevel = xdg_surface->toplevel;
  view->scene_tree = wlr_scene_xdg_surface_create(
      simwm_output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM],
      view->xdg_toplevel->base);
  view->scene_tree->node.data = view;
  xdg_surface->data = view->scene_tree;

  view->map.notify = on_map;
  wl_signal_add(&xdg_surface->events.map, &view->map);

  view->unmap.notify = on_unmap;
  wl_signal_add(&xdg_surface->events.unmap, &view->unmap);

  view->destroy.notify = on_destroy;
  wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

  struct wlr_xdg_toplevel *xdg_toplevel = xdg_surface->toplevel;

  // More signals
  view->request_move.notify = on_request_move;
  wl_signal_add(&xdg_toplevel->events.request_move, &view->request_move);

  view->request_resize.notify = on_request_resize;
  wl_signal_add(&xdg_toplevel->events.request_resize, &view->request_resize);

  view->request_maximize.notify = on_request_maximize;
  wl_signal_add(&xdg_toplevel->events.request_maximize,
                &view->request_maximize);
}

struct simwm_view *view_at(double lx, double ly, struct wlr_surface **surface,
                           double *sx, double *sy) {
  struct wlr_scene_node *node =
      wlr_scene_node_at(&server->scene->tree.node, lx, ly, sx, sy);

  if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
    return NULL;
  }

  struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
  struct wlr_scene_surface *scene_surface =
      wlr_scene_surface_from_buffer(scene_buffer);

  if (!scene_surface) {
    return NULL;
  }

  *surface = scene_surface->surface;

  struct wlr_scene_tree *tree = node->parent;

  while (tree != NULL && tree->node.data == NULL) {
    tree = tree->node.parent;
  }

  return tree->node.data;
}

void focus_view(struct simwm_view *view, struct wlr_surface *surface) {
  if (view == NULL) {
    return;
  }

  struct wlr_surface *prev_surface =
      server->seat->keyboard_state.focused_surface;

  if (prev_surface == surface) {
    return;
  }

  if (prev_surface) {
    struct wlr_xdg_surface *prev_xdg_surface =
        wlr_xdg_surface_from_wlr_surface(prev_surface);
    assert(prev_xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
    wlr_xdg_toplevel_set_activated(prev_xdg_surface->toplevel, false);
  }

  wlr_scene_node_raise_to_top(&view->scene_tree->node);

  wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);

  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);

  if (keyboard != NULL) {
    wlr_seat_keyboard_notify_enter(
        server->seat, view->xdg_toplevel->base->surface, keyboard->keycodes,
        keyboard->num_keycodes, &keyboard->modifiers);
  }
}
