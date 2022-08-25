#include "output.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include <includes.h>
#include <layer_shell.h>
#include <server.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/util/log.h>

void on_layer_surface_map(struct wl_listener *listener, void *data) {
  struct simwm_layer_surface *simwm_layer =
      wl_container_of(listener, simwm_layer, map);

  wlr_log(WLR_INFO, "mapped");
}

enum simwm_anchor parse_anchor(int anchor) {
  int top = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
  int left = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
  int right = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
  int bottom = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;

  int all = left & right & top & bottom;

  if (anchor & all)
    return SIMWM_ANCHOR_ALL;

  int both_vert = top & bottom;
  if (anchor & both_vert)
    return SIMWM_ANCHOR_VERTICAL;

  int both_horiz = left & right;
  if (anchor & both_horiz)
    return SIMWM_ANCHOR_HORIZONTAL;

  if (anchor & top) {
    if (anchor & left) {
      return SIMWM_ANCHOR_TOP_LEFT;
    }
    if (anchor & right) {
      return SIMWM_ANCHOR_TOP_RIGHT;
    }
  }

  if (anchor & bottom) {
    if (anchor & left) {
      return SIMWM_ANCHOR_BOTTOM_LEFT;
    }
    if (anchor & right) {
      return SIMWM_ANCHOR_BOTTOM_RIGHT;
    }
  }

  return SIMWM_ANCHOR_NONE;
}

void on_layer_surface_commit(struct wl_listener *listener, void *data) {
  struct simwm_layer_surface *layer_surface =
      wl_container_of(listener, layer_surface, commit);

  if (layer_surface->layer_surface->current.committed == 0) {
    return;
  }

  struct wlr_output *output = layer_surface->output;
  if (output == NULL) {
    output = wlr_output_layout_get_center_output(server->output_layout);
  }

  int monitor_width, monitor_height;
  wlr_output_effective_resolution(output, &monitor_width, &monitor_height);
  const int wlr_anchor = layer_surface->layer_surface->pending.anchor;

  int desired_width = layer_surface->layer_surface->pending.desired_width;
  int desired_height = layer_surface->layer_surface->pending.desired_height;

  int configured_width;
  int configured_height;

  enum simwm_anchor anchor = parse_anchor(anchor);
  switch (anchor) {
  case SIMWM_ANCHOR_ALL:
    configured_height = monitor_height;
    configured_width = monitor_width;
    break;
  case SIMWM_ANCHOR_HORIZONTAL:
    configured_height = desired_height;
    configured_width = monitor_width;
    break;
  case SIMWM_ANCHOR_VERTICAL:
    configured_height = monitor_width;
    configured_width = desired_width;
    break;
  case SIMWM_ANCHOR_TOP_LEFT:
  case SIMWM_ANCHOR_TOP_RIGHT:
  case SIMWM_ANCHOR_BOTTOM_LEFT:
  case SIMWM_ANCHOR_BOTTOM_RIGHT:
    configured_height = desired_height;
    configured_width = desired_width;
    break;
  case SIMWM_ANCHOR_NONE:
    break;
  }

  wlr_layer_surface_v1_configure(layer_surface->layer_surface, configured_width,
                                 configured_height);
}

void arrange_layers() {
  wlr_scene_node_lower_to_bottom(
      &server->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND]->node);

  wlr_scene_node_raise_to_top(
      &server->layers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY]->node);

  wlr_scene_node_place_above(
      &server->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM]->node,
      &server->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND]->node);

  wlr_scene_node_place_above(
      &server->layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP]->node,
      &server->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM]->node);
}

void on_new_layer_surface(struct wl_listener *listener, void *data) {
  struct wlr_layer_surface_v1 *layer_surface = data;

  wlr_log(
      WLR_INFO,
      "new layer surface: namespace %s layer %d anchor %" PRIu32
      " size %" PRIu32 "x%" PRIu32 " margin %" PRIu32 ",%" PRIu32 ",%" PRIu32
      ",%" PRIu32 ",",
      layer_surface->namespace, layer_surface->pending.layer,
      layer_surface->pending.anchor, layer_surface->pending.desired_width,
      layer_surface->pending.desired_height, layer_surface->pending.margin.top,
      layer_surface->pending.margin.right, layer_surface->pending.margin.bottom,
      layer_surface->pending.margin.left);

  struct simwm_layer_surface *simwm_layer =
      calloc(1, sizeof(struct simwm_layer_surface));
  simwm_layer->layer_surface = layer_surface;
  simwm_layer->output = layer_surface->output;

  simwm_layer->commit.notify = on_layer_surface_commit;
  wl_signal_add(&simwm_layer->layer_surface->surface->events.commit,
                &simwm_layer->commit);

  simwm_layer->map.notify = on_layer_surface_map;
  wl_signal_add(&simwm_layer->layer_surface->events.map, &simwm_layer->map);

  wlr_scene_subsurface_tree_create(server->layers[layer_surface->pending.layer],
                                   layer_surface->surface);

  arrange_layers();
}
