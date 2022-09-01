#include "input.h"
#include "layer_shell.h"
#include "xdg_shell.h"
#include <cursor.h>
#include <includes.h>
#include <output.h>
#include <server.h>
#include <seat.h>
#include <stdlib.h>
#include <wayland-server-core.h>

void simwm_server_init() {
  server = calloc(1, sizeof(struct simwm_server));

  // The main wayland display. Mainly handles the client from the unix socket.
  server->wl_display = wl_display_create();

  // The backend is a wlroots feature which abstracts the underlying input and
  // output hardware-> wlr_backend_autocreate automatically selects a suitable
  // backend (for eg X11)->
  server->backend = wlr_backend_autocreate(server->wl_display);
  if (server->backend == NULL) {
    wlr_log(WLR_ERROR, "Failed to create wlr_backend");
  }

  // The renderer, for example Pixman, Vulkan etc->
  server->renderer = wlr_renderer_autocreate(server->backend);
  if (server->renderer == NULL) {
    wlr_log(WLR_ERROR, "Failed to create wlr_renderer");
  }

  // Hook up the renderer and the wayland display->
  wlr_renderer_init_wl_display(server->renderer, server->wl_display);

  // Allocator acts as a bridge between the backend and the renderer-
  // Hanldes the buffer creation which allows rendering-
  server->allocator =
      wlr_allocator_autocreate(server->backend, server->renderer);

  // Compositor used for clients to allocate surfaces->
  struct wlr_compositor *compositor =
      wlr_compositor_create(server->wl_display, server->renderer);

  wlr_subcompositor_create(server->wl_display);

  // Data device manager handles the clipboard->
  wlr_data_device_manager_create(server->wl_display);

  // Responsible for handling the arrangement of physical monitors/screens->
  server->output_layout = wlr_output_layout_create();

  // Create a list of outputs (screens/monitors)->
  wl_list_init(&server->outputs);

  // Set up a listener for when new outputs are connected->
  server->new_output.notify = server_new_output;
  wl_signal_add(&server->backend->events.new_output, &server->new_output);

  wlr_xdg_output_manager_v1_create(server->wl_display, server->output_layout);

  // Scene Graph. wlroots abstraction responsible for rendering and damage
  // tracking
  server->scene = wlr_scene_create();
  wlr_scene_attach_output_layout(server->scene, server->output_layout);

  // Setup xdg-shell which is a wayland protocol to manage application windows.
  wl_list_init(&server->views);
  server->xdg_shell = wlr_xdg_shell_create(server->wl_display, 4);
  server->new_xdg_surface.notify = on_new_xdg_surface;
  wl_signal_add(&server->xdg_shell->events.new_surface,
                &server->new_xdg_surface);

  server->layer_shell = wlr_layer_shell_v1_create(server->wl_display);
  server->new_layer_surface.notify = on_new_layer_surface;
  wl_signal_add(&server->layer_shell->events.new_surface,
                &server->new_layer_surface);

  server->layers[LAYER_BG] = wlr_scene_tree_create(&server->scene->tree);
  server->layers[LAYER_BOTTOM] = wlr_scene_tree_create(&server->scene->tree);
  server->layers[LAYER_TILE] = wlr_scene_tree_create(&server->scene->tree);
  server->layers[LAYER_TOP] = wlr_scene_tree_create(&server->scene->tree);
  server->layers[LAYER_OVERLAY] = wlr_scene_tree_create(&server->scene->tree);

  cursor_init();
  cursor_events_init();

  wl_list_init(&server->keyboards);
  server->new_input.notify = on_new_input;
  wl_signal_add(&server->backend->events.new_input, &server->new_input);

  server->seat = wlr_seat_create(server->wl_display, "seat0");

  wl_list_init(&server->workspaces);
  seat_add_workspace("1");
  seat_add_workspace("2");

  server->request_cursor.notify = on_seat_request_cursor;
  wl_signal_add(&server->seat->events.request_set_cursor,
                &server->request_cursor);
}
