#include <includes.h>
#include <server.h>

void seat_kb_notify_enter(struct wlr_surface *surface) {
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);

  if (!keyboard) {
    return;
  }

  wlr_seat_keyboard_notify_enter(server->seat, surface, keyboard->keycodes,
                                 keyboard->num_keycodes, &keyboard->modifiers);
}
