#include "layer_shell.h"
#include "xdg-shell-protocol.h"
#include <stdlib.h>

#include <includes.h>
#include <output.h>
#include <popup.h>
#include <server.h>
#include <wayland-util.h>
#include <wlr/util/log.h>

void on_popup_new_popup(struct wl_listener *listener, void *data) {
  struct simwm_popup *popup = wl_container_of(listener, popup, popup_new_popup);
  struct wlr_xdg_popup *xdg_popup = data;

  create_popup(xdg_popup, popup->parent);
}

void on_popup_destroy(struct wl_listener *listener, void *data) {
  struct simwm_popup *popup = wl_container_of(listener, popup, popup_destroy);

  wl_list_remove(&popup->popup_destroy.link);
  wl_list_remove(&popup->popup_new_popup.link);

  free(popup->view);
  free(popup);
}

void unconstrain_popup(struct simwm_popup *popup) {
  int lx, ly;
  wlr_scene_node_coords(&popup->parent->layer->scene->tree->node, &lx, &ly);

  struct wlr_box ox;
  wlr_output_layout_get_box(server->output_layout,
                            popup->parent->layer->output->wlr_output, &ox);

  // the output box expressed in the coordinate system of the toplevel parent
  // of the popup
  struct wlr_box output_toplevel_sx_box = {
      .x = ox.x - lx,
      .y = ox.y - ly,
      .width = ox.width,
      .height = ox.height,
  };

  wlr_xdg_popup_unconstrain_from_box(popup->wlr_popup, &output_toplevel_sx_box);
}

void create_popup(struct wlr_xdg_popup *xdg_popup, struct simwm_view *parent) {
  struct simwm_view *view = calloc(1, sizeof(struct simwm_view));
  view->type = SIMWM_VIEW_POPUP;

  struct simwm_popup *popup = calloc(1, sizeof(struct simwm_popup));
  popup->view = view;

  view->popup = popup;

  popup->wlr_popup = xdg_popup;
  popup->parent = parent;

  popup->scene =
      wlr_scene_xdg_surface_create(parent->layer->popup_scene, xdg_popup->base);
  popup->scene->node.data = view;

  popup->popup_destroy.notify = on_popup_destroy;
  wl_signal_add(&xdg_popup->base->events.destroy, &popup->popup_destroy);

  popup->popup_new_popup.notify = on_popup_new_popup;
  wl_signal_add(&xdg_popup->base->events.new_popup, &popup->popup_new_popup);

  unconstrain_popup(popup);
}
