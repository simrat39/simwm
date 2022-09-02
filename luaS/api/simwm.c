#include "keyboard.h"
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
#include <wayland-util.h>
#include <wlr/util/log.h>

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

  lua_len(L, 1);
  int modifier_count = lua_tonumber(L, -1);

  dumpstack(L);

  for (int i = 1; i <= modifier_count; i++) {
    lua_rawgeti(L, 1, i);
    const char *modifier_string = lua_tostring(L, -1);
    wlr_log(WLR_INFO, "%s", modifier_string);

    int wlr_modifier = modifier_from_string(modifier_string);
    wlr_log(WLR_INFO, "%i", wlr_modifier);
    modifiers |= wlr_modifier;
  }

  if (!lua_isnumber(L, 2)) {
    wlr_log(WLR_ERROR, "Key name needs to be a number");
    return 0;
  }
  int keyname = lua_tonumber(L, 2);

  if (!lua_isfunction(L, 3)) {
    wlr_log(WLR_ERROR, "Where callback?");
    return 0;
  }
  // Push callback to top of stack cause luaL_ref takes the top one
  lua_pushvalue(L, 3);

  struct simwm_keymap *km = calloc(1, sizeof(struct simwm_keymap));
  km->callback = luaL_ref(L, LUA_REGISTRYINDEX);
  km->key = keyname;
  km->modifiers = modifiers;

  wl_list_insert(&server->keymaps, &km->link);

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
}
