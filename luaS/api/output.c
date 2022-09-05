#include <includes.h>
#include <lauxlib.h>
#include <lua.h>
#include <luaS/api/workspace.h>
#include <output.h>
#include <seat.h>
#include <server.h>
#include <stdio.h>
#include <wlr/util/log.h>

int get_resolution(lua_State *L) {
  lua_getfield(L, -1, "userdata");
  struct simwm_output *o = lua_touserdata(L, -1);

  int width, height;
  wlr_output_effective_resolution(o->wlr_output, &width, &height);

  lua_newtable(L);

  lua_pushnumber(L, (float)width);
  lua_setfield(L, -2, "width");

  lua_pushnumber(L, (float)height);
  lua_setfield(L, -2, "height");

  return 1;
}

int get_coords(lua_State *L) {
  lua_getfield(L, -1, "userdata");
  struct simwm_output *o = lua_touserdata(L, -1);

  double lx, ly;
  wlr_output_layout_output_coords(server->output_layout, o->wlr_output, &lx,
                                  &ly);

  lua_newtable(L);

  lua_pushnumber(L, (float)lx);
  lua_setfield(L, -2, "x");

  lua_pushnumber(L, (float)ly);
  lua_setfield(L, -2, "y");

  return 1;
}
int get_current_workspace(lua_State *L) {
  lua_getfield(L, -1, "userdata");
  struct simwm_output *o = lua_touserdata(L, -1);

  luaS_workspace_from_simwm_workspace(L, o->current_workspace);
  return 1;
}

int set_workspace(lua_State *L) {
  if (!lua_isstring(L, -1)) {
    wlr_log(WLR_ERROR, "Workspace name needs to be a string");
    return 0;
  }

  const char *ws_name = lua_tostring(L, -1);

  lua_getfield(L, -2, "userdata");
  if (!lua_islightuserdata(L, -1)) {
    wlr_log(WLR_ERROR, "Output has invalid userdata");
    return 0;
  }

  struct simwm_output *output = lua_touserdata(L, -1);
  seat_set_current_workspace(output, (char *)ws_name);

  return 0;
}

// Sets up a simwm_output table in lua and puts it on top of the stack.
void luaS_output_from_simwm_output(lua_State *L, struct simwm_output *output) {
  lua_newtable(L);

  lua_pushstring(L, "name");
  lua_pushstring(L, output->wlr_output->name);
  lua_settable(L, -3);

  lua_pushstring(L, "description");
  lua_pushstring(L, output->wlr_output->description);
  lua_settable(L, -3);

  lua_pushstring(L, "userdata");
  lua_pushlightuserdata(L, output);
  lua_settable(L, -3);

  lua_pushstring(L, "get_coords");
  lua_pushcfunction(L, get_coords);
  lua_settable(L, -3);

  lua_pushstring(L, "get_resolution");
  lua_pushcfunction(L, get_resolution);
  lua_settable(L, -3);

  lua_pushstring(L, "get_current_workspace");
  lua_pushcfunction(L, get_current_workspace);
  lua_settable(L, -3);

  lua_pushstring(L, "set_workspace");
  lua_pushcfunction(L, set_workspace);
  lua_settable(L, -3);
}
