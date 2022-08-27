#include "output.h"
#include "popup.h"
#include "view.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include <includes.h>
#include <layer_shell.h>
#include <server.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>

void configure_layers(struct simwm_view *view) {
  struct wlr_box full_area;
  wlr_output_layout_get_box(server->output_layout, view->output->wlr_output,
                            &full_area);
  struct wlr_box usable_area = full_area;

  for (int i = 0; i < LAYER_COUNT; i++) {
    struct simwm_view *layer_scene_view;

    wl_list_for_each(layer_scene_view, &view->output->layer_views[i],
                     layer_link) {

      wlr_scene_layer_surface_v1_configure(
          layer_scene_view->scene_layer_surface, &full_area, &usable_area);
    }
  }

  view->usable_area = usable_area;
}

void on_layer_new_popup(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, new_popup);
  struct wlr_xdg_popup *xdg_popup = data;

  create_popup(xdg_popup, view);
}

void on_layer_surface_map(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, map);
  configure_layers(view);
  wlr_log(WLR_INFO, "mapped");
}

void on_layer_surface_unmap(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, unmap);
  wl_list_remove(&view->layer_link);
  configure_layers(view);
  wlr_log(WLR_INFO, "unmapped");
}

void on_layer_surface_commit(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, commit);

  configure_layers(view);

  int lx, ly;
  wlr_scene_node_coords(&view->scene_layer_surface->tree->node, &lx, &ly);
  wlr_scene_node_set_position(&view->popup_scene_tree->node, lx, ly);
}

void on_new_layer_surface(struct wl_listener *listener, void *data) {
  struct wlr_layer_surface_v1 *layer_surface = data;

  struct simwm_view *view = calloc(1, sizeof(struct simwm_view));
  struct wlr_output *wlr_output = layer_surface->output;
  if (wlr_output == NULL) {
    wlr_output = wlr_output_layout_get_center_output(server->output_layout);
  }
  struct simwm_output *output = wlr_output->data;

  view->type = SIMWM_VIEW_LAYER;
  view->output = output;

  view->scene_layer_surface = wlr_scene_layer_surface_v1_create(
      server->layers[layer_surface->current.layer], layer_surface);
  view->scene_layer_surface->tree->node.data = view;

  view->popup_scene_tree = wlr_scene_tree_create(server->layers[LAYER_TOP]);

  wl_list_insert(&output->layer_views[layer_surface->current.layer],
                 &view->layer_link);

  view->commit.notify = on_layer_surface_commit;
  wl_signal_add(&layer_surface->surface->events.commit, &view->commit);

  view->map.notify = on_layer_surface_map;
  wl_signal_add(&layer_surface->events.map, &view->map);

  view->unmap.notify = on_layer_surface_unmap;
  wl_signal_add(&layer_surface->events.unmap, &view->unmap);

  view->new_popup.notify = on_layer_new_popup;
  wl_signal_add(&layer_surface->events.new_popup, &view->new_popup);

  configure_layers(view);
}
