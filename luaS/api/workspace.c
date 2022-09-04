#include <includes.h>
#include <lua.h>
#include <luaS/api/window.h>
#include <luaS/utils/dump.h>
#include <seat.h>
#include <wlr/util/log.h>

int luaS_workspace_get_windows(lua_State *L) {
  lua_getfield(L, 1, "userdata");
  struct simwm_workspace *workspace = lua_touserdata(L, -1);

  lua_newtable(L);

  struct simwm_xdg_surface *xdg_surface;
  int idx = 1;

  wl_list_for_each(xdg_surface, &workspace->views, ws_link) {
    // Make output object and put it on top of the stack
    luaS_window_from_simwm_xdg_surface(L, xdg_surface);

    // Table is now at -2 in the stack, pop the current element and add it to
    // the table.
    lua_rawseti(L, -2, idx);
    idx++;
  }

  return 1;
}

// Sets up a simwm_output table in lua and puts it on top of the stack.
void luaS_workspace_from_simwm_workspace(lua_State *L,
                                         struct simwm_workspace *workspace) {
  lua_newtable(L);

  lua_pushstring(L, "name");
  lua_pushstring(L, workspace->name);
  lua_settable(L, -3);

  lua_pushstring(L, "get_windows");
  lua_pushcfunction(L, luaS_workspace_get_windows);
  lua_settable(L, -3);

  lua_pushstring(L, "userdata");
  lua_pushlightuserdata(L, workspace);
  lua_settable(L, -3);
}
