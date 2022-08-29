#include "output.h"
#include "popup.h"
#include "view.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include <assert.h>
#include <includes.h>
#include <layer_shell.h>
#include <server.h>
#include <stdint.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>

void configure_layer(struct simwm_output *output, struct wlr_box *full_area,
                     struct wlr_box *usable_area, struct wlr_scene_tree *tree) {

  struct wlr_scene_node *node;
  wl_list_for_each(node, &tree->children, link) {
    struct simwm_view *view = node->data;

    if (!node) {
      continue;
    }

    assert(view);

    if (view->type == SIMWM_VIEW_LAYER) {
      wlr_scene_layer_surface_v1_configure(view->layer->scene, full_area,
                                           usable_area);
    }
  }
}

void configure_all_layers(struct simwm_output *output) {
  struct wlr_box full_area;
  wlr_output_layout_get_box(server->output_layout, output->wlr_output,
                            &full_area);
  struct wlr_box usable_area = full_area;

  configure_layer(output, &full_area, &usable_area, server->layers[LAYER_BG]);
  configure_layer(output, &full_area, &usable_area,
                  server->layers[LAYER_BOTTOM]);
  configure_layer(output, &full_area, &usable_area, server->layers[LAYER_TOP]);
  configure_layer(output, &full_area, &usable_area,
                  server->layers[LAYER_OVERLAY]);
}

void on_layer_new_popup(struct wl_listener *listener, void *data) {
  struct simwm_layer_surface *layer =
      wl_container_of(listener, layer, new_popup);
  struct wlr_xdg_popup *xdg_popup = data;

  create_popup(xdg_popup, layer->view);
}

void on_layer_surface_map(struct wl_listener *listener, void *data) {
  struct simwm_layer_surface *layer = wl_container_of(listener, layer, map);

  if (layer->scene->layer_surface->current.keyboard_interactive) {
    wlr_seat_keyboard_notify_enter(
        server->seat, layer->scene->layer_surface->surface,
        server->seat->keyboard_state.keyboard->keycodes,
        server->seat->keyboard_state.keyboard->num_keycodes,
        &server->seat->keyboard_state.keyboard->modifiers);

    configure_all_layers(layer->output);
  }
}

void on_layer_surface_unmap(struct wl_listener *listener, void *data) {
  struct simwm_layer_surface *layer = wl_container_of(listener, layer, unmap);
  configure_all_layers(layer->output);
  wlr_log(WLR_INFO, "unmapped");
}

void on_layer_surface_destroy(struct wl_listener *listener, void *data) {
  struct simwm_layer_surface *layer = wl_container_of(listener, layer, destroy);

  wl_list_remove(&layer->map.link);
  wl_list_remove(&layer->unmap.link);
  wl_list_remove(&layer->commit.link);
  wl_list_remove(&layer->destroy.link);

  free(layer->view);
  free(layer);
}

void on_layer_surface_commit(struct wl_listener *listener, void *data) {
  struct simwm_layer_surface *layer = wl_container_of(listener, layer, commit);

  uint32_t commited = layer->scene->layer_surface->current.committed;

  // Layer was changed, reparent node to new layer.
  if (commited & WLR_LAYER_SURFACE_V1_STATE_LAYER) {
    wlr_scene_node_reparent(
        &layer->scene->tree->node,
        server->layers[layer->scene->layer_surface->current.layer]);
  }

  // If any other change was detected, or the mapped state changed, we
  // reconfigure all the layers.
  if (commited || layer->scene->layer_surface->mapped != layer->mapped) {
    layer->mapped = layer->scene->layer_surface->mapped;
    configure_all_layers(layer->output);
  }

  int lx, ly;
  wlr_scene_node_coords(&layer->scene->tree->node, &lx, &ly);
  wlr_scene_node_set_position(&layer->popup_scene->node, lx, ly);
}

void on_new_layer_surface(struct wl_listener *listener, void *data) {
  struct wlr_layer_surface_v1 *layer_surface = data;

  struct simwm_view *view = calloc(1, sizeof(struct simwm_view));
  view->type = SIMWM_VIEW_LAYER;

  struct simwm_layer_surface *layer =
      calloc(1, sizeof(struct simwm_layer_surface));
  view->layer = layer;
  layer->view = view;

  struct wlr_output *wlr_output = layer_surface->output;
  if (wlr_output == NULL) {
    wlr_output = wlr_output_layout_get_center_output(server->output_layout);
  }
  struct simwm_output *output = wlr_output->data;

  layer->output = output;

  layer->scene = wlr_scene_layer_surface_v1_create(
      server->layers[layer_surface->current.layer], layer_surface);
  layer->scene->tree->node.data = view;

  layer->popup_scene = wlr_scene_tree_create(server->layers[LAYER_TOP]);
  layer->popup_scene->node.data = view;

  layer->commit.notify = on_layer_surface_commit;
  wl_signal_add(&layer_surface->surface->events.commit, &layer->commit);

  layer->map.notify = on_layer_surface_map;
  wl_signal_add(&layer_surface->events.map, &layer->map);

  layer->unmap.notify = on_layer_surface_unmap;
  wl_signal_add(&layer_surface->events.unmap, &layer->unmap);

  layer->destroy.notify = on_layer_surface_destroy;
  wl_signal_add(&layer_surface->events.destroy, &layer->destroy);

  layer->new_popup.notify = on_layer_new_popup;
  wl_signal_add(&layer_surface->events.new_popup, &layer->new_popup);

  configure_all_layers(layer->output);
}
