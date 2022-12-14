#include "luaS/utils/dump.h"
#include <includes.h>
#include <keyboard.h>
#include <lua.h>
#include <output.h>
#include <seat.h>
#include <server.h>

#include <stdint.h>
#include <stdlib.h>

#include <unistd.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

void on_modifiers(struct wl_listener *listener, void *data) {
  struct simwm_keyboard *keyboard =
      wl_container_of(listener, keyboard, modifiers);

  wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);
  wlr_seat_keyboard_notify_modifiers(server->seat,
                                     &keyboard->wlr_keyboard->modifiers);
}

void on_key(struct wl_listener *listener, void *data) {
  struct simwm_keyboard *keyboard = wl_container_of(listener, keyboard, key);
  struct wlr_keyboard_key_event *event = data;

  // libinput -> xkb
  uint32_t keycode = event->keycode + 8;
  const xkb_keysym_t *syms;
  int nsyms =
      xkb_state_key_get_syms(keyboard->wlr_keyboard->xkb_state, keycode, &syms);
  uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);
  bool handled = false;

  struct simwm_keymap *km;
  wl_list_for_each(km, &server->keymaps, link) {
    if (modifiers & km->modifiers) {
      for (int i = 0; i < nsyms; i++) {
        if (km->key == syms[i]) {
          if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
            lua_rawgeti(server->L, LUA_REGISTRYINDEX, km->on_press);
            lua_pcall(server->L, 0, 0, 0);
          } else {
            lua_rawgeti(server->L, LUA_REGISTRYINDEX, km->on_release);
            lua_pcall(server->L, 0, 0, 0);
          }

          handled = true;
        }
      }
    }
  }

  if (!handled) {
    wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_key(server->seat, event->time_msec, event->keycode,
                                 event->state);
  }
}

void server_new_keyboard(struct wlr_input_device *device) {
  struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

  struct simwm_keyboard *keyboard = calloc(1, sizeof(struct simwm_keyboard));
  keyboard->wlr_keyboard = wlr_keyboard;

  struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  struct xkb_keymap *keymap =
      xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
  wlr_keyboard_set_keymap(keyboard->wlr_keyboard, keymap);

  xkb_keymap_unref(keymap);
  xkb_context_unref(context);
  wlr_keyboard_set_repeat_info(keyboard->wlr_keyboard, 50, 300);

  keyboard->modifiers.notify = on_modifiers;
  wl_signal_add(&keyboard->wlr_keyboard->events.modifiers,
                &keyboard->modifiers);

  keyboard->key.notify = on_key;
  wl_signal_add(&keyboard->wlr_keyboard->events.key, &keyboard->key);

  wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);

  wl_list_insert(&server->keyboards, &keyboard->link);
}
