#include <includes.h>

#include <view.h>
#include <cursor.h>
#include <server.h>

#include <stdint.h>

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
}

void process_cursor_move(uint32_t time) {
  struct simwm_view *view = server->grabbed_view;

  view->x = server->cursor->x - server->grab_x;
  view->y = server->cursor->y - server->grab_y;

  wlr_scene_node_set_position(&view->scene_tree->node, view->x, view->y);
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
  } else {
    wlr_xcursor_manager_set_cursor_image(server->cursor_mgr, "crosshair",
                                         server->cursor);
  }

  if (surface) {
    wlr_seat_pointer_notify_enter(server->seat, surface, sx, sy);
    wlr_seat_pointer_notify_motion(server->seat, time, sx, sy);
  } else {
    wlr_seat_pointer_clear_focus(server->seat);
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
  struct wlr_pointer_button_event *event =
      data;

  wlr_seat_pointer_notify_button(server->seat, event->time_msec, event->button,
                                 event->state);

  struct wlr_surface *surface = NULL;
  double sx, sy;

  struct simwm_view *view =
      view_at(server->cursor->x, server->cursor->y, &surface, &sx, &sy);

  if (view == NULL) {
    return;
  }

  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);

  if (event->state == WLR_BUTTON_RELEASED) {
    server->cursor_mode = SIMWL_CURSOR_PASSTHROUGH;
  } else {
    focus_view(view, surface);
  }
}
