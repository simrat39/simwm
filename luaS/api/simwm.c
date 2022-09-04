#include "keyboard.h"
#include "layout.h"
#include <includes.h>
#include <lauxlib.h>
#include <lua.h>
#include <luaS/api/output.h>
#include <luaS/utils/dump.h>
#include <output.h>
#include <seat.h>
#include <server.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-util.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

int get_outputs(lua_State *L) {
  lua_newtable(L);

  struct simwm_output *output;
  int idx = 1;

  wl_list_for_each(output, &server->outputs, link) {
    // Make output object and put it on top of the stack
    luaS_output_from_simwm_output(L, output);

    // Table is now at -2 in the stack, pop the current element and add it to
    // the table.
    lua_rawseti(L, -2, idx);
    idx++;
  }

  return 1;
}

int add_workspace(lua_State *L) {
  wlr_log(WLR_INFO, "ADDING WORKSPACE");

  if (!lua_isstring(L, -1)) {
    wlr_log(WLR_ERROR, "Workspace name needs to be a string");
  }
  const char *ws_name = lua_tostring(L, -1);

  // Lua GC might free the string above, so copy it.
  seat_add_workspace(strdup(ws_name));
  return 0;
}

enum wlr_keyboard_modifier modifier_from_string(const char *str) {
  if (strcmp(str, "Shift") == 0) {
    return WLR_MODIFIER_SHIFT;
  }

  if (strcmp(str, "Logo") == 0) {
    return WLR_MODIFIER_LOGO;
  }

  if (strcmp(str, "Alt") == 0) {
    return WLR_MODIFIER_ALT;
  }

  return 0;
}

int add_keymap(lua_State *L) {
  wlr_log(WLR_INFO, "ADDING ALT KEYMAP");

  if (!lua_istable(L, 1)) {
    wlr_log(WLR_ERROR, "Modifiers need to be a table");
    return 0;
  }

  int modifiers = 0;

  int modifier_count = lua_objlen(L, 1);

  for (int i = 1; i <= modifier_count; i++) {
    lua_rawgeti(L, 1, i);
    const char *modifier_string = lua_tostring(L, -1);
    wlr_log(WLR_INFO, "%s", modifier_string);

    int wlr_modifier = modifier_from_string(modifier_string);
    wlr_log(WLR_INFO, "%i", wlr_modifier);
    modifiers |= wlr_modifier;
  }

  if (!lua_isstring(L, 2)) {
    wlr_log(WLR_ERROR, "Key name needs to be a string");
    return 0;
  }
  const char *keyname = lua_tostring(L, 2);
  xkb_keysym_t keysym = xkb_keysym_from_name(keyname, XKB_KEYSYM_NO_FLAGS);

  if (!lua_isfunction(L, 3)) {
    wlr_log(WLR_ERROR, "Where callback?");
    return 0;
  }
  // Push callback to top of stack cause luaL_ref takes the top one
  lua_pushvalue(L, 3);

  struct simwm_keymap *km = calloc(1, sizeof(struct simwm_keymap));
  km->on_press = luaL_ref(L, LUA_REGISTRYINDEX);
  km->key = keysym;
  km->modifiers = modifiers;

  if (lua_isfunction(L, 4)) {
    // Push callback to top of stack cause luaL_ref takes the top one
    lua_pushvalue(L, 4);
    km->on_release = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  wl_list_insert(&server->keymaps, &km->link);

  return 0;
}

int register_layout_manager(lua_State *L) {
  dumpstack(L);
  struct simwm_layout *layout = calloc(1, sizeof(struct simwm_layout));

  lua_getfield(L, 1, "name");
  const char *name = lua_tostring(L, -1);
  layout->name = strdup(name);

  lua_getfield(L, 1, "arrange");
  layout->arrange = luaL_ref(L, LUA_REGISTRYINDEX);

  lua_getfield(L, 1, "on_new_view");
  layout->on_new_view = luaL_ref(L, LUA_REGISTRYINDEX);

  lua_getfield(L, 1, "on_view_close");
  layout->on_view_close = luaL_ref(L, LUA_REGISTRYINDEX);

  wl_list_insert(&server->layouts, &layout->link);

  if (!server->current_layout) {
    server->current_layout = layout;
  }
  return 0;
}

int spawn(lua_State *L) {
  if (!lua_isstring(L, 1)) {
    wlr_log(WLR_ERROR, "Command is not a string");
    return 0;
  }

  const char *command = lua_tostring(L, 1);

  if (fork() == 0) {
    execl("/bin/sh", "/bin/sh", "-c", command, (void *)NULL);
  }

  return 0;
}

// Creates the global simwm table to interfacw with the api
void luaS_simwm_init() {
  lua_pushcfunction(server->L, get_outputs);
  lua_setglobal(server->L, "get_outputs");

  lua_pushcfunction(server->L, add_workspace);
  lua_setglobal(server->L, "add_workspace");

  lua_pushcfunction(server->L, add_keymap);
  lua_setglobal(server->L, "add_keymap");

  lua_pushcfunction(server->L, spawn);
  lua_setglobal(server->L, "spawn");

  lua_pushcfunction(server->L, register_layout_manager);
  lua_setglobal(server->L, "register_layout_manager");
}
