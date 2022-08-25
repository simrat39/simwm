#include <includes.h>
#include <input.h>
#include <keyboard.h>
#include <server.h>

void on_new_input(struct wl_listener *listener, void *data) {
  struct wlr_input_device *device = data;

  switch (device->type) {
  case WLR_INPUT_DEVICE_KEYBOARD:
    server_new_keyboard(device);
    break;
  case WLR_INPUT_DEVICE_POINTER:
    wlr_cursor_attach_input_device(server->cursor, device);
    break;
  case WLR_INPUT_DEVICE_TOUCH:
  case WLR_INPUT_DEVICE_TABLET_TOOL:
  case WLR_INPUT_DEVICE_TABLET_PAD:
  case WLR_INPUT_DEVICE_SWITCH:
    break;
  }

  uint32_t caps = WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD;
  wlr_seat_set_capabilities(server->seat, caps);
}
