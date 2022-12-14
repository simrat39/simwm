#include <includes.h>

#include <server.h>

#include <luaS/luaS.h>
#include <stdlib.h>
#include <unistd.h>
#include <wlr/util/log.h>

struct simwm_server *server;

int main() {
  // Initialize wlr logging
  wlr_log_init(WLR_INFO, NULL);

  simwm_server_init();

  // Unix domain socket over which wayland communicates.
  const char *socket = wl_display_add_socket_auto(server->wl_display);
  if (!socket) {
    wlr_backend_destroy(server->backend);
  }

  // Start the backend. This will enumerate outputs and inputs, become the DRM
  // master, etc
  if (!wlr_backend_start(server->backend)) {
    // Cleanup
    wlr_backend_destroy(server->backend);
    wl_display_destroy(server->wl_display);
  }

  luaS_doconfig();

  /* Set the WAYLAND_DISPLAY environment variable to our socket and run the
   * startup command if requested. */
  setenv("WAYLAND_DISPLAY", socket, true);

  // Run the wayland display event loop.
  wl_display_run(server->wl_display);

  // Cleanup
  wl_display_destroy_clients(server->wl_display);
  wl_display_destroy(server->wl_display);

  free(server);
}
