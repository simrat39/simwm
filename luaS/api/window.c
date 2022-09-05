#include "xdg_shell.h"
#include <includes.h>
#include <lua.h>
#include <luaS/utils/dump.h>
#include <output.h>
#include <view.h>
#include <wlr/util/log.h>

int luaS_window_set_size(lua_State *L) {
  lua_getfield(L, 1, "userdata");
  struct simwm_xdg_surface *window = lua_touserdata(L, -1);

  lua_Number width = lua_tonumber(L, 2);
  lua_Number height = lua_tonumber(L, 3);

  wlr_xdg_toplevel_set_resizing(window->toplevel, true);
  wlr_xdg_toplevel_set_size(window->toplevel, width, height);
  wlr_xdg_toplevel_set_resizing(window->toplevel, false);

  return 0;
}

int luaS_window_set_pos(lua_State *L) {
  lua_getfield(L, 1, "userdata");
  struct simwm_xdg_surface *window = lua_touserdata(L, -1);

  lua_Number x = lua_tonumber(L, 2);
  lua_Number y = lua_tonumber(L, 3);

  wlr_scene_node_set_position(&window->scene->node, x, y);
  return 0;
}

int luaS_window_close(lua_State *L) {
  lua_getfield(L, 1, "userdata");
  struct simwm_xdg_surface *window = lua_touserdata(L, -1);

  wlr_xdg_toplevel_send_close(window->toplevel);

  return 0;
}

// Sets up a simwm_view table in lua and puts it on top of the stack.
void luaS_window_from_simwm_xdg_surface(lua_State *L,
                                        struct simwm_xdg_surface *window) {
  lua_newtable(L);

  lua_pushstring(L, "app_id");
  lua_pushstring(L, window->toplevel->app_id);
  lua_settable(L, -3);

  lua_pushstring(L, "set_pos");
  lua_pushcfunction(L, luaS_window_set_pos);
  lua_settable(L, -3);

  lua_pushstring(L, "set_size");
  lua_pushcfunction(L, luaS_window_set_size);
  lua_settable(L, -3);

  lua_pushstring(L, "close");
  lua_pushcfunction(L, luaS_window_close);
  lua_settable(L, -3);

  lua_pushstring(L, "userdata");
  lua_pushlightuserdata(L, window);
  lua_settable(L, -3);
}
