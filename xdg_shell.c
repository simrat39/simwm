#include <assert.h>
#include <includes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/util/log.h>

#include "layer_shell.h"
#include "output.h"
#include "server.h"
#include "view.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "xdg_shell.h"

void on_map(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, map);
  wlr_log(WLR_INFO, "Mapped window: %s", view->xdg_toplevel->title);

  wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);

  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);
  wlr_seat_keyboard_notify_enter(
      server->seat, view->xdg_toplevel->base->surface, keyboard->keycodes,
      keyboard->num_keycodes, &keyboard->modifiers);
}

void on_unmap(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, unmap);
  wlr_log(WLR_INFO, "Unmapped window: %s", view->xdg_toplevel->title);
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

  struct simwm_view *view = calloc(1, sizeof(struct simwm_view));
  view->type = SIMWM_VIEW_XDG;

  /* We must add xdg popups to the scene graph so they get rendered. The
   * wlroots scene graph provides a helper for this, but to use it we must
   * provide the proper parent scene node of the xdg popup. To enable this,
   * we always set the user data field of xdg_surfaces to the corresponding
   * scene node. */
  if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
    view->scene_tree =
        wlr_scene_xdg_surface_create(server->layers[LAYER_TOP], xdg_surface);

    return;
  }

  assert(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

  view->xdg_toplevel = xdg_surface->toplevel;
  view->scene_tree = wlr_scene_xdg_surface_create(server->layers[LAYER_TILE],
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

void focus_view(struct simwm_view *view, struct wlr_surface *surface) {
  assert(view != NULL);
  assert(view->type == SIMWM_VIEW_XDG);

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

  wlr_scene_node_raise_to_top(&view->scene_tree->node);

  wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);

  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);

  if (keyboard != NULL) {
    wlr_seat_keyboard_notify_enter(
        server->seat, view->xdg_toplevel->base->surface, keyboard->keycodes,
        keyboard->num_keycodes, &keyboard->modifiers);
  }
}
