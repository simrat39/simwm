#include <includes.h>

#include <cursor.h>
#include <server.h>
#include <view.h>

#include <stdint.h>
#include <wayland-util.h>
#include <wlr/util/log.h>

void on_cursor_frame(struct wl_listener *listener, void *data) {
  /* This event is forwarded by the cursor when a pointer emits an frame
   * event. Frame events are sent after regular pointer events to group
   * multiple events together. For instance, two axis events may happen at the
   * same time, in which case a frame event won't be sent in between. */
  /* Notify the client with pointer focus of the frame event. */
  wlr_seat_pointer_notify_frame(server->seat);
}

void cursor_init() {
  // A cursor is a wlroots utility to track cursor movement on the screen.
  server->cursor = wlr_cursor_create();
  wlr_cursor_attach_output_layout(server->cursor, server->output_layout);

  // The cursor manager is a wlroots utility for loading up cursor themes and
  // handling scale.
  server->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
  wlr_xcursor_manager_load(server->cursor_mgr, 1);
}

void cursor_events_init() {
  server->cursor_mode = SIMWL_CURSOR_PASSTHROUGH;

  server->cursor_motion.notify = on_cursor_motion;
  wl_signal_add(&server->cursor->events.motion, &server->cursor_motion);

  server->cursor_motion_absolute.notify = on_cursor_motion_absolute;
  wl_signal_add(&server->cursor->events.motion_absolute,
                &server->cursor_motion_absolute);

  server->cursor_button.notify = on_cursor_button;
  wl_signal_add(&server->cursor->events.button, &server->cursor_button);

  server->cursor_frame.notify = on_cursor_frame;
  wl_signal_add(&server->cursor->events.frame, &server->cursor_frame);
}

void process_cursor_move(uint32_t time) {
  struct simwm_view *view = server->grabbed_view;

  view->x = server->cursor->x - server->grab_x;
  view->y = server->cursor->y - server->grab_y;

  wlr_scene_node_set_position(&view->xdg->scene->node, view->x, view->y);
}

void process_cursor_motion(uint32_t time) {
  if (server->cursor_mode == SIMWL_CURSOR_MOVE) {
    process_cursor_move(time);
    return;
  } else if (server->cursor_mode == SIMWL_CURSOR_RESIZE) {
    return;
  }

  double sx, sy;
  struct wlr_surface *surface = NULL;
  struct simwm_view *view =
      view_at(server->cursor->x, server->cursor->y, &surface, &sx, &sy);

  if (!view) {
    wlr_xcursor_manager_set_cursor_image(server->cursor_mgr, "left_ptr",
                                         server->cursor);
    return;
  }

  if (surface) {
    wlr_seat_pointer_notify_enter(server->seat, surface, sx, sy);
    wlr_seat_pointer_notify_motion(server->seat, time, sx, sy);
  } else {
    wlr_seat_pointer_notify_clear_focus(server->seat);
  }
}

void on_cursor_motion(struct wl_listener *listener, void *data) {
  struct wlr_pointer_motion_event *event = data;
  wlr_cursor_move(server->cursor, &event->pointer->base, event->delta_x,
                  event->delta_y);
  process_cursor_motion(event->time_msec);
}

void on_cursor_motion_absolute(struct wl_listener *listener, void *data) {
  struct wlr_pointer_motion_absolute_event *event = data;
  wlr_cursor_warp_absolute(server->cursor, &event->pointer->base, event->x,
                           event->y);
  process_cursor_motion(event->time_msec);
}

void on_cursor_button(struct wl_listener *listener, void *data) {
  struct wlr_pointer_button_event *event = data;

  struct wlr_surface *surface = NULL;
  double sx, sy;

  struct simwm_view *view =
      view_at(server->cursor->x, server->cursor->y, &surface, &sx, &sy);

  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);

  if (event->state == WLR_BUTTON_RELEASED) {
    server->cursor_mode = SIMWL_CURSOR_PASSTHROUGH;
  } else {
    if (!view) {
      return;
    }
    focus_view(view, surface);
  }

  wlr_seat_pointer_notify_button(server->seat, event->time_msec, event->button,
                                 event->state);
}

void on_seat_request_cursor(struct wl_listener *listener, void *data) {
  struct wlr_seat_pointer_request_set_cursor_event *event = data;
  struct wlr_seat_client *focused_client =
      server->seat->pointer_state.focused_client;

  // Can be sent by any client, so check if client is focused
  if (focused_client == event->seat_client) {
    wlr_cursor_set_surface(server->cursor, event->surface, event->hotspot_x,
                           event->hotspot_y);
  }
}
