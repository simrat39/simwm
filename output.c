#include <stdio.h>
#include <stdlib.h>

#include <includes.h>
#include <output.h>
#include <seat.h>
#include <server.h>

void on_output_frame(struct wl_listener *listener, void *data) {
  struct simwm_output *output = wl_container_of(listener, output, frame);
  struct wlr_scene *scene = server->scene;

  struct wlr_scene_output *scene_output =
      wlr_scene_get_scene_output(scene, output->wlr_output);

  wlr_scene_output_commit(scene_output);

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  wlr_scene_output_send_frame_done(scene_output, &now);
}

void server_new_output(struct wl_listener *listener, void *data) {
  struct wlr_output *wlr_output = data;

  wlr_output_init_render(wlr_output, server->allocator, server->renderer);
  wlr_log(WLR_ERROR, "%d", wlr_output->refresh);

  if (!wl_list_empty(&wlr_output->modes)) {
    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    wlr_output_set_mode(wlr_output, mode);
    wlr_output_enable(wlr_output, true);
    if (!wlr_output_commit(wlr_output)) {
      return;
    }
  }

  struct simwm_output *output = calloc(1, sizeof(struct simwm_output));
  output->wlr_output = wlr_output;
  wlr_output->data = output;

  output->frame.notify = on_output_frame;
  wl_signal_add(&wlr_output->events.frame, &output->frame);

  for (int i = 0; i < LAYER_COUNT; i++) {
    wl_list_init(&output->layer_views[i]);
  }

  /* output->destroy.notify = output_destroy; */
  /* wl_signal_add(&wlr_output->events.destroy, &output->destroy); */

  wl_list_insert(&server->outputs, &output->link);
  wlr_output_layout_add_auto(server->output_layout, wlr_output);
}

struct simwm_output *simwm_output_from_wlr_output(struct wlr_output *outpout) {
  return outpout->data;
}
