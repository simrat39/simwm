#include <includes.h>
#include <lua.h>
#include <output.h>
#include <server.h>

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
}
