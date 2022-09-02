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
  wlr_log(WLR_INFO, "%f %f", lx, ly);

  lua_newtable(L);

  lua_pushnumber(L, (float)lx);
  lua_setfield(L, -2, "x");

  lua_pushnumber(L, (float)ly);
  lua_setfield(L, -2, "y");

  return 1;
}
