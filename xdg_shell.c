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
#include <seat.h>

void on_map(struct wl_listener *listener, void *data) {
  struct simwm_xdg_surface *xdg = wl_container_of(listener, xdg, map);
  wlr_log(WLR_INFO, "Mapped window: %s", xdg->toplevel->title);

  focus_view(xdg->view, xdg->toplevel->base->surface);

  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);
  wlr_seat_keyboard_notify_enter(server->seat, xdg->toplevel->base->surface,
                                 keyboard->keycodes, keyboard->num_keycodes,
                                 &keyboard->modifiers);
}

void on_unmap(struct wl_listener *listener, void *data) {
  struct simwm_xdg_surface *xdg = wl_container_of(listener, xdg, unmap);
  wlr_log(WLR_INFO, "Unmapped window: %s", xdg->toplevel->title);
}

void on_destroy(struct wl_listener *listener, void *data) {
  struct simwm_xdg_surface *xdg = wl_container_of(listener, xdg, destroy);
  wlr_log(WLR_INFO, "Destroyed window: %s", xdg->toplevel->title);

  wl_list_remove(&xdg->map.link);
  wl_list_remove(&xdg->unmap.link);
  wl_list_remove(&xdg->destroy.link);
  wl_list_remove(&xdg->ws_link);

  free(xdg->view);
  free(xdg);
}

void begin_interactive(struct simwm_xdg_surface *xdg,
                       enum simwl_cursor_mode mode, uint32_t edges) {
  struct wlr_surface *focused_surface =
      server->seat->pointer_state.focused_surface;

  // Unfocused client requesting mouse drag lul
  if (xdg->toplevel->base->surface !=
      wlr_surface_get_root_surface(focused_surface)) {
    return;
  }

  server->grabbed_view = xdg->view;
  server->cursor_mode = mode;

  if (mode == SIMWL_CURSOR_MOVE) {
    server->grab_x = server->cursor->x - xdg->view->x;
    server->grab_y = server->cursor->y - xdg->view->y;
  } else {
    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(xdg->toplevel->base, &geo_box);
    double border_x = (xdg->view->x + geo_box.x) +
                      ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);

    double border_y = (xdg->view->y + geo_box.y) +
                      ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
  }
}

void on_request_move(struct wl_listener *listener, void *data) {
  struct simwm_xdg_surface *xdg = wl_container_of(listener, xdg, request_move);
  begin_interactive(xdg, SIMWL_CURSOR_MOVE, 0);
}

void on_request_resize(struct wl_listener *listener, void *data) {
  struct simwm_xdg_surface *xdg =
      wl_container_of(listener, xdg, request_resize);
  struct wlr_xdg_toplevel_resize_event *event = data;

  begin_interactive(xdg, SIMWL_CURSOR_RESIZE, event->edges);
}

void on_request_maximize(struct wl_listener *listener, void *data) {
  struct simwm_xdg_surface *xdg =
      wl_container_of(listener, xdg, request_maximize);

  struct wlr_output *output =
      wlr_output_layout_output_at(server->output_layout, 0, 0);

  wlr_xdg_toplevel_set_size(xdg->toplevel, output->width, output->height);

  wlr_scene_node_set_position(&xdg->scene->node, 0, 0);
  wlr_xdg_toplevel_set_maximized(xdg->toplevel, true);
  xdg->view->x = 0;
  xdg->view->y = 0;
  wlr_xdg_surface_schedule_configure(xdg->toplevel->base);
}

void on_new_xdg_surface(struct wl_listener *listener, void *data) {
  struct wlr_xdg_surface *xdg_surface = data;

  /* We must add xdg popups to the scene graph so they get rendered. The
   * wlroots scene graph provides a helper for this, but to use it we must
   * provide the proper parent scene node of the xdg popup. To enable this,
   * we always set the user data field of xdg_surfaces to the corresponding
   * scene node. */
  if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
    return;
  }

  assert(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

  struct simwm_view *view = calloc(1, sizeof(struct simwm_view));
  view->type = SIMWM_VIEW_XDG;

  struct simwm_xdg_surface *xdg = calloc(1, sizeof(struct simwm_xdg_surface));
  view->xdg = xdg;
  xdg->view = view;

  xdg->toplevel = xdg_surface->toplevel;

  struct simwm_output *output = simwm_output_from_wlr_output(
      wlr_output_layout_output_at(server->output_layout, 0, 0));
  xdg->workspace = output->current_workspace;
  wl_list_insert(&xdg->workspace->views, &xdg->ws_link);

  xdg->scene = wlr_scene_xdg_surface_create(xdg->workspace->scene,
                                            view->xdg->toplevel->base);
  xdg->scene->node.data = view;
  xdg_surface->data = xdg->scene;

  xdg->map.notify = on_map;
  wl_signal_add(&xdg_surface->events.map, &xdg->map);

  xdg->unmap.notify = on_unmap;
  wl_signal_add(&xdg_surface->events.unmap, &xdg->unmap);

  xdg->destroy.notify = on_destroy;
  wl_signal_add(&xdg_surface->events.destroy, &xdg->destroy);

  struct wlr_xdg_toplevel *xdg_toplevel = xdg_surface->toplevel;

  // More signals
  xdg->request_move.notify = on_request_move;
  wl_signal_add(&xdg_toplevel->events.request_move, &xdg->request_move);

  xdg->request_resize.notify = on_request_resize;
  wl_signal_add(&xdg_toplevel->events.request_resize, &xdg->request_resize);

  xdg->request_maximize.notify = on_request_maximize;
  wl_signal_add(&xdg_toplevel->events.request_maximize, &xdg->request_maximize);
}
