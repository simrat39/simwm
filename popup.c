#include "layer_shell.h"
#include "xdg-shell-protocol.h"
#include <includes.h>
#include <stdlib.h>

#include <output.h>
#include <popup.h>
#include <server.h>
#include <wayland-util.h>
#include <wlr/util/log.h>

void on_popup_map(struct wl_listener *listener, void *data) {
  struct simwm_popup *popup = wl_container_of(listener, popup, popup_map);
  wlr_log(WLR_INFO, "POPUP map");
}

void on_popup_new_popup(struct wl_listener *listener, void *data) {
  struct simwm_popup *popup = wl_container_of(listener, popup, popup_new_popup);
  struct wlr_xdg_popup *xdg_popup = data;

  create_popup(xdg_popup, popup->parent);

  wlr_log(WLR_INFO, "nested POPUP create");
}

void on_popup_destroy(struct wl_listener *listener, void *data) {
  struct simwm_popup *popup = wl_container_of(listener, popup, popup_destroy);

  wl_list_remove(&popup->popup_map.link);
  wl_list_remove(&popup->popup_destroy.link);
  wl_list_remove(&popup->popup_new_popup.link);

  free(popup);
  wlr_log(WLR_INFO, "POPUP destroy");
}

void unconstrain_popup(struct simwm_popup *popup) {
  int lx, ly;
  wlr_scene_node_coords(&popup->parent->scene_layer_surface->tree->node, &lx,
                        &ly);

  struct wlr_box ox;
  wlr_output_layout_get_box(server->output_layout,
                            popup->parent->output->wlr_output, &ox);

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

void create_popup(struct wlr_xdg_popup *xdg_popup, struct simwm_view *view) {
  struct simwm_popup *popup = calloc(1, sizeof(struct simwm_popup));
  popup->wlr_popup = xdg_popup;
  popup->parent = view;

  popup->scene_tree =
      wlr_scene_xdg_surface_create(view->popup_scene_tree, xdg_popup->base);

  popup->popup_map.notify = on_popup_map;
  wl_signal_add(&xdg_popup->base->events.map, &popup->popup_map);

  popup->popup_destroy.notify = on_popup_destroy;
  wl_signal_add(&xdg_popup->base->events.destroy, &popup->popup_destroy);

  popup->popup_new_popup.notify = on_popup_new_popup;
  wl_signal_add(&xdg_popup->base->events.new_popup, &popup->popup_new_popup);

  unconstrain_popup(popup);
}
