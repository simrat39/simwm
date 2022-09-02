#pragma once

#include <lua.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>

struct simwm_keyboard {
  struct wl_list link;
  struct wlr_keyboard *wlr_keyboard;

  struct wl_listener key;
  struct wl_listener modifiers;
};

struct simwm_keymap {
  struct wl_list link;
  xkb_keysym_t key;
  int on_press;
  int on_release;
  int modifiers;
};

void server_new_keyboard(struct wlr_input_device *);
