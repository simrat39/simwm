#include "output.h"
#include "view.h"
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
  struct simwm_view *simwm_layer = wl_container_of(listener, simwm_layer, map);

  wlr_log(WLR_INFO, "mapped");
}

enum simwm_anchor parse_anchor(int anchor) {
  int top = anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
  int left = anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
  int right = anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
  int bottom = anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;

  int all = left && right && top && bottom;

  if (all)
    return SIMWM_ANCHOR_ALL;

  if (top) {
    if (left) {
      return SIMWM_ANCHOR_TOP_LEFT;
    }
    if (right) {
      return SIMWM_ANCHOR_TOP_RIGHT;
    }
    return SIMWM_ANCHOR_TOP;
  }

  if (bottom) {
    if (left) {
      return SIMWM_ANCHOR_BOTTOM_LEFT;
    }
    if (right) {
      return SIMWM_ANCHOR_BOTTOM_RIGHT;
    }
    return SIMWM_ANCHOR_BOTTOM;
  }

  if (left) {
    return SIMWM_ANCHOR_LEFT;
  }

  if (right) {
    return SIMWM_ANCHOR_RIGHT;
  }

  return SIMWM_ANCHOR_NONE;
}

void on_layer_surface_commit(struct wl_listener *listener, void *data) {
  struct simwm_view *view = wl_container_of(listener, view, commit);

  if (view->layer_surface->current.committed == 0) {
    return;
  }

  int monitor_width, monitor_height;
  wlr_output_effective_resolution(view->output, &monitor_width,
                                  &monitor_height);
  const int wlr_anchor = view->layer_surface->pending.anchor;

  int desired_width = view->layer_surface->pending.desired_width;
  int desired_height = view->layer_surface->pending.desired_height;

  int configured_width = desired_width;
  int configured_height = desired_height;

  int pos_x = 0;
  int pos_y = 0;

  enum simwm_anchor anchor = parse_anchor(wlr_anchor);
  wlr_log(WLR_INFO, "sim anchor %d", anchor);
  wlr_log(WLR_INFO, "wlr anchor %d", wlr_anchor);
  wlr_log(WLR_INFO, "test anchor b %d",
          wlr_anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
  wlr_log(WLR_INFO, "test anchor l %d",
          wlr_anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
  wlr_log(WLR_INFO, "test anchor r %d",
          wlr_anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
  switch (anchor) {
  case SIMWM_ANCHOR_ALL:
    pos_x = monitor_width / 2;
    pos_y = monitor_height / 2;

    if (desired_height == 0) {
      pos_y = 0;
      configured_height = monitor_height;
    }

    if (desired_width == 0) {
      pos_x = 0;
      configured_width = monitor_width;
    }
    break;
  case SIMWM_ANCHOR_TOP:
    pos_x = 0;
    pos_y = 0;

    if (desired_width == 0) {
      configured_width = monitor_width;
    }
    break;
  case SIMWM_ANCHOR_BOTTOM:
    pos_x = 0;
    pos_y = monitor_height - configured_height;

    if (desired_width == 0) {
      configured_width = monitor_width;
    }
    break;
  case SIMWM_ANCHOR_LEFT:
    pos_x = 0;
    pos_y = monitor_height / 2;

    if (desired_height == 0) {
      pos_y = 0;
      configured_height = monitor_height;
    }
    break;
  case SIMWM_ANCHOR_RIGHT:
    pos_x = monitor_width - configured_width;
    pos_y = monitor_height / 2;

    if (desired_height == 0) {
      pos_y = 0;
      configured_height = monitor_height;
    }
    break;
  case SIMWM_ANCHOR_TOP_LEFT:
    pos_x = 0;
    pos_y = 0;
    break;
  case SIMWM_ANCHOR_TOP_RIGHT:
    pos_x = monitor_width - configured_width;
    pos_y = 0;
    break;
  case SIMWM_ANCHOR_BOTTOM_LEFT:
    pos_x = 0;
    pos_y = monitor_height - configured_height;
    break;
  case SIMWM_ANCHOR_BOTTOM_RIGHT:
    pos_x = monitor_width - configured_width;
    pos_y = monitor_height - configured_height;
    break;
  case SIMWM_ANCHOR_NONE:
    configured_width = 0;
    configured_height = 0;
    break;
  }

  wlr_layer_surface_v1_configure(view->layer_surface, configured_width,
                                 configured_height);
  wlr_scene_node_set_position(&view->scene_tree->node, pos_x, pos_y);
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

  struct simwm_view *view = calloc(1, sizeof(struct simwm_view));
  struct wlr_output *wlr_output = layer_surface->output;
  if (wlr_output == NULL) {
    wlr_output = wlr_output_layout_get_center_output(server->output_layout);
  }

  view->type = SIMWM_VIEW_LAYER;
  view->layer_surface = layer_surface;
  view->output = wlr_output;

  view->commit.notify = on_layer_surface_commit;
  wl_signal_add(&view->layer_surface->surface->events.commit, &view->commit);

  view->map.notify = on_layer_surface_map;
  wl_signal_add(&view->layer_surface->events.map, &view->map);

  view->scene_tree = wlr_scene_subsurface_tree_create(
      server->layers[layer_surface->pending.layer], layer_surface->surface);
  view->scene_tree->node.data = view;
}
