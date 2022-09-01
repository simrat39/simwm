#include "server.h"
#include <assert.h>
#include <includes.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <output.h>
#include <wayland-server-core.h>
#include <wlr/util/log.h>

int get_coords(lua_State *L) {
  lua_getfield(L, -1, "userdata");
  struct simwm_output *o = lua_touserdata(L, -1);

  double lx, ly;
  wlr_output_layout_output_coords(server->output_layout, o->wlr_output, &lx,
                                  &ly);
  wlr_log(WLR_INFO, "%f %f", lx, ly);

  lua_newtable(L);

  lua_pushnumber(L, (float)lx);
  lua_setfield(L, -2, "x");

  lua_pushnumber(L, (float)ly);
  lua_setfield(L, -2, "y");

  return 1;
}

int get_outputs(lua_State *L) {
  struct simwm_output *o =
      wlr_output_layout_output_at(server->output_layout, 0, 0)->data;
  lua_newtable(L);

  lua_pushstring(L, "name");
  lua_pushstring(L, o->wlr_output->name);
  lua_settable(L, -3);

  lua_pushstring(L, "userdata");
  lua_pushlightuserdata(L, o);
  lua_settable(L, -3);

  lua_pushstring(L, "get_coords");
  lua_pushcfunction(L, get_coords);
  lua_settable(L, -3);

  return 1;
}

struct lua_State *luaS_init() {
  struct lua_State *L = luaL_newstate();

  luaL_openlibs(L);
  const char *LUA_FILE = "test.lua";
  lua_pushcfunction(L, get_outputs);
  lua_setglobal(L, "get_outputs");
  luaL_dofile(L, LUA_FILE);

  return L;
}

void luaS_fini(struct lua_State *L) { lua_close(L); }
