#include <includes.h>
#include <lua.h>
#include <output.h>
#include <server.h>
#include <luaS/api/output.h>

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

// Creates the global simwm table to interfacw with the api
void luaS_simwm_init() {
  lua_pushcfunction(server->L, get_outputs);
  lua_setglobal(server->L, "get_outputs");
}
